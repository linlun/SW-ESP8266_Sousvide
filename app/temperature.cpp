/*
 * temperature.c
 *
 *  Created on: 23 aug 2015
 *      Author: Linus
 */

#include <temperature.h>

temperature::temperature(OneWire* sensor)
{
	m_sensor = sensor;
	m_sensorid = 0;
	m_mode = TEMP_READ_MODE::MAX;
}

void temperature::init(void)
{
	byte i, j;
	byte type_s;
	//

	this->setMode(TEMP_READ_MODE::MAX, 0u);

	m_sensor->begin(); // It's required for one-wire initialization!
	m_sensor->reset();
	delayMicroseconds(2);

	i = 0;
	while (i < MAX_NUM_DS_SENSORS)
	{
		m_addr[i][0] = 0u;
		i++;
	}
	Serial.println("Search");
	i = 0;
	while (m_sensor->search(m_addr[i]))
	{
		Serial.print("Thermometer ROM =");
		for( j = 0; j < 8; j++)
		{
			Serial.write(' ');
			Serial.print(m_addr[i][j], HEX);
		}
		Serial.println();
		if (OneWire::crc8(m_addr[i], 7) != m_addr[i][7])
		{
		  Serial.println("CRC is not valid!");
		  i--;
		}
		Serial.println();

		// the first ROM byte indicates which chip
		switch (m_addr[i][0]) {
		case 0x10:
		  Serial.println("  Chip = DS18S20");  // or old DS1820
		  type_s = 1;
		  break;
		case 0x28:
		  Serial.println("  Chip = DS18B20");
		  type_s = 0;
		  break;
		case 0x22:
		  Serial.println("  Chip = DS1822");
		  type_s = 0;
		  break;
		default:
		  Serial.println("Device is not a DS18x20 family device.");
		  i--;
		}
		i++;
		if (i >= MAX_NUM_DS_SENSORS)
			break;
	}
	numSensors = i;
	m_sensor->reset();
	m_sensor->skip();
	m_sensor->write(0x44, 1);        // start conversion, with parasite power on at the end
	Serial.println("...done");
}
void temperature::run()
{
	byte i, j;
	byte present = 0;
	byte type_s;
	byte data[12];

	if (m_running)
	{
		return;
	}
	m_running = true;

		//Serial.println("Print Temperatures");
	for( j = 0; j < MAX_NUM_DS_SENSORS; j++)
	{
		if (m_addr[j][0] == 0u)
		{
			break;
		}
		present = m_sensor->reset();
		m_sensor->select(m_addr[j]);
		m_sensor->write(0xBE);         // Read Scratchpad

		//Serial.print("  Data = ");
		//Serial.print(present, HEX);
		//Serial.print(" ");
		for ( i = 0; i < 9; i++)
		{
			// we need 9 bytes
			data[i] = m_sensor->read();
			//Serial.print(data[i], HEX);
			//Serial.print(" ");
		}
		//Serial.print(" CRC=");
		//Serial.print(OneWire::crc8(data, 8), HEX);
		//Serial.println();

		switch (m_addr[i][0]) {
			case 0x10:
			  type_s = 1;
			  break;
			case 0x28:
			case 0x22:
			  type_s = 0;
			  break;
			default:
			  type_s = 0;
		}

		// Convert the data to actual temperature
		// because the result is a 16 bit signed integer, it should
		// be stored to an "int16_t" type, which is always 16 bits
		// even when compiled on a 32 bit processor.
		int16_t raw = (data[1] << 8) | data[0];
		if (type_s)
		{
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10)
			{
			  // "count remain" gives full 12 bit resolution
			  raw = (raw & 0xFFF0) + 12 - data[6];
			}
		} else {
			byte cfg = (data[4] & 0x60);
			// at lower res, the low bits are undefined, so let's zero them
			if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
			//// default is 12 bit resolution, 750 ms conversion time
		}

		m_Temp_celsius[j] = (float)raw / 16.0;
//			fahrenheit = celsius * 1.8 + 32.0;
		//Serial.print("  Temperature = ");
		//Serial.print(raw);
		//Serial.print("  ");
		//Serial.print(Temp_celsius[j]);
		//Serial.print(" Celsius, ");
//			Serial.print(fahrenheit);
//			Serial.println(" Fahrenheit");
		//Serial.println();
	}

	//Input = Temp_celsius[0];
	m_sensor->reset();
	m_sensor->skip();
	m_sensor->write(0x44, 1);        // start conversion, with parasite power on at the end
	m_running = false;

	for (i = 0; i < numSensors; i++)
	{
		//Serial.print(m_Temp_celsius[i]);
		//Serial.print("  ");
	}
	//Serial.println();
}


void temperature::setMode(TEMP_READ_MODE mode, uint8_t sensorId)
{
	m_sensorid = sensorId;
	m_mode = mode;
}

TEMP_READ_MODE temperature::getMode(uint8_t* sensorId)
{
	*sensorId = m_sensorid;
	return m_mode;
}

float temperature::get()
{
	float output;
	if (m_mode == TEMP_READ_MODE::SENSORID)
	{
		this->get(m_sensorid);
	} else if (m_mode == TEMP_READ_MODE::MIN)
	{
		output = 9999.0;
		for (uint8_t i = 0; i < numSensors; i++)
		{
			if (m_Temp_celsius[i] < output )
			{
				output = m_Temp_celsius[i];
			}
		}
	} else if (m_mode == TEMP_READ_MODE::MAX)
	{
		output = -9999.0;
		for (uint8_t i = 0; i < numSensors; i++)
		{
			if (output < m_Temp_celsius[i] )
			{
				output = m_Temp_celsius[i];
			}
		}
	} else if (m_mode == TEMP_READ_MODE::MEAN)
	{
		output = 0.0;
		for (uint8_t i = 0; i < numSensors; i++)
		{
			output += m_Temp_celsius[i];
		}
		output = output / numSensors;
	}
	return output;
}

float temperature::get(uint8_t id)
{
	if (id < MAX_NUM_DS_SENSORS)
	{
		return m_Temp_celsius[id];
	}
	return 0.0;
}





MenuItem_Adjust_TempMode_Screen::MenuItem_Adjust_TempMode_Screen(const String* name) {
	m_name = name;
}

void MenuItem_Adjust_TempMode_Screen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, TEMP_READ_MODE (*GetValue)(uint8_t*), void (*SetValue)(TEMP_READ_MODE, uint8_t), temperature* temp){
	m_display = display;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_SetValue = SetValue;
	m_tempsensor = temp;
}

void MenuItem_Adjust_TempMode_Screen::redraw(void)	// text display tests
{
Serial.print("redraw adjust");
m_display->clearDisplay();
m_display->setTextColor(BLACK, WHITE); // 'inverted' text
m_display->setTextSize(1);
m_display->setCursor(0,0);
m_display->print("Adjust value");
m_display->setTextColor(WHITE);
uint16_t txtLen = m_name->length();

if(txtLen > 10)
{
	m_display->setTextSize(1);
} else if(txtLen > 6)
{
	m_display->setTextSize(2);
} else
{
	m_display->setTextSize(3);
}
m_display->setCursor(5,12);
m_display->print(*m_name);
m_display->setTextSize(3);
m_display->setCursor(0,40);
if (m_state == 0)
{
	m_display->print(">");
}
switch (m_value)
{
	case TEMP_READ_MODE::MAX:
		m_display->print("Max");
		break;
	case TEMP_READ_MODE::MIN:
		m_display->print("Min");
		break;
	case TEMP_READ_MODE::MEAN:
		m_display->print("Mean");
		break;
	case TEMP_READ_MODE::SENSORID:
		m_display->print("Sens ");
		char buff[3];
		itoa(m_id, buff, 10);
		m_display->print(buff);
		m_display->setTextSize(1);
		m_display->setCursor(90,0);
		m_display->print(m_tempsensor->get(m_id));
		m_display->print((char)247);
		break;
}
/*
for (uint8_t i = 0; i < 7; i++)
{
	if (i + 10 == m_state)
	{
		m_display->setTextColor(BLACK, WHITE); // 'inverted' text
		m_display->print(m_charbuf[i]);
		m_display->setTextColor(WHITE);
	} else
	{
		m_display->print(m_charbuf[i]);
	}
}
//m_display->println(m_charbuf);
m_display->setCursor(0,0);
*/
switch (m_state)
{
case 1:
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Cancel");
	break;
case 2:
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Save");
	break;
}

m_display->display();

}

MenuItem* MenuItem_Adjust_TempMode_Screen::Loop () {
  return this;
}

MenuItem* MenuItem_Adjust_TempMode_Screen::Update (int rotation, bool pressed) {

	if (m_state >= 10) //Adjust mode
	{
		//Serial.print(m_tempsensor->numSensors);
		m_state += rotation;
		if (m_state > (12 + m_tempsensor->numSensors))
			m_state -= (3 + m_tempsensor->numSensors);
		if (m_state < 10)
			m_state += (3 + m_tempsensor->numSensors);
		//adjustChar(rotation, &m_charbuf[m_state - 10]);
		if (m_state == 10)
		{
			m_value = TEMP_READ_MODE::MAX;
		} else if (m_state == 11)
		{
			m_value = TEMP_READ_MODE::MIN;
		} else if (m_state == 12)
		{
			m_value = TEMP_READ_MODE::MEAN;
		} else
		{
			m_value = TEMP_READ_MODE::SENSORID;
			m_id = m_state - 13;
		}
		//Serial.print(" state ");
		//Serial.println(m_state);
	} else //Move mode
	{
		m_state += rotation;
		if (m_state >= (3))
			m_state -= (3);
		if (m_state < 0)
			m_state += (3);
	}

	if (pressed)
	{
		pressTime = millis();
		Serial.print("PressTime ");
		Serial.print(pressTime);
	}

	if (!pressed & pressTime != 0)
	{
		unsigned long passedTime = millis() - pressTime ;
		Serial.print("PassedTime ");
				Serial.print(passedTime);
		if (passedTime > 60 && passedTime < 500)
		{
			if (m_state == 0)
			{
				if (m_value == TEMP_READ_MODE::MAX)
				{
					m_state = 10;
				} else if (m_value == TEMP_READ_MODE::MIN)
				{
					m_state = 11;
				} else if (m_value == TEMP_READ_MODE::MEAN)
				{
					m_state = 12;
				} else
				{
					m_state = 13 + m_id;
				}
			} else if (m_state >= 10)
			{
				m_state = 0;
				//Go back to move mode
				//Check and correct if a faulty value is entered
				//checkCharBuf();
			}
			if (m_state == 2)
			{
				//Save value
				m_SetValue(m_value, m_id);
				return m_submenu;
			}
			if (m_state == 1)
			{
				//Cancel
				return m_submenu;
			}
		}
		pressTime = 0;
	}

	redraw();
	return this;
}
void MenuItem_Adjust_TempMode_Screen::Init () {
	pressTime = 0;
	m_value = m_GetValue(&m_id);
	m_state = 0u;
	redraw();
}
void MenuItem_Adjust_TempMode_Screen::DeInit () {
  ;
}

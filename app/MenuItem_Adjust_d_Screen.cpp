
#include "MenuItem_Adjust_d_Screen.h"

MenuItem_Adjust_d_Screen::MenuItem_Adjust_d_Screen(const String* name) {
	m_name = name;
}

void MenuItem_Adjust_d_Screen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, double (*GetValue)(void), void (*SetValue)(double)){
	m_display = display;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_SetValue = SetValue;
}

void MenuItem_Adjust_d_Screen::redraw(void)	// text display tests
{
Serial.print("redraw adjust");
m_display->clearDisplay();
m_display->setTextColor(BLACK, WHITE); // 'inverted' text
m_display->setTextSize(1);
m_display->setCursor(0,0);
m_display->print("Adjust value");
m_display->setTextColor(WHITE);
uint16_t txtLen = m_name->length();
//Serial.print(txtLen);
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

switch (m_state)
{
case 0:
	m_display->drawFastHLine(0,63,18,WHITE);
	break;
case 1:
	m_display->drawFastHLine(18,63,18,WHITE);
	break;
case 2:
	m_display->drawFastHLine(36,63,18,WHITE);
	break;
case 3:
	m_display->drawFastHLine(54,63,18,WHITE);
	break;
case 4:
	m_display->drawFastHLine(72,63,18,WHITE);
	break;
case 5:
	m_display->drawFastHLine(90,63,18,WHITE);
	break;
case 6:
	m_display->drawFastHLine(108,63,18,WHITE);
	break;
case 7:
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Save");
	break;
case 8:
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Cancel");
	break;
}
m_display->display();

}

MenuItem* MenuItem_Adjust_d_Screen::Loop () {
  return this;
}

void MenuItem_Adjust_d_Screen::adjustChar(int rot, char* chr)
{
	int8_t position;
	if (*chr <= '9' && *chr >= '0')
		position = *chr - '0';
	else if (*chr == '-')
		position = 10;
	else if (*chr == '.')
		position = 11;
	else if (*chr == ' ')
		position = 12;

	Serial.print("Position was ");
	Serial.println(position);
	position += rot;
	if (position < 0)
		position += 13;
	if (position > 12)
		position -= 13;

	Serial.print("Position is ");
	Serial.println(position);

	if (position >= 0 && position <= 9)
		*chr = '0' + position;
	else if (position == 10)
		*chr = '-';
	else if (position == 11)
		*chr = '.';
	else if (position == 12)
		*chr = ' ';
}

void MenuItem_Adjust_d_Screen::checkCharBuf()
{
	bool foundDigit = false;
	bool foundDot = false;
	bool foundMinus = false;

	for(uint8_t i = 0; i < 8;i++)
	{
		 if (m_charbuf[i] == '-')
		{
			if (foundMinus)
				m_charbuf[i] = '0';
			if (foundDigit)
				m_charbuf[i] = '0';
			if (m_charbuf[i+1] >= '0' && m_charbuf[i+1] <= '9')
				foundMinus = true;
		} else if (m_charbuf[i] == '.')
		{
			if (foundDot)
				m_charbuf[i] = '0';
			if (!foundDigit)
			{
				m_charbuf[i] = ' ';

			} else
			{
				foundDot = true;
			}
		} else if (m_charbuf[i] == ' ')
		{
			if (foundDigit)
			{
				//Set all remaining to space
				while(m_charbuf[++i])
				{
					m_charbuf[i] = ' ';
				}
				break;
			}

		} else
		{
			foundDigit= true;
		}
	}

}
MenuItem* MenuItem_Adjust_d_Screen::Update (int rotation, bool pressed) {

	if (m_state >= 10) //Adjust mode
	{
		adjustChar(rotation, &m_charbuf[m_state - 10]);
	} else //Move mode
	{
		m_state += rotation;
		if (m_state >= 9)
			m_state -= 9;
		if (m_state < 0)
			m_state += 9;
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
			if (m_state < 7)
			{
				m_state += 10;
			} else if (m_state >= 10)
			{
				m_state -= 10;
				//Go back to move mode
				//Check and correct if a faulty value is entered
				checkCharBuf();
			}
			if (m_state == 7)
			{
				//Save value
				String str(m_charbuf);
				double value = (double)str.toFloat();
				Serial.print("value was ");
				Serial.println(value);
				m_SetValue(value);
				return m_submenu;
			}
			if (m_state == 8)
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
void MenuItem_Adjust_d_Screen::Init () {
	pressTime = 0;
	m_value = m_GetValue();
	m_state = 0u;
	m_charbuf[0] = ' ';
	m_charbuf[1] = ' ';
	m_charbuf[2] = ' ';
	m_charbuf[3] = ' ';
	m_charbuf[4] = ' ';
	m_charbuf[5] = ' ';
	m_charbuf[6] = ' ';
	m_charbuf[7] = 0;
	char result[11];
	dtostrf(m_value, 6, 4, result);
	uint8_t i =0u;
	while (result[i])
	{
		m_charbuf[i] = result[i];
		i++;
		if (i > 6)
			break;
	}
	redraw();
}
void MenuItem_Adjust_d_Screen::DeInit () {
  ;
}

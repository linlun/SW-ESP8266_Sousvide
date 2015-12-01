
#include "MenuItem_Adjust_Enum_Screen.h"

MenuItem_Adjust_Enum_Screen::MenuItem_Adjust_Enum_Screen(const String* name) {
	m_name = name;
}

void MenuItem_Adjust_Enum_Screen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, String enumString[], uint8 enumSize, int (*GetValue)(void), void (*SetValue)(int)){
	m_display = display;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_SetValue = SetValue;
	m_charbuf = enumString;
	m_enumSize = enumSize;
}

void MenuItem_Adjust_Enum_Screen::redraw(void)	// text display tests
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

txtLen = m_charbuf[m_value].length();
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
m_display->setCursor(0,40);
if (m_state == 0)
{
	m_display->print(">");
}
m_display->print(m_charbuf[m_value]);

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

MenuItem* MenuItem_Adjust_Enum_Screen::Loop () {
  return this;
}



MenuItem* MenuItem_Adjust_Enum_Screen::Update (int rotation, bool pressed) {

	if (m_state >= 10) //Adjust mode
	{
		//Serial.print(m_tempsensor->numSensors);
		m_state += rotation;
		while (m_state >= (10 + m_enumSize))
			m_state -= (m_enumSize);
		while (m_state < 10)
			m_state += m_enumSize;
		//adjustChar(rotation, &m_charbuf[m_state - 10]);
		m_value = m_state - 10;

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
				m_state = 10 + m_value;
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
				m_SetValue(m_value);
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
void MenuItem_Adjust_Enum_Screen::Init () {
	pressTime = 0;
	m_value = m_GetValue();
	m_state = 0u;
	redraw();
}
void MenuItem_Adjust_Enum_Screen::DeInit () {
  ;
}

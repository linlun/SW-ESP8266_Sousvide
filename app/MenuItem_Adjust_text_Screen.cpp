
#include "MenuItem_Adjust_text_Screen.h"

MenuItem_Adjust_text_Screen::MenuItem_Adjust_text_Screen(const String* name) {
	m_name = name;
}

void MenuItem_Adjust_text_Screen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, String (*GetValue)(void), void (*SetValue)(String)){
	m_display = display;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_SetValue = SetValue;
}

void MenuItem_Adjust_text_Screen::redraw(void)	// text display tests
{
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
m_display->setTextSize(1);
m_display->setCursor(0,40);

for (uint8_t i = 0; i < 22; i++)
{
	if (i + 32 == m_state)
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
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Save");
	break;
case 1:
	m_display->setTextColor(BLACK, WHITE); // 'inverted' text
	m_display->setTextSize(1);
	m_display->setCursor(90,0);
	m_display->print("Cancel");
	break;
default:
	if (m_state < 32)
	{
		m_display->drawFastHLine((m_state-2)*6,53,6,WHITE);
	} else {
		m_display->drawFastHLine((m_state-32)*6,53,6,WHITE);
	}
	break;
}
m_display->display();

}

MenuItem* MenuItem_Adjust_text_Screen::Loop () {
  return this;
}

void MenuItem_Adjust_text_Screen::adjustChar(int rot, char* chr)
{
	//Serial.print("Was: ");
	//Serial.println(chr[0]);
	chr[0] += rot;
	if ((int)chr[0] < 32)
	{
		chr[0] += 95;
	}
	if ((int)chr[0] > 126)
	{
		chr[0] -= 95;
	}
	//Serial.print("Now: ");
	//Serial.println(chr[0]);
}

void MenuItem_Adjust_text_Screen::checkCharBuf()
{


}
MenuItem* MenuItem_Adjust_text_Screen::Update (int rotation, bool pressed) {

	if (m_state >= 32) //Adjust mode
	{
		adjustChar(rotation, &m_charbuf[m_state - 32]);
	} else //Move mode
	{
		m_state += rotation;
		if (m_state >= 22+2)
			m_state -= 22+2;
		if (m_state < 0)
			m_state += 22+2;
	}

	if (pressed)
	{
		pressTime = millis();
		//Serial.print("PressTime ");
		//Serial.print(pressTime);
	}

	if (!pressed & pressTime != 0)
	{
		unsigned long passedTime = millis() - pressTime;
		//Serial.print("PassedTime ");
		//		Serial.print(passedTime);
		if (passedTime > 60 && passedTime < 500)
		{
			if (m_state == 0)
			{
				//Save value
				String str(m_charbuf);
				str.trim();
				m_SetValue(str);
				return m_submenu;
			}
			if (m_state == 1)
			{
				//Cancel
				return m_submenu;
			}
			if (m_state < (22+2))
			{
				m_state += 30;
			} else if (m_state >= 30)
			{
				m_state -= 30;
				//Go back to move mode
				//Check and correct if a faulty value is entered
				checkCharBuf();
			}

		}
		pressTime = 0;
	}
	redraw();
	return this;
}
void MenuItem_Adjust_text_Screen::Init () {
	pressTime = 0;
	m_value = m_GetValue();

	for (int i = 0; i<22; i++)
	{
		if (i < m_value.length())
		{
			m_charbuf[i] = m_value.charAt(i);
		} else
		{
			m_charbuf[i] = ' ';
		}
	}
	m_state = 2u;
	redraw();
}
void MenuItem_Adjust_text_Screen::DeInit () {
  ;
}

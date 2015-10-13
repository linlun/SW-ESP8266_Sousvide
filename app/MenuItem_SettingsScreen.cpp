
#include "MenuItem_SettingsScreen.h"

MenuItem_SettingsScreen::MenuItem_SettingsScreen(const String* name) {
	m_name = name;
}

void MenuItem_SettingsScreen::Config(Adafruit_SSD1306* display , SettingsElement* elements[], uint8_t elementSize) {
	m_display = display;
	m_elements = elements;
	m_elementSize = elementSize;
	m_selected = 0;
	m_viewStart = 0;
	m_viewEnd = 6;
}

void MenuItem_SettingsScreen::redraw(void)	// text display tests
{
Serial.print("Init Settings");
m_display->clearDisplay();
m_display->setTextColor(BLACK, WHITE); // 'inverted' text
m_display->setTextSize(1);
m_display->setCursor(0,0);
m_display->println(*m_name);
m_display->setTextColor(WHITE);
for (uint8_t i = 0; i < min(m_elementSize, 7); i++)
{
	if (i+m_viewStart == m_selected)
	{
		m_display->setCursor(0,(i+1)*8);
		m_display->print(">");
	}
	m_display->setCursor(6,(i+1)*8);

	m_display->print(*m_elements[i+m_viewStart]->m_name);
	if (m_elements[i+m_viewStart]->m_GetValue!= null)
	{
		m_display->setCursor(70,(i+1)*8);
		char result[10];
		m_elements[i+m_viewStart]->m_GetValue(result);
		m_display->print(result);
	}
}

m_display->setCursor(0,0);
m_display->display();
}

MenuItem* MenuItem_SettingsScreen::Loop () {
  return this;
}
MenuItem* MenuItem_SettingsScreen::Update (int rotation, bool pressed) {
	m_selected += rotation;
	while (m_selected < 0)
		m_selected += m_elementSize;
	while (m_selected >= m_elementSize)
		m_selected -= m_elementSize;

	while (m_selected > m_viewEnd)
	{
		m_viewEnd++;
		m_viewStart++;
	}
	while (m_selected < m_viewStart)
	{
		m_viewEnd--;
		m_viewStart--;
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
			if (m_elements[m_selected]->m_RunFuncOnPress != null)
			{
				m_elements[m_selected]->m_RunFuncOnPress();
			}
			return m_elements[m_selected]->m_submenu;
		}
		pressTime = 0;
	}

	redraw();
	return this;
}
void MenuItem_SettingsScreen::Init () {
	pressTime = 0;
	redraw();
}
void MenuItem_SettingsScreen::DeInit () {
  ;
}


SettingsElement::SettingsElement(const String* name, MenuItem* submenu)
{
	m_name = name;
	m_submenu = submenu;
	m_GetValue = null;
	m_RunFuncOnPress = null;
}
SettingsElement::SettingsElement(const String* name, void (*GetValue)(char*), MenuItem* submenu)
{
	m_name = name;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_RunFuncOnPress = null;
}
SettingsElement::SettingsElement(const String* name, void (*GetValue)(char*), MenuItem* submenu, void (*RunFuncOnPress)(void))
{
	m_name = name;
	m_submenu = submenu;
	m_GetValue = GetValue;
	m_RunFuncOnPress = RunFuncOnPress;
}


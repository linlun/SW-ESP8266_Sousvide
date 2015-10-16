/*
 * WifiMenu.cpp
 *
 *  Created on: 14 okt 2015
 *      Author: linus
 */
#include "application.h"
#include "WifiMenu.h"
#include "SettingsMenu.h"
#include "AppSettings.h"

const String wifiname = " Wifi Settings     ";
MenuItem_SettingsScreen scr_WifiSettings(&wifiname);


SettingsElement WifiExit(&exit_name, &scr_settings);


BssList networks;
const String SsidSelect_name = "Select SSID in list:";
MenuItem_Adjust_SSID_Screen scr_adjSSID(&SsidSelect_name);
void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
	for (int i = 0; i < networks.count(); i++)
	{
		Serial.print("SSID: ");
		Serial.println(networks[i].ssid);
	}
	scr_adjSSID.m_elementSize = networks.count() + 1;
}

void GetSSIDList(void)
{
	WifiStation.startScan(networkScanCompleted);
}


const String Ssid_name = "SSID";


void SetSsid(String ssid)
{
	AppSettings.ssid = ssid;
}
String GetSsid(void)
{
	return AppSettings.ssid;
}
void GetSsidStr(char* str)
{
	str[9] = 0;
	AppSettings.ssid.toCharArray(str, min(AppSettings.ssid.length(),10));
}
SettingsElement Ssid(&Ssid_name, &GetSsidStr, &scr_adjSSID);//&scr_adjSsid);

const String ScanNetwork_name = "Scan for networks";
SettingsElement ScanNetwork(&ScanNetwork_name, null, &scr_WifiSettings, &GetSSIDList);//&scr_adjSsid);


SettingsElement* WifiSettingsElements[] = { &ScanNetwork, &Ssid, &WifiExit};

void WifiMenuConfig(Adafruit_SSD1306 *display)
{
	Serial.println("Created wifi menu");
	Serial.print("SSID: ");
	Serial.println(AppSettings.ssid);
	char buffer[30];
	   sprintf(buffer, "with %%p:  x    = %p\n", AppSettings);
	   Serial.print(buffer);

	//Serial.println((uint32_t)&AppSettings);
	scr_WifiSettings.Config(display, WifiSettingsElements, (sizeof(WifiSettingsElements)/sizeof(*WifiSettingsElements)));

	scr_adjSSID.Config(display, &scr_WifiSettings, &networks);
}





MenuItem_Adjust_SSID_Screen::MenuItem_Adjust_SSID_Screen(const String* name) {
	m_name = name;
}

void MenuItem_Adjust_SSID_Screen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, BssList* networks){
	m_display = display;
	m_submenu = submenu;
	m_networks = networks;
}

void MenuItem_Adjust_SSID_Screen::redraw(void)	// text display tests
{

m_display->clearDisplay();
m_display->setTextColor(BLACK, WHITE); // 'inverted' text
m_display->setTextSize(1);
m_display->setCursor(0,0);
m_display->print(*m_name);
m_display->setTextColor(WHITE);

for (uint8_t i = 0; i < min(m_elementSize, 7); i++)
{
	if (i+m_viewStart == m_selected)
	{
		m_display->setCursor(0,(i+1)*8);
		m_display->print(">");
	}
	m_display->setCursor(6,(i+1)*8);
	if (i+m_viewStart == (m_elementSize - 1))
	{
		m_display->print("Cancel");
	} else
	{
		if (m_networks->elementAt(i+m_viewStart).ssid.length() > 19)
		{
			m_display->print(m_networks->elementAt(i+m_viewStart).ssid.substring(0,4));
			m_display->print("...");
			m_display->print(m_networks->elementAt(i+m_viewStart).ssid.substring(m_networks->elementAt(i+m_viewStart).ssid.length()-12));
		} else
		{
			m_display->print(m_networks->elementAt(i+m_viewStart).ssid);
		}

	}
}
m_display->display();

}

MenuItem* MenuItem_Adjust_SSID_Screen::Loop () {
  return this;
}

MenuItem* MenuItem_Adjust_SSID_Screen::Update (int rotation, bool pressed) {
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
			if (m_selected == (m_elementSize-1))
			{
				return m_submenu;
			} else
			{
				//update configuration with new ssid
				AppSettings.ssid = m_networks->elementAt(m_selected).ssid;
				return m_submenu;
			}
		}
		pressTime = 0;
	}
	redraw();
	return this;
}
void MenuItem_Adjust_SSID_Screen::Init () {
	pressTime = 0;
	m_elementSize = 1;
	if (m_networks != null)
	{
		m_elementSize = m_networks->count() + 1;
	}
	m_selected = 0;
	m_viewStart = 0;
	m_viewEnd = 6;
	redraw();
}
void MenuItem_Adjust_SSID_Screen::DeInit () {
  ;
}

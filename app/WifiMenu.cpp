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
}

void GetSSIDList(void)
{
	WifiStation.startScan(networkScanCompleted);
}


const String Ssid_name = "SSID";
//MenuItem_Adjust_ts_Screen scr_adjSsid(&Ssid_name);
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
	AppSettings.ssid.toCharArray(str, AppSettings.ssid.length());
}
SettingsElement Ssid(&Ssid_name, &GetSsidStr, &scr_settings);//&scr_adjSsid);

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
}


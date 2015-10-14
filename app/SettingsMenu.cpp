/*
 * SettingsMenu.cpp
 *
 *  Created on: 14 okt 2015
 *      Author: linus
 */

#include "application.h"
#include "SettingsMenu.h"
#include "AppSettings.h"
#include "temperature.h"
#include "WifiMenu.h"
#include "MenuItem_Adjust_d_Screen.h"
#include "MenuItem_Adjust_ts_Screen.h"

const String name = " Settings          ";
MenuItem_SettingsScreen scr_settings(&name);


const String ConsKp_name = "ConsKp";
MenuItem_Adjust_d_Screen scr_adjConsKp(&ConsKp_name);
void SetConsKp(double value)
{
	AppSettings.consKp = value;
}
double GetConsKp(void)
{
	return AppSettings.consKp;
}
void GetConsKpStr(char* str)
{
	dtostrf(AppSettings.consKp, 2, 3, str);
}
SettingsElement ConsKp(&ConsKp_name, &GetConsKpStr, &scr_adjConsKp);

const String SensorMode_name = "Temp Mode";
MenuItem_Adjust_TempMode_Screen scr_adjSensorMode(&SensorMode_name);
void SetSensorMode(TEMP_READ_MODE mode, uint8_t id)
{
	tempSensor.setMode(mode,id);
}
TEMP_READ_MODE GetSensorMode(uint8_t* id)
{
	return tempSensor.getMode(id);
}
void GetSensorModeStr(char* str)
{
	uint8_t id;
	TEMP_READ_MODE mode = tempSensor.getMode(&id);

	switch (mode)
	{
		case TEMP_READ_MODE::MAX:
			strcpy(str,"Max");
			break;
		case TEMP_READ_MODE::MIN:
			strcpy(str,"Min");
			break;
		case TEMP_READ_MODE::MEAN:
			strcpy(str,"Mean");
			break;
		case TEMP_READ_MODE::SENSORID:
			strcpy(str,"Sens ");
			char buff[3];
			itoa(id, buff, 10);
			strcat(str, buff);
			break;
	}
	//dtostrf(consKi, 2, 3, str);
}
SettingsElement SensorMode(&SensorMode_name, &GetSensorModeStr, &scr_adjSensorMode);

const String ConsKi_name = "ConsKi";
MenuItem_Adjust_d_Screen scr_adjConsKi(&ConsKi_name);
void SetConsKi(double value)
{
	AppSettings.consKi = value;
}
double GetConsKi(void)
{
	return AppSettings.consKi;
}
void GetConsKiStr(char* str)
{
	dtostrf(AppSettings.consKi, 2, 3, str);
}
SettingsElement ConsKi(&ConsKi_name, &GetConsKiStr, &scr_adjConsKi);

const String ConsKd_name = "ConsKd";
MenuItem_Adjust_d_Screen scr_adjConsKd(&ConsKd_name);
void SetConsKd(double value)
{
	AppSettings.consKd = value;
}
double GetConsKd(void)
{
	return AppSettings.consKd;
}
void GetConsKdStr(char* str)
{
	dtostrf(AppSettings.consKd, 2, 3, str);
}
SettingsElement ConsKd(&ConsKd_name, &GetConsKdStr, &scr_adjConsKd);

const String AggKp_name = "AggKp";
MenuItem_Adjust_d_Screen scr_adjAggKp(&AggKp_name);
void SetAggKp(double value)
{
	AppSettings.aggKp = value;
}
double GetAggKp(void)
{
	return AppSettings.aggKp;
}
void GetAggKpStr(char* str)
{
	dtostrf(AppSettings.aggKp, 2, 3, str);
}
SettingsElement AggKp(&AggKp_name, &GetAggKpStr, &scr_adjAggKp);

const String AggKi_name = "AggKi";
MenuItem_Adjust_d_Screen scr_adjAggKi(&AggKi_name);
void SetAggKi(double value)
{
	AppSettings.aggKi = value;
}
double GetAggKi(void)
{
	return AppSettings.aggKi;
}
void GetAggKiStr(char* str)
{
	dtostrf(AppSettings.aggKi, 2, 3, str);
}
SettingsElement AggKi(&AggKi_name, &GetAggKiStr, &scr_adjAggKi);

const String AggKd_name = "AggKd";
MenuItem_Adjust_d_Screen scr_adjAggKd(&AggKd_name);
void SetAggKd(double value)
{
	AppSettings.aggKd = value;
}
double GetAggKd(void)
{
	return AppSettings.aggKd;
}
void GetAggKdStr(char* str)
{
	dtostrf(AppSettings.aggKd, 2, 3, str);
}
SettingsElement AggKd(&AggKd_name, &GetAggKdStr, &scr_adjAggKd);

const String PidTime_name = "Reg.Period";
MenuItem_Adjust_ts_Screen scr_adjPidTime(&PidTime_name);
void SetPidTime(int32_t value)
{
	AppSettings.pidPeriod = value;
}
int32_t GetPidTime(void)
{
	return AppSettings.pidPeriod;
}
void GetPidTimeStr(char* str)
{
	itoa((int)AppSettings.pidPeriod, str, 10);
}
SettingsElement PidTime(&PidTime_name, &GetPidTimeStr, &scr_adjPidTime);

const String save_name = "Save Settings";
void SaveSettings()
{
	AppSettings.save();
}
void LoadSettings()
{
	AppSettings.load();
}
SettingsElement Save(&save_name, null, &scr_main, &SaveSettings);

const String restore_name = "Restore Settings";
SettingsElement Restore(&restore_name, null, &scr_main, &LoadSettings);

const String exit_name = "Exit";
SettingsElement Exit(&exit_name, &scr_main);

const String wifisettings_name = "Wifi Settings";
SettingsElement WifiSettings(&wifisettings_name, &scr_WifiSettings);


SettingsElement* settingsElements[] = {&ConsKp, &ConsKi, &ConsKd, &AggKp, &AggKi, &AggKd, &PidTime, &SensorMode, &Restore, &Save, &WifiSettings, &Exit};

void SettingsMenuConfig(Adafruit_SSD1306 *display)
{
	scr_adjSensorMode.Config(display, &scr_settings, &GetSensorMode, &SetSensorMode,&tempSensor);
	scr_adjConsKp.Config(display, &scr_settings, &GetConsKp, &SetConsKp);
	scr_adjConsKi.Config(display, &scr_settings, &GetConsKi, &SetConsKi);
	scr_adjConsKd.Config(display, &scr_settings, &GetConsKd, &SetConsKd);
	scr_adjAggKp.Config(display, &scr_settings, &GetAggKp, &SetAggKp);
	scr_adjAggKi.Config(display, &scr_settings, &GetAggKi, &SetAggKi);
	scr_adjAggKd.Config(display, &scr_settings, &GetAggKd, &SetAggKd);
	scr_adjPidTime.Config(display, &scr_settings, &GetPidTime, &SetPidTime,1000,100000);
	scr_settings.Config(display, settingsElements, (sizeof(settingsElements)/sizeof(*settingsElements)));
}


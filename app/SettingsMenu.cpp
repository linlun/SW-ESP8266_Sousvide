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
#include "MenuItem_Adjust_Enum_Screen.h"
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

const String RegulatorType_name = "Reg. Type";
MenuItem_Adjust_Enum_Screen scr_adjRegulatorType(&RegulatorType_name);
void SetRegulatorType(int value)
{
	AppSettings.usedRegulator = (RegulatorTypes)value;
}
int GetRegulatorType()
{
	return AppSettings.usedRegulator;
}

String regulatorTypes[3] = {"PID", "Adaptive","PID only"};
void GetRegulatorTypeStr(char* str)
{
	//regulatorTypes[AppSettings.usedRegulator].toCharArray(str, regulatorTypes[AppSettings.usedRegulator]->length)
	strcpy(str,regulatorTypes[AppSettings.usedRegulator].c_str());
}
SettingsElement RegulatorType(&RegulatorType_name, &GetRegulatorTypeStr, &scr_adjRegulatorType);


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

const String OuterRange_name = "OuterRange";
MenuItem_Adjust_d_Screen scr_adjOuterRange(&OuterRange_name);
void SetOuterRange(double value)
{
	AppSettings.OuterRange = value;
}
double GetOuterRange(void)
{
	return AppSettings.OuterRange;
}
void GetOuterRangeStr(char* str)
{
	dtostrf(AppSettings.OuterRange, 2, 3, str);
}
SettingsElement OuterRange(&OuterRange_name, &GetOuterRangeStr, &scr_adjOuterRange);

const String InnerRange_name = "InnerRange";
MenuItem_Adjust_d_Screen scr_adjInnerRange(&InnerRange_name);
void SetInnerRange(double value)
{
	AppSettings.InnerRange = value;
}
double GetInnerRange(void)
{
	return AppSettings.InnerRange;
}
void GetInnerRangeStr(char* str)
{
	dtostrf(AppSettings.InnerRange, 2, 3, str);
}
SettingsElement InnerRange(&InnerRange_name, &GetInnerRangeStr, &scr_adjInnerRange);

const String Factor_name = "Factor";
MenuItem_Adjust_d_Screen scr_adjFactor(&Factor_name);
void SetFactor(double value)
{
	AppSettings.Factor = value;
}
double GetFactor(void)
{
	return AppSettings.Factor;
}
void GetFactorStr(char* str)
{
	dtostrf(AppSettings.Factor, 2, 3, str);
}
SettingsElement Factor(&Factor_name, &GetFactorStr, &scr_adjFactor);

const String Offset_name = "Offset";
MenuItem_Adjust_d_Screen scr_adjOffset(&Offset_name);
void SetOffset(double value)
{
	AppSettings.Offset = value;
}
double GetOffset(void)
{
	return AppSettings.Offset;
}
void GetOffsetStr(char* str)
{
	dtostrf(AppSettings.Offset, 2, 3, str);
}
SettingsElement Offset(&Offset_name, &GetOffsetStr, &scr_adjOffset);



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


SettingsElement* settingsElements[] = {&ConsKp, &ConsKi, &ConsKd, &AggKp, &AggKi, &AggKd, &PidTime, &SensorMode, &RegulatorType, &OuterRange, &InnerRange, &Offset, &Factor, &Restore, &Save, &WifiSettings, &Exit};

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
	scr_adjOuterRange.Config(display, &scr_settings, &GetOuterRange, &SetOuterRange);
	scr_adjInnerRange.Config(display, &scr_settings, &GetInnerRange, &SetInnerRange);
	scr_adjOffset.Config(display, &scr_settings, &GetOffset, &SetOffset);
	scr_adjFactor.Config(display, &scr_settings, &GetFactor, &SetFactor);
	scr_adjRegulatorType.Config(display,&scr_settings,regulatorTypes,3,&GetRegulatorType, &SetRegulatorType);

	scr_settings.Config(display, settingsElements, (sizeof(settingsElements)/sizeof(*settingsElements)));
}


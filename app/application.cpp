#include <user_config.h>
#include <rotary.h>
#include <PID_v1.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <WS2812.h>

#include <MenuItem_MainScreen.h>
#include <MenuItem_SettingsScreen.h>
#include <MenuItem_Adjust_d_Screen.h>
#include <MenuItem_Adjust_ts_Screen.h>

#include <stdlib.h>
#include <SlowPWM.h>

#include <config.h>

#include <temperature.h>


/* PID regulator */
//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Define the aggressive and conservative Tuning Parameters
double aggKp=4, aggKi=0.2, aggKd=1;
double consKp=10000, consKi=0.053, consKd=2.254576;
uint32_t pidPeriod=10000;
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);


// For i2c oled module.
// Default i2c pins 0 and 2


Adafruit_SSD1306 display(PIN_OLED_SCL,PIN_OLED_SDA);
 void pollEncoder(void);
 Rotary rotary(PIN_ROTARY_A,PIN_ROTARY_B,PIN_ROTARY_SW, &pollEncoder);


 OneWire ds(PIN_DS18B20);
 temperature tempSensor(&ds);

 void runTempSensor(void)
 {
 	tempSensor.run();
 }

const String name = "Settings           ";
MenuItem_SettingsScreen scr_settings(&name);
MenuItem_MainScreen scr_main;

unsigned long TimerCounter = 0;


void pollEncoder(void)
{
	rotary.pollEncoder();
}

void PwmUpdate(void);
SlowPWM outputPwm(PIN_RELAY,1000,&PwmUpdate);

void PwmUpdate(void)
{
	//Serial.println("ext timer int");
	outputPwm.timerInt();
}


void SetTimer(unsigned long value)
{
	TimerCounter = (unsigned long) value;
}
unsigned long GetTimer(void)
{
	return (unsigned long)TimerCounter;
}

void SetPidMode(uint8_t mode)
{
	if (mode == MANUAL)
	{
		outputPwm.SetDuty( 0 );
		Output = 0;
	}
	myPID.SetMode((int)mode);
}

uint8_t GetPidMode(void)
{
	return (uint8_t)myPID.GetMode();
}

void SetPidSetPoint(float value)
{
	Setpoint = (double) value;
}
float GetPidSetPoint(void)
{
	return (float)Setpoint;
}
float GetPidCurrent(void)
{
	return (float)Input;
}


const String ConsKp_name = "ConsKp";
MenuItem_Adjust_d_Screen scr_adjConsKp(&ConsKp_name);
void SetConsKp(double value)
{
	consKp = value;
}
double GetConsKp(void)
{
	return consKp;
}
void GetConsKpStr(char* str)
{
	dtostrf(consKp, 2, 3, str);
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
	consKi = value;
}
double GetConsKi(void)
{
	return consKi;
}
void GetConsKiStr(char* str)
{
	dtostrf(consKi, 2, 3, str);
}
SettingsElement ConsKi(&ConsKi_name, &GetConsKiStr, &scr_adjConsKi);

const String ConsKd_name = "ConsKd";
MenuItem_Adjust_d_Screen scr_adjConsKd(&ConsKd_name);
void SetConsKd(double value)
{
	consKd = value;
}
double GetConsKd(void)
{
	return consKd;
}
void GetConsKdStr(char* str)
{
	dtostrf(consKd, 2, 3, str);
}
SettingsElement ConsKd(&ConsKd_name, &GetConsKdStr, &scr_adjConsKd);

const String AggKp_name = "AggKp";
MenuItem_Adjust_d_Screen scr_adjAggKp(&AggKp_name);
void SetAggKp(double value)
{
	aggKp = value;
}
double GetAggKp(void)
{
	return aggKp;
}
void GetAggKpStr(char* str)
{
	dtostrf(aggKp, 2, 3, str);
}
SettingsElement AggKp(&AggKp_name, &GetAggKpStr, &scr_adjAggKp);

const String AggKi_name = "AggKi";
MenuItem_Adjust_d_Screen scr_adjAggKi(&AggKi_name);
void SetAggKi(double value)
{
	aggKi = value;
}
double GetAggKi(void)
{
	return aggKi;
}
void GetAggKiStr(char* str)
{
	dtostrf(aggKi, 2, 3, str);
}
SettingsElement AggKi(&AggKi_name, &GetAggKiStr, &scr_adjAggKi);

const String AggKd_name = "AggKd";
MenuItem_Adjust_d_Screen scr_adjAggKd(&AggKd_name);
void SetAggKd(double value)
{
	aggKd = value;
}
double GetAggKd(void)
{
	return aggKd;
}
void GetAggKdStr(char* str)
{
	dtostrf(aggKd, 2, 3, str);
}
SettingsElement AggKd(&AggKd_name, &GetAggKdStr, &scr_adjAggKd);

const String PidTime_name = "Reg.Pe2riod";
MenuItem_Adjust_ts_Screen scr_adjPidTime(&PidTime_name);
void SetPidTime(int32_t value)
{
	pidPeriod = value;
}
int32_t GetPidTime(void)
{
	return pidPeriod;
}
void GetPidTimeStr(char* str)
{
	itoa(pidPeriod, str, 10);
}
SettingsElement PidTime(&PidTime_name, &GetPidTimeStr, &scr_adjPidTime);


void GetExit(char* str)
{
	str[0]=0;
}
const String exit_name = "Exit";
SettingsElement Exit(&exit_name, &GetExit, &scr_main);

SettingsElement* settingsElements[] = {&ConsKp, &ConsKi, &ConsKd, &AggKp, &AggKi, &AggKd, &PidTime, &SensorMode, &Exit};


MenuItem* currentMenu;
Timer procTimer;
bool state = true;

Timer rotaryTimer;
Timer menuTimer;
Timer tempTimer;


void menuFunction(void)
{
	static boolean last_switchsts = false;
	int8_t positionDiff = rotary.GetPositiondiff();
	boolean switchsts = rotary.GetSwitch();
	if (positionDiff != 0 || switchsts != last_switchsts)
	{
		MenuItem* activeMenu = currentMenu;
		//Serial.print("Index: ");
		//Serial.print(rotary.GetPosition(), DEC);
		//Serial.print(" Diff: ");
		//Serial.println(positionDiff, DEC);
		currentMenu = activeMenu->Update(positionDiff, switchsts);
		if (currentMenu != activeMenu)
		{
			activeMenu->DeInit();
			currentMenu->Init();
		}
		last_switchsts = switchsts;
	} else
	{
		MenuItem* activeMenu = currentMenu;
		currentMenu = activeMenu->Loop();
		if (currentMenu != activeMenu)
		{
			activeMenu->DeInit();
			currentMenu->Init();
		}
	}

}

float pidConservativeLimit = 5.0;
void CalculatePID()
{
	Input = tempSensor.get();
  double gap = abs(Setpoint-Input); //distance away from setpoint
  if (gap < pidConservativeLimit)
  {  //we're close to setpoint, use conservative tuning parameters
	myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
	 //we're far from setpoint, use aggressive tuning parameters
	 myPID.SetTunings(aggKp, aggKi, aggKd);
  }

  myPID.Compute();
  outputPwm.SetDuty( Output/255);
}

void init()
{
	//initialize the variables we're linked to
	Input = 0;
	Setpoint = 58;
//	pinMode(PIN_LED_WS2812, OUTPUT);

	outputPwm.SetDuty(0);
	outputPwm.Start();
//turn the PID on
	myPID.SetMode(AUTOMATIC);
	scr_adjSensorMode.Config(&display, &scr_settings, &GetSensorMode, &SetSensorMode,&tempSensor);
	scr_adjConsKp.Config(&display, &scr_settings, &GetConsKp, &SetConsKp);
	scr_adjConsKi.Config(&display, &scr_settings, &GetConsKi, &SetConsKi);
	scr_adjConsKd.Config(&display, &scr_settings, &GetConsKd, &SetConsKd);
	scr_adjAggKp.Config(&display, &scr_settings, &GetAggKp, &SetAggKp);
	scr_adjAggKi.Config(&display, &scr_settings, &GetAggKi, &SetAggKi);
	scr_adjAggKd.Config(&display, &scr_settings, &GetAggKd, &SetAggKd);
	scr_adjPidTime.Config(&display, &scr_settings, &GetPidTime, &SetPidTime,1000,100000);
	scr_main.Config(&display, &scr_settings, &GetPidCurrent, &GetPidSetPoint, &SetPidSetPoint, &GetTimer, &SetTimer, &GetPidMode, &SetPidMode  );
	scr_settings.Config(&display, settingsElements, (sizeof(settingsElements)/sizeof(*settingsElements)));
	//currentMenu = &scr_settings;
	currentMenu = &scr_main;
	WDT.enable(false); // First (but not the best) option: fully disable watch dog timer
//
//
	char buffer1[] = "\x40\x00\x00\x00\x40\x00\x00\x00\x40";
	ws2812_writergb(PIN_LED_WS2812, buffer1, sizeof(buffer1));
//
	Serial.println("Display start");
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
	display.display();
	Serial.println("Started");
	char buffer2[] = "\x00\x40\x40\x40\x00\x40\x40\x40\x00";
    ws2812_writergb(PIN_LED_WS2812, buffer2, sizeof(buffer2));
//
//
	delay(500);
	display.clearDisplay();
//	Serial.println("fill");
//	// draw a circle, 10 pixel radius
	display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
	display.display();
	delay(500);
	display.clearDisplay();
	display.display();
	currentMenu->Init();
	currentMenu->Update(0,false);

	tempSensor.init();
	tempTimer.initializeMs(1000, runTempSensor).start();

	menuTimer.initializeMs(50, menuFunction).start();
	procTimer.initializeMs(100, CalculatePID).start();

	char buffer3[] = "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF";
	    ws2812_writergb(PIN_LED_WS2812, buffer3, sizeof(buffer3));
}

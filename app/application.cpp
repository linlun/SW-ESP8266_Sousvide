#include <application.h>
#include <rotary.h>
#include <PID_v1.h>

#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <WS2812.h>
#include <AppSettings.h>
#include <MenuItem_MainScreen.h>
#include <MenuItem_SettingsScreen.h>
#include <MenuItem_Adjust_d_Screen.h>
#include <MenuItem_Adjust_ts_Screen.h>
#include <SlowPWM.h>
#include <temperature.h>
#include <WifiMenu.h>
#include <SettingsMenu.h>


/* PID regulator */
//Define Variables we'll be connecting to
double Input, Output;
ApplicationSettingsStorage AppSettings;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &AppSettings.Setpoint, AppSettings.consKp, AppSettings.consKi, AppSettings.consKd, DIRECT);


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
	AppSettings.Setpoint = (double) value;
}
float GetPidSetPoint(void)
{
	return (float)AppSettings.Setpoint;
}
float GetPidCurrent(void)
{
	return (float)Input;
}

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

void CalculatePID()
{
	Input = tempSensor.get();
  double gap = abs(AppSettings.Setpoint-Input); //distance away from setpoint
  if (gap < AppSettings.pidConservativeLimit)
  {  //we're close to setpoint, use conservative tuning parameters
	myPID.SetTunings(AppSettings.consKp, AppSettings.consKi, AppSettings.consKd);
  }
  else
  {
	 //we're far from setpoint, use aggressive tuning parameters
	 myPID.SetTunings(AppSettings.aggKp, AppSettings.aggKi, AppSettings.aggKd);
  }

  myPID.Compute();
  outputPwm.SetDuty( Output/255);
}

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		Serial.println("Failed to scan networks");
		return;
	}

	for (int i = 0; i < list.count(); i++)
	{
		Serial.print("\tWiFi: ");
		Serial.print(list[i].ssid);
		Serial.print(", ");
		Serial.print(list[i].getAuthorizationMethodName());
		if (list[i].hidden) Serial.print(" (hidden)");
		Serial.println();
	}
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	debugf("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

FTPServer ftp;

String network, password;
Timer connectionTimer;


void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when system initialization was completed
void startServers()
{
	startFTP();
	//startWebServer();
}


void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	//Define the aggressive and conservative Tuning Parameters
	if (!AppSettings.exist())
	{
		Serial.println("Created Appsettings");
		AppSettings.aggKp=4;
		AppSettings.aggKi=0.2;
		AppSettings.aggKd=1;
		AppSettings.consKp=10000;
		AppSettings.consKi=0.053;
		AppSettings.consKd=2.254576;
		AppSettings.pidPeriod=10000;
		AppSettings.Setpoint = 58;
		AppSettings.pidConservativeLimit = 5.0;
		AppSettings.ssid = "Linuxz";
		AppSettings.password ="asdfghjkl";
		AppSettings.save();
	}
	Serial.print("Loading Appsettings...");
	AppSettings.load();
	Serial.println("done");
	Serial.print("Main SSID: ");
	Serial.println(AppSettings.ssid);
	char buffer[30];
	   sprintf(buffer, "with %%p:  x    = %p\n", AppSettings);
	   Serial.print(buffer);
	//initialize the variables we're linked to
	Input = 0;

//	pinMode(PIN_LED_WS2812, OUTPUT);

	outputPwm.SetDuty(0);
	outputPwm.Start();
//turn the PID on
	myPID.SetMode(AUTOMATIC);

	SettingsMenuConfig(&display);
	WifiMenuConfig(&display);

	scr_main.Config(&display, &scr_settings, &GetPidCurrent, &GetPidSetPoint, &SetPidSetPoint, &GetTimer, &SetTimer, &GetPidMode, &SetPidMode  );


	currentMenu = &scr_settings;
	//currentMenu = &scr_main;
	//WDT.enable(false); // First (but not the best) option: fully disable watch dog timer
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
    Serial.systemDebugOutput(true); // Enable debug output to serial

    // Soft access point
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sous vide", "", AUTH_OPEN);

	// Station - WiFi client
	WifiStation.enable(true);
	if (AppSettings.exist())
	{
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
	}
	//WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	// Optional: Change IP addresses (and disable DHCP)
	//WifiAccessPoint.setIP(IPAddress(192, 168, 2, 1));
	//WifiStation.setIP(IPAddress(192, 168, 1, 171));


	// Print available access points
	//WifiStation.startScan(listNetworks); // In Sming we can start network scan from init method without additional code

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start

	// Run WEB server on system ready
	System.onReady(startServers);
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

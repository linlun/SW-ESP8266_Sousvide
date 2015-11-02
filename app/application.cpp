#include <application.h>
#include <rotary.h>
#include <PID_v1.h>

#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <WS2812.h>
#include "AppSettings.h"
#include <MenuItem_MainScreen.h>
#include <MenuItem_SettingsScreen.h>
#include <MenuItem_Adjust_d_Screen.h>
#include <MenuItem_Adjust_ts_Screen.h>
#include <SlowPWM.h>
#include <temperature.h>
#include <WifiMenu.h>
#include <SettingsMenu.h>
#include <AdaptiveRegulator.h>
//#define DONOTUSEPID	1

enum RegulatorTypes { REG_PID, REG_ADAPTIVE };
RegulatorTypes usedRegulator = REG_ADAPTIVE;

/* PID regulator */
//Define Variables we'll be connecting to
float Input, Output;
ApplicationSettingsStorage AppSettings;

#ifndef DONOTUSEPID
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &AppSettings.Setpoint, AppSettings.consKp, AppSettings.consKi, AppSettings.consKd, DIRECT);
#endif

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

AdaptiveRegulator myReg(PIN_RELAY,&Input, &AppSettings.Setpoint);

#ifndef DONOTUSEPID
void PwmUpdate(void);
SlowPWM outputPwm(PIN_RELAY,1000,&PwmUpdate);



void PwmUpdate(void)
{
	//Serial.println("ext timer int");
	outputPwm.timerInt();
}
#endif

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
#ifndef DONOTUSEPID
		outputPwm.SetDuty( 0 );
#endif
		Output = 0;
	}
#ifndef DONOTUSEPID
	myPID.SetMode((int)mode);
#endif
}

uint8_t GetPidMode(void)
{
#ifndef DONOTUSEPID
	return (uint8_t)myPID.GetMode();
#else
	return 1;
#endif
}

void SetPidSetPoint(float value)
{
	AppSettings.Setpoint = (float) value;
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

void CalculateRegulator()
{
	float gap;
	Input = tempSensor.get();
	switch (usedRegulator)
	{
	case REG_ADAPTIVE:
		if (Input > 20.0)
		{
			myReg.Compute();
		}
		break;
	case REG_PID:
#ifndef DONOTUSEPID
		gap = abs(AppSettings.Setpoint-Input); //distance away from setpoint
		if (gap < AppSettings.pidConservativeLimit)
		{
			//we're close to setpoint, use conservative tuning parameters
			myPID.SetTunings(AppSettings.consKp, AppSettings.consKi, AppSettings.consKd);
		}
		else
		{
			//we're far from setpoint, use aggressive tuning parameters
			myPID.SetTunings(AppSettings.aggKp, AppSettings.aggKi, AppSettings.aggKd);
		}

		myPID.Compute();
		outputPwm.SetDuty( Output/255);
#endif
		break;
	default:
		break;
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
HttpServer server;
String network, password;
Timer connectionTimer;


void startFTP()
{
	//if (!fileExist("index.html"))
	//	fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	response.sendTemplate(tmpl); // will be automatically deleted
}

void onIpConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
	auto &vars = tmpl->variables();

	bool dhcp = WifiStation.isEnabledDHCP();
	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

	if (!WifiStation.getIP().isNull())
	{
		vars["ip"] = WifiStation.getIP().toString();
		vars["netmask"] = WifiStation.getNetworkMask().toString();
		vars["gateway"] = WifiStation.getNetworkGateway().toString();
	}
	else
	{
		vars["ip"] = "192.168.1.77";
		vars["netmask"] = "255.255.255.0";
		vars["gateway"] = "192.168.1.1";
	}

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json["network"]= WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++)
	{
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}
void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
}
void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if (updating && connectingNow)
	{
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

//const char* names[] = { "sensor_0", "sensor_1", "sensor_2", "sensor_3", "sensor_4", "sensor_5" };
void onApiSensors(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;
	json["heap"] = system_get_free_heap_size();
	JsonObject& sensors = json.createNestedObject("sensors");
	sensors["temperature"] = tempSensor.get();
	for( int i = 0; i < tempSensor.numSensors; i++)
	{
		char buff[3];
		itoa(i, buff, 10);
		String desiredString = "sensor_";
		desiredString += buff;
		sensors[desiredString] = tempSensor.get(i);
		//sensors[names[i]] = tempSensor.get(i);
	}
	JsonObject& regulator = json.createNestedObject("reg");
	switch (usedRegulator)
		{
		case REG_ADAPTIVE:
			regulator["Type"] = "Adaptive";
			regulator["Setpoint"] = AppSettings.Setpoint;
			regulator["State"] = myReg.opState;
			if (myReg.isHeatOn)
				regulator["output"] = 255.0;
			else
				regulator["output"] = 0;
			break;
		case REG_PID:
#ifndef DONOTUSEPID
			regulator["Type"] = "PID";
			regulator["Setpoint"] = AppSettings.Setpoint;
			regulator["Kp"] = myPID.GetKp();
			regulator["Ki"] = myPID.GetKi();
			regulator["Kd"] = myPID.GetKd();
			regulator["aggKp"] = AppSettings.aggKp;
			regulator["aggKi"] = AppSettings.aggKi;
			regulator["aggKd"] = AppSettings.aggKd;
			regulator["consKp"] = AppSettings.consKp;
			regulator["consKi"] = AppSettings.consKi;
			regulator["consKd"] = AppSettings.consKd;
			regulator["pidPeriod"] = AppSettings.pidPeriod;
			regulator["pidConservativeLimit"] = AppSettings.pidConservativeLimit;
			regulator["output"] = Output;
			regulator["PTerm"] = myPID.GetKpTerm();
			regulator["ITerm"] = myPID.GetKiTerm();
			regulator["DTerm"] = myPID.GetKdTerm();
#endif
			break;
		default:
			break;
		}

	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ipconfig", onIpConfig);
	server.addPath("/ajax/get-networks", onAjaxNetworkList);
	server.addPath("/ajax/connect", onAjaxConnect);
	server.addPath("/sensors", onApiSensors);
	server.setDefaultHandler(onFile);
}


// Will be called when system initialization was completed
void startServers()
{
	startFTP();
	startWebServer();
}


void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	//Define the aggressive and conservative Tuning Parameters
	if (!AppSettings.exist())
	{
		Serial.println("Created Appsettings");
		AppSettings.aggKp=2000;
		AppSettings.aggKi=0;
		AppSettings.aggKd=10;
		AppSettings.consKp=500;
		AppSettings.consKi=0.3;
		AppSettings.consKd=0;
		AppSettings.pidPeriod=15000;
		AppSettings.Setpoint = 48;
		AppSettings.pidConservativeLimit = 5.0;
		AppSettings.ssid = WIFI_SSID;
		AppSettings.password = WIFI_PASS;
		AppSettings.save();
	}
	Serial.print("Loading Appsettings...");
	AppSettings.load();
	Serial.println("done");
	Serial.print("Main SSID: ");
	Serial.println(AppSettings.ssid);
	Serial.print("Main password: ");
		Serial.println(AppSettings.password);
	char buffer[30];
	   sprintf(buffer, "with %%p:  x    = %p\n", AppSettings);
	   Serial.print(buffer);
	//initialize the variables we're linked to
	Input = 0;

//	pinMode(PIN_LED_WS2812, OUTPUT);
#ifndef DONOTUSEPID
if (usedRegulator == REG_PID)
{
	outputPwm.SetDuty(0);
	outputPwm.Start();
//turn the PID on
	myPID.SetMode(AUTOMATIC);
}
#endif
if (usedRegulator == REG_ADAPTIVE)
{
	;
}
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
    Serial.systemDebugOutput(false); // Enable debug output to serial

    // Soft access point
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sous vide", "", AUTH_OPEN);

	// Station - WiFi client
	WifiStation.enable(true);
	if (AppSettings.exist())
	{
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		Serial.println("Found appsettings");
		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
		{
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
			Serial.println("Using static ip");
		}
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
	tempSensor.init();
		tempTimer.initializeMs(1000, runTempSensor).start();

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



	menuTimer.initializeMs(50, menuFunction).start();
	procTimer.initializeMs(100, CalculateRegulator).start();

	char buffer3[] = "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF";
	ws2812_writergb(PIN_LED_WS2812, buffer3, sizeof(buffer3));
}

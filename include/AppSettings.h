/*
 * AppSettings.h
 *
 *  Created on: 13 мая 2015 г.
 *      Author: Anakod
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE "settings.conf" // leading point for security reasons :)

struct ApplicationSettingsStorage
{
	String ssid;
	String password;

	bool dhcp = true;

	IPAddress ip;
	IPAddress netmask;
	IPAddress gateway;

	float Setpoint;
	//Define the aggressive and conservative Tuning Parameters
	float aggKp;
	float aggKi;
	float aggKd;
	float consKp;
	float consKi;
	float consKd;
	uint32_t pidPeriod;
	float pidConservativeLimit;

	void load()
	{
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& network = root["network"];
			ssid = String((const char*)network["ssid"]);
			password = String((const char*)network["password"]);

			dhcp = network["dhcp"];

			ip = String((const char*)network["ip"]);
			netmask = String((const char *)network["netmask"]);
			gateway = String((const char *)network["gateway"]);

			JsonObject& pid = root["pid"];

			Setpoint = pid["Setpoint"];
			aggKp = pid["aggKp"];
			aggKi = pid["aggKi"];
			aggKd = pid["aggKd"];
			consKp = pid["consKp"];
			consKi = pid["consKi"];
			consKd = pid["consKd"];
			pidPeriod = pid["pidPeriod"];
			pidConservativeLimit = pid["pidConservativeLimit"];

			delete[] jsonString;
		}
	}

	void save()
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& network = jsonBuffer.createObject();
		root["network"] = network;
		network["ssid"] = ssid.c_str();
		network["password"] = password.c_str();

		network["dhcp"] = dhcp;

		// Make copy by value for temporary string objects
		network["ip"] = ip.toString();
		network["netmask"] = netmask.toString();
		network["gateway"] = gateway.toString();


		JsonObject& pid = jsonBuffer.createObject();
		root["pid"] = pid;
		pid["Setpoint"] = Setpoint;
		pid["aggKp"] = aggKp;
		pid["aggKi"] = aggKi;
		pid["aggKd"] = aggKd;
		pid["consKp"] = consKp;
		pid["consKi"] = consKi;
		pid["consKd"] = consKd;
		pid["pidPeriod"] = pidPeriod;
		pid["pidConservativeLimit"] = pidConservativeLimit;


		//TODO: add direct file stream writing
		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};



#endif /* INCLUDE_APPSETTINGS_H_ */

# SW-ESP8266_Sousvide
Software for a sous vide controller based on ESP8266
1.1 Software
The software is created with the Sming framework for the ESP8266, to set up the environment the follwoing steps is needed (Also described on https://github.com/anakod/Sming/wiki/Windows-Quickstart): 
1. Install Chocolatey
Open an adminstrative command window (search for cmd in the search window on the windows start menu, right click and choos run as administrator) paste and run the following in the console window:
@powershell -NoProfile -ExecutionPolicy unrestricted -Command "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))" && SET PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin
2. Add the sming package:
Paste and run:
choco sources add -name kireevco -source 'https://www.myget.org/F/kireevco-chocolatey/'
3. Install Sming, this takes quite some time and will install alot off packages:
Paste and run:
choco install sming -y -source 'https://www.myget.org/F/kireevco-chocolatey/'
4. Open eclipse and create a workspace folder, choose any you like.
Click on arrow to show the workspace, right click  in the project explorer and choose “import”, General, Existing project into workspace.
Browse to c:\tools\sming and select that folder, select Finish
5. Build the sming framework
Go the the right tab in eclipse and select make targets, then find and expand SmingFramework, double click on “all” and make sure it builds correctly.
6. Import the Sous vide code.
In the project explorer and right click and choose “import”, General, Existing project into workspace.
Browse to the checked out source code and select that folder, select Finish
7. Modify existing driver for SSD1306
The existing driver for the display does not work right of the box, the following changes are needed (open the file Adafruit_SSD1306.cpp in the SmingFramework/Libraries/Adafruit_SSD1306 folder)
1. Find the call to Wire.begin() and just before that line add:
	if (sclk != -1) { Wire.pins(dc ,sclk); }
2. After the constructor “Adafruit_SSD1306::Adafruit_SSD1306(int8_t reset)” that has the comment “// initializer for I2C - we only indicate the reset pin!” a new contstructor is needed:
// initializer for I2C - we only indicate the reset pin! Added by LiLu
Adafruit_SSD1306::Adafruit_SSD1306(int8_t SDA, int8_t SCL) :
Adafruit_GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
sclk = SCL;
dc = SDA;
cs = sid = -1;
rst = 0;
}
3. The command sent in the i2c version for Adafruit_SSD1306::ssd1306_Command is wrong, change “uint8_t control = 0x00;” to “uint8_t control = 0x80;”
4. In the .h file add the new constructor “Adafruit_SSD1306(int8_t SDA, int8_t SCL);“
5. Add:
#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H
To the beginning of the .h file and add:
#endif
To the end.
8. Rebuild the sming framework
To make the changes take affect, rebuild the sming framfork.

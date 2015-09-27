/*
 * temperature.h
 *
 *  Created on: 23 aug 2015
 *      Author: Linus
 */

#ifndef APP_TEMPERATURE_H_
#define APP_TEMPERATURE_H_

#include <config.h>
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <stdlib.h>
#include <Libraries/OneWire/OneWire.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

void temperature_init(void);

enum class TEMP_READ_MODE
{
	MIN,
	MAX,
	MEAN,
	SENSORID
};

class temperature {


  public:
	temperature(OneWire* sensor);
	float get(void);
	float get(uint8_t id);
	void init(void);
	void run(void);



	void setMode(TEMP_READ_MODE mode, uint8_t sensorId);
	TEMP_READ_MODE getMode(uint8_t* sensorId);
	byte numSensors = 0u;
  private:
	TEMP_READ_MODE m_mode;
	uint8_t m_sensorid;

	OneWire* m_sensor;
	float m_Temp_celsius[MAX_NUM_DS_SENSORS];

	boolean m_running = false;
	byte m_addr[MAX_NUM_DS_SENSORS][8]; //Allow a maximum of x sensors

};

class MenuItem_Adjust_TempMode_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_TempMode_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, TEMP_READ_MODE (*GetValue)(uint8_t*), void (*SetValue)(TEMP_READ_MODE, uint8_t), temperature* temp);
    //Called periodically
    MenuItem* Loop();
    //called when input changes state
    MenuItem* Update(int rotation, bool pressed);
    //initialize the menu
    void Init();
    //initialize the menu
    void DeInit();
  private:
    void redraw(void);
    Adafruit_SSD1306* m_display;
    const String* m_name;
    TEMP_READ_MODE (*m_GetValue)(uint8_t*);
    void (*m_SetValue)(TEMP_READ_MODE, uint8_t);
    MenuItem* m_submenu;
    unsigned long pressTime;
    TEMP_READ_MODE m_value;
    uint8_t	m_id;
    int8_t	m_state;
    char m_charbuf[8];
    temperature* m_tempsensor;
};

#endif /* APP_TEMPERATURE_H_ */

/*
 * WifiMenu.h
 *
 *  Created on: 14 okt 2015
 *      Author: linus
 */

#ifndef INCLUDE_WIFIMENU_H_
#define INCLUDE_WIFIMENU_H_
#include "MenuItem_SettingsScreen.h"
#include "application.h"
extern MenuItem_SettingsScreen scr_WifiSettings;


void WifiMenuConfig(Adafruit_SSD1306 *display);


class MenuItem_Adjust_SSID_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_SSID_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, BssList* networks);
    //Called periodically
    MenuItem* Loop();
    //called when input changes state
    MenuItem* Update(int rotation, bool pressed);
    //initialize the menu
    void Init();
    //initialize the menu
    void DeInit();
    uint8_t m_elementSize;
  private:
    void redraw(void);
    Adafruit_SSD1306* m_display;
    const String* m_name;
    MenuItem* m_submenu;
    unsigned long pressTime;
    BssList* m_networks;

    int8_t m_selected;
    uint8_t m_viewStart;
    uint8_t m_viewEnd;
};

#endif /* INCLUDE_WIFIMENU_H_ */

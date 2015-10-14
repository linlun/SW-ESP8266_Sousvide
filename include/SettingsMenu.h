/*
 * SettingsMenu.h
 *
 *  Created on: 14 okt 2015
 *      Author: linus
 */

#ifndef INCLUDE_SETTINGSMENU_H_
#define INCLUDE_SETTINGSMENU_H_


#include "MenuItem_SettingsScreen.h"
#include "WifiMenu.h"

extern MenuItem_SettingsScreen scr_settings;
extern const String exit_name;
void SettingsMenuConfig(Adafruit_SSD1306 *display);

#endif /* INCLUDE_SETTINGSMENU_H_ */

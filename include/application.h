/*
 * application.h
 *
 *  Created on: 14 okt 2015
 *      Author: linus
 */

#ifndef INCLUDE_APPLICATION_H_
#define INCLUDE_APPLICATION_H_
#include <user_config.h>
#include <stdlib.h>
#include <config.h>
#include <SmingCore/SmingCore.h>
#include "temperature.h"
#include "MenuItem_MainScreen.h"
#include "AppSettings.h"
extern temperature tempSensor;
extern MenuItem_MainScreen scr_main;
extern ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPLICATION_H_ */


#include "MenuItem_MainScreen.h"
#include <WS2812.h>
#include "config.h"
static float SetTempAdjust;
static unsigned long SetTimerAdjust;

MenuItem_MainScreen::MenuItem_MainScreen() {
	TimerCounter = 0;
}

void MenuItem_MainScreen::Config(Adafruit_SSD1306* display ,MenuItem* submenu, float (*GetTemp)(void), float (*GetSetTemp)(void), void (*SetTemp)(float), unsigned long (*GetTimer)(void), void (*SetTimer)(unsigned long), uint8_t (*GetPidMode)(void), void (*SetPidMode)(uint8_t)) {
	m_display = display;
	m_submenu = submenu;
	m_GetSetTemp = GetSetTemp;
	m_GetTemp = GetTemp;
	m_SetTemp = SetTemp;
	m_GetTimer = GetTimer;
	m_SetTimer = SetTimer;
	m_GetPidMode = GetPidMode;
	m_SetPidMode = SetPidMode;
}

MenuItem* MenuItem_MainScreen::Loop () {
	if (TimerCounter != 0)
	{
		if (TimerCounter - millis() < 2000)
		{
			if (m_GetPidMode())
				m_SetPidMode(0);
			else
				m_SetPidMode(1);
			TimerCounter = 0;
		}
	}
	static uint8_t counter;
	if (counter > 20)
	{
		redraw();
		counter=0;
	}
	counter++;
	if (m_state != 0)
	{
		if (returnTime + 2000 < millis())
		{
			m_state = 0;
			redraw();
		}
	}
	return this;
}

void MenuItem_MainScreen::displayTimer(unsigned long Timleft)
{
	if (Timleft == 0)
	{
		m_display->print(" Off  ");
	} else
	{
		Timleft = Timleft / 60000;
		uint8_t hours = Timleft / 60;
		uint8_t minutes = Timleft - hours*60;
		if (hours > 99)
			hours = 99;
		m_display->print(hours);
		m_display->print("h");
		m_display->print(minutes);
		m_display->print("m");
	}
}
void MenuItem_MainScreen::redraw(void)	// text display tests
{
	float currentSetTemp;
	float currentTemp;
	switch (m_state)
	{
	case 0:
		m_display->clearDisplay();
		m_display->setTextSize(1);
		m_display->setCursor(0,0);
		if (m_GetPidMode())
			m_display->print("ON");
		else
			m_display->print("OFF");

		m_display->setCursor(0,14);
		m_display->print("Now");
		m_display->setTextSize(3);
		m_display->setCursor(20,0);
		currentTemp = m_GetTemp();
		m_display->print(currentTemp);
		m_display->print((char)247);
		m_display->setTextSize(1);
		m_display->setCursor(0,40);
		m_display->print("Set");
		m_display->setTextSize(3);
		m_display->setCursor(20,26);
		currentSetTemp = m_GetSetTemp();
		m_display->print(currentSetTemp);
		m_display->print((char)247);
		m_display->setTextSize(1);
		m_display->setCursor(0,56);
		m_display->print("Timer:");
		m_display->setCursor(40,50);
		m_display->setTextSize(2);
		if (TimerCounter <= millis() )
			displayTimer(TimerCounter - millis());
		else
			displayTimer(0);
		//m_display->print("03h13m");
		m_display->display();
		break;
	case 1:
		m_display->clearDisplay();
		m_display->setTextSize(2);
		m_display->setCursor(15,0);
		m_display->print("Regulator");
		m_display->setCursor(38,18);
		m_display->println("mode:");
		m_display->setTextSize(3);
		if (m_GetPidMode())
		{
			m_display->setCursor(45,40);
			m_display->print("ON");
		}
		else
		{
			m_display->setCursor(40,40);
			m_display->print("OFF");
		}
		m_display->display();
		break;
	case 2:
		SetTempAdjust = m_GetSetTemp();
	case 10:
	case 11:
		m_display->clearDisplay();
		m_display->setTextSize(2);
		m_display->setCursor(47,0);
		m_display->print("Set");
		m_display->setCursor(0,18);
		m_display->println("temperatur");
		m_display->setTextSize(3);
		m_display->setCursor(10,40);
		m_display->print(SetTempAdjust);
		m_display->print((char)247);
		if (m_state == 10)
		{
			m_display->drawFastHLine(10,63,32,WHITE);
		} else if (m_state == 11)
		{
			m_display->drawFastHLine(65,63,32,WHITE);
		}
		m_display->display();
		break;
	case 3:
	case 20:
	case 21:
		m_display->clearDisplay();
		m_display->setTextSize(2);
		m_display->setCursor(47,0);
		m_display->print("Set");
		m_display->setCursor(35,18);
		m_display->println("timer:");
		m_display->setTextSize(3);
		m_display->setCursor(10,40);
		displayTimer(SetTimerAdjust);
		//m_display->print("03h13m");
		if (m_state == 20)
		{
			m_display->drawFastHLine(10,63,32,WHITE);
		} else if (m_state == 21)
		{
			m_display->drawFastHLine(65,63,32,WHITE);
		}
		m_display->display();
		break;
	default:
		m_display->clearDisplay();
		m_display->setTextSize(2);
		m_display->setCursor(20,0);
		m_display->print("Default");
		m_display->display();

	}

}


MenuItem* MenuItem_MainScreen::Update (int rotation, bool pressed) {
	Serial.print("main ");
	returnTime = millis();

	switch (m_state)
	{
	case 10:
		SetTempAdjust+= (float)rotation;
		break;
	case 11:
		SetTempAdjust+= (float)rotation/10;
		break;
	case 20:
		if (rotation < 0)
			if (SetTimerAdjust > -rotation*60000*60)
				SetTimerAdjust += rotation*60000*60;
			else
				;
		else
			SetTimerAdjust += rotation*60000*60;
		break;
	case 21:
		if (rotation < 0)
			if (SetTimerAdjust > -rotation*60000)
				SetTimerAdjust += rotation*60000;
			else
				SetTimerAdjust = 0;
		else
			SetTimerAdjust += rotation*60000;
		break;
	default:
		m_state += rotation;
		if (m_state >= 4)
			m_state -= 4;
		if (m_state < 0)
			m_state += 4;

	}


	Serial.print(m_state);
	if (pressed)
	{
		pressTime = millis();

	}

	if (!pressed & pressTime != 0)
	{
		char buffer3[] = "\xFF\xFF\xFF\xFF\xFF\xFF";
		char buffer4[] = "\xFF\x00\x00\xFF\x00\x00";
		char buffer5[] = "\x00\xFF\x00\x00\xFF\x00";
		unsigned long passedTime = millis() - pressTime ;
		if (passedTime > 60 && passedTime < 300)
		{
			switch (m_state)
			{
			case 0:

				break;
			case 1:

				ws2812_writergb(PIN_LED_WS2812, buffer3, sizeof(buffer3));
				if (m_GetPidMode())
					m_SetPidMode(0);
				else
					m_SetPidMode(1);
				break;
			case 2:

				ws2812_writergb(PIN_LED_WS2812, buffer4, sizeof(buffer4));

				SetTempAdjust = m_GetSetTemp();
				m_state = 10;

				break;
			case 3:

								ws2812_writergb(PIN_LED_WS2812, buffer5, sizeof(buffer5));
				SetTimerAdjust = 16732238;
				m_state = 20;
				break;
			case 10:
				m_state = 11;
				break;
			case 11:
				m_state = 2;
				m_SetTemp(SetTempAdjust);
				break;
			case 20:
				m_state = 21;
				break;
			case 21:
				m_state = 3;
				TimerCounter = SetTimerAdjust + millis();
				break;
			default:
			;

			}
		} else if (passedTime >= 500 )// && passedTime < 1000)
		{
			return m_submenu;
		}
		pressTime = 0;
	}
	Serial.println(".");
	redraw();
	return this;
}
void MenuItem_MainScreen::Init () {
	m_state = 0u;
	TimerCounter=0;
	pressTime = 0;
	redraw();
	char buffer6[] = "\x00\x00\xFF\x00\x00\xFF";
									ws2812_writergb(PIN_LED_WS2812, buffer6, sizeof(buffer6));

}
void MenuItem_MainScreen::DeInit () {
  ;
}

#ifndef MENUITEM_MAINSCREEN_H
#define MENUITEM_MAINSCREEN_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

class MenuItem_MainScreen : public  MenuItem {
  public:
    MenuItem_MainScreen();
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, float (*GetTemp)(void), float (*GetSetTemp)(void), void (*SetTemp)(float), unsigned long (*GetTimer)(void), void (*SetTimer)(unsigned long), uint8_t (*GetPidMode)(void), void (*SetPidMode)(uint8_t));
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
    void displayTimer(unsigned long Timleft);
    MenuItem* m_submenu;
    Adafruit_SSD1306* m_display;
    unsigned long TimerCounter;
    unsigned long pressTime;
    unsigned long returnTime;
    float (*m_GetSetTemp)(void);
    float (*m_GetTemp)(void);
    void (*m_SetTemp)(float);
    unsigned long (*m_GetTimer)(void);
    void (*m_SetTimer)(unsigned long);
    uint8_t (*m_GetPidMode)(void);
    void (*m_SetPidMode)(uint8_t);
    int8_t	m_state;
	char m_timerbuf[8];
	char m_setbuf[8];
};

#endif

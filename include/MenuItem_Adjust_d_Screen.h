#ifndef MENUITEM_ADJUST_D_SCREEN_H
#define MENUITEM_ADJUST_D_SCREEN_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

class MenuItem_Adjust_d_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_d_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, double (*GetValue)(void), void (*SetValue)(double));
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
    void checkCharBuf(void);
    void adjustChar(int rot, char* chr);
    Adafruit_SSD1306* m_display;
    const String* m_name;
    double (*m_GetValue)(void);
    void (*m_SetValue)(double);
    MenuItem* m_submenu;
    unsigned long pressTime;
    double m_value;
    int8_t	m_state;
    char m_charbuf[8];
};



#endif

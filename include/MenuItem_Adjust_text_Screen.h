#ifndef MENUITEM_ADJUST_text_SCREEN_H
#define MENUITEM_ADJUST_text_SCREEN_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

class MenuItem_Adjust_text_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_text_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, String (*GetValue)(void), void (*SetValue)(String));
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
    String (*m_GetValue)(void);
    void (*m_SetValue)(String);
    MenuItem* m_submenu;
    unsigned long pressTime;
    int8_t	m_state;
    char m_charbuf[22];
    String m_value;
};

#endif

#ifndef MENUITEM_ADJUST_TS_SCREEN_H
#define MENUITEM_ADJUST_TS_SCREEN_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

class MenuItem_Adjust_ts_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_ts_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, int32_t (*GetValue)(void), void (*SetValue)(int32_t), int32_t min, int32_t max );
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
    int32_t (*m_GetValue)(void);
    void (*m_SetValue)(int32_t);
    MenuItem* m_submenu;
    unsigned long pressTime;
    double m_value;
    int8_t	m_state;
    char m_charbuf[8];
    int32_t m_min;
    int32_t m_max;
    uint8_t m_len;
};



#endif

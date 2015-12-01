#ifndef MenuItem_Adjust_Enum_Screen_H
#define MenuItem_Adjust_Enum_Screen_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

class MenuItem_Adjust_Enum_Screen : public  MenuItem {
  public:
	MenuItem_Adjust_Enum_Screen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display ,MenuItem* submenu, String enumString[], uint8 enumSize, int (*GetValue)(void), void (*SetValue)(int));
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
    void adjustChar(int rot, char* chr);
    Adafruit_SSD1306* m_display;
    const String* m_name;
    int (*m_GetValue)(void);
    void (*m_SetValue)(int);
    MenuItem* m_submenu;
    unsigned long pressTime;
    int m_value;
    int8_t	m_state;
    uint8 m_enumSize;
    String* m_charbuf;
};



#endif

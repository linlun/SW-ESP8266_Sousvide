#ifndef MENUITEM_SETTINGSSCREEN_H
#define MENUITEM_SETTINGSSCREEN_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <MenuItem.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
class SettingsElement {
  public:
	SettingsElement(const String* name, MenuItem* submenu);
	SettingsElement(const String* name, void (*GetValue)(char*), MenuItem* submenu);
	SettingsElement(const String* name, void (*GetValue)(char*), MenuItem* submenu, void (*RunFuncOnPress)(void));
	//Called to setup this menu, add sub-menus for example
    MenuItem* m_submenu;
    const String* m_name;
    void (*m_GetValue)(char*);
    void (*m_RunFuncOnPress)(void);
};
class MenuItem_SettingsScreen : public  MenuItem {
  public:
	MenuItem_SettingsScreen(const String* name);
	//Called to setup this menu, add sub-menus for example
	void Config(Adafruit_SSD1306* display , SettingsElement* elements[], uint8_t elementSize);
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
    SettingsElement** m_elements;
    uint8_t m_elementSize;
    int8_t m_selected;
    uint8_t m_viewStart;
    uint8_t m_viewEnd;
    unsigned long pressTime;
};



#endif

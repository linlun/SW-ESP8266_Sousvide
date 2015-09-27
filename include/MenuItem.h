#ifndef MENUITEM_H
#define MENUITEM_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>

class MenuItem {
  public:
    virtual ~MenuItem() {};
    //Called periodically
    virtual MenuItem* Loop() = 0;
    //called when input changes state
    virtual MenuItem* Update(int rotation, bool pressed) = 0;
    //initialize the menu
    virtual void Init() = 0;
    //initialize the menu
    virtual void DeInit() = 0;
};

#endif

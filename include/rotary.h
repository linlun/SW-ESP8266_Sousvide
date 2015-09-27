#ifndef ROTARY_H
#define ROTARY_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>



class Rotary {
#ifndef ROTARY_CHx_INVERT_DIRECTION
#define ROTARY_CHx_INVERT_DIRECTION 0
#endif

  public:
	Rotary(uint8_t rotA,uint8_t rotb,uint8_t sw,  void (*rotInt)(void));

	//
	uint8_t GetPosition();
	int8_t GetPositiondiff();
	boolean GetSwitch();
	void pollEncoder(void);

  private:
	void rotary_int(void);
	uint8_t _rotA;
	uint8_t _rotB;
	uint8_t _sw;
	uint8_t last_Position;
	uint8_t rotaryEncoder_Position = 0;
	boolean rotaryEncoder_Button_Position = 0;
};

#endif


#include "rotary.h"
#include <user_config.h>
#include <SmingCore/SmingCore.h>

void Rotary::pollEncoder()
{
	rotary_int();
}
void Rotary::rotary_int()
{
  uint8_t rot_data = 0;
  static uint8_t rot_lastdir = 0, rot_laststate = 0;

  //Take care of button push
  if ((boolean)digitalRead(_sw) != rotaryEncoder_Button_Position) { //The buttonstate has changed!
    rotaryEncoder_Button_Position = (boolean)digitalRead(_sw);
  }
  //Take care of rotary encoder movement
  if (digitalRead(_rotA)) {
    rot_data |= 0x01;
  }
  if (digitalRead(_rotB)) {
    rot_data |= 0x02;
  }
  //Serial.println(rot_data + (int)rotaryEncoder_Button_Position*4);

  if ( rot_data == 0 || rot_data == 3 ) { // Are both signals high or low?
    if ( rot_data == 0 && rot_laststate != rot_data ) { // Are both signals low? In that case we are finished with one turn and should print out the direction it went.
      if ( rot_lastdir & 0x01 ) {
#if ROTARY_CHx_INVERT_DIRECTION==1
        rotaryEncoder_Position--;	// Moving clockwise
#else
        rotaryEncoder_Position++;	// Moving counter clockwise
#endif
      } else {
#if ROTARY_CHx_INVERT_DIRECTION==1
        rotaryEncoder_Position++;	// Moving counter clockwise
#else
        rotaryEncoder_Position--;	// Moving clockwise
#endif
      }
    }
    rot_laststate = rot_data;
  } else { // No, only one of the signals are high. We can use this to find out what direction we are moving.
    rot_lastdir = rot_data;
  }
}

Rotary::Rotary(uint8_t rotA,uint8_t rotB,uint8_t sw,  void (*rotInt)(void) ) {
	_rotA = rotA;
	_rotB = rotB;
	_sw = sw;
	last_Position = 0;

	pinMode(_rotA, INPUT);
	pinMode(_rotB, INPUT);
	pinMode(_sw, INPUT);

	// encoder pin on interrupt 0 (pin 2)
	attachInterrupt(_rotA, rotInt, CHANGE);
	// encoder pin on interrupt 1 (pin 3)
	attachInterrupt(_rotB, rotInt, CHANGE);
	// encoder pin on interrupt 1 (pin 3)
	attachInterrupt(_sw, rotInt, CHANGE);
}


uint8_t Rotary::GetPosition()
{
	return rotaryEncoder_Position;
}

int8_t Rotary::GetPositiondiff()
{
	//int16_t currentPos = (int16_t)rotaryEncoder_Position;
	uint8_t currentPos = rotaryEncoder_Position;
	int8_t out = (int8_t)(currentPos - last_Position);
	last_Position = currentPos;
	return out;
}

boolean Rotary::GetSwitch()
{
	return !rotaryEncoder_Button_Position;
}


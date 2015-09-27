
#include "SlowPWM.h"
#include <user_config.h>
#include <SmingCore/SmingCore.h>



SlowPWM::SlowPWM(uint8_t pin,uint16_t period,  void (*timerInt)(void)) {
	_pin = pin;
	_period = period;
	_duty = 0.0;
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin,0);
	_state = 0;
	_lowTime = 0;
	_timerInt = timerInt;
}


void SlowPWM::SetDuty(float duty)
{
	if (duty < 0.0)
		duty = 0.0;
	if (duty > 1.0)
		duty = 1.0;
	_duty = duty;
}

void SlowPWM::Start(void)
{
	uint16_t highValue = (uint16_t)(_period * _duty);
	_lowTime = (uint16_t)(_period * (1.0 - _duty));
	if (highValue == 0)
		highValue = 1;
	if (_lowTime == 0)
		_lowTime = 1;
	/*Serial.println("Start pwm");
	Serial.print("period ");
	Serial.print(highValue);
	Serial.print(" + ");
	Serial.println(_lowTime);
	*/
	_timer.initializeMs(highValue, _timerInt).start(false);
}

void SlowPWM::Stop(void)
{
	_timer.stop();
	digitalWrite(_pin,0);
}

void SlowPWM::timerInt(void)
{
	if (_state)
	{	//Restart cycle, set output high
		//Serial.println("timer int 1");
		digitalWrite(_pin,1);
		uint16_t highValue = (uint16_t)(_period * _duty);
		_lowTime = (uint16_t)(_period * (1.0 - _duty));
		if (highValue == 0)
			highValue = 1;
		if (_lowTime == 0)
			_lowTime = 1;
		/*Serial.println("pwm");
			Serial.print("period ");
			Serial.print(highValue);
			Serial.print(" + ");
			Serial.println(_lowTime);
*/
		_timer.initializeMs(highValue, _timerInt).start(false);
		_state = 0;

	}  else
	{	//Set output low
	//	Serial.println("timer int 0");
		digitalWrite(_pin,0);
		_timer.initializeMs(_lowTime, _timerInt).start(false);
		_state = 1;
	}
}

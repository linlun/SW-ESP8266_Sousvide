#ifndef SLOWPWM_H
#define SLOWPWM_H
#include <user_config.h>
#include <SmingCore/SmingCore.h>



class SlowPWM {


  public:
	SlowPWM(uint8_t pin,uint16_t period,  void (*timerInt)(void));
	void SetDuty(float duty);
	void Start(void);
	void Stop(void);
	void timerInt(void);
  private:

	uint8_t _pin;
	uint16_t _period;
	uint16_t _lowTime;
	Timer _timer;
	boolean	_state;
	float _duty;
	void (*_timerInt)(void);

};

#endif

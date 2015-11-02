/*
*
*  This class is based on :
*
*  SousVideWith8SegmentDisplays
*
*  Adaptative regulation sous-vide cooker algorithm
*
*  See http://www.instructables.com/id/Cheap-and-effective-Sous-Vide-cooker-Arduino-power/ for more info
*
*  Author : Etienne Giust - 2013, 2014
*
*  Features
*
*  - Works out of the box : no need for tweaking or tuning, the software adapts itself to the characteristics of your cooker :  whether it is big, small, full of water, half-full, whether room temperature is low or high, it works.
*  - Efficient regulation in the range of 0.5°C
*  - Sound alarm warns when target temperature is reached
*  - Automatic detection of lid opening and closing : regulation does not get mad when temperature probe is taken out of the water (which is a thing you need to do if you want to actually put food in your cooker)
*  - Safety features : 
*     - automatic cut-off after 5 minutes of continuous heating providing no change in temperature
*     - automatic cut-off after 24 hours of operation
*     - automatic cut-off when temperature reaches 95 °C
*     - allows target temperature only in the safe 50°c to 90°C range 
*  - Dead cheap and simple : no expensive LCD or Solid State Relay
*
*
*  License
*	
*  Copyright (C) 2014  Etienne Giust
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as published
*  by the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
*/
#ifndef AdaptiveRegulator_h
#define AdaptiveRegulator_h
#include <config.h>
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <stdlib.h>
class AdaptiveRegulator
{


  public:
#define OUTPUT_TO_SERIAL	1
  //Constants used in some of the functions
// temperature sensor
#define SAMPLE_DELAY 5000

// First Ramp
#define FIRST_RAMP_CUTOFF_RATIO 0.65

// Security features
#define MIN_TARGET_TEMP 39   /*sufficient for most sous-vide recipes*/
#define MAX_TARGET_TEMP 95   /*sufficient for most sous-vide recipes*/
#define SHUTDOWN_TEMP 98   /*shutdown if temp reaches that temp*/
#define MAX_UPTIME_HOURS 72   /*shutdown after 72 hours of operation*/
#define MAX_HEATINGTIME_NO_TEMP_CHANGE_MINUTES 10   /*detect when temp sensor is not in the water and prevent overheating*/

// regulation
#define MIN_SWITCHING_TIME 1500  /* Minimum ON duration of the heating element */
#define DROP_DEGREES_FOR_CALC_REGULATION 0.12 /* minimum drop in degrees used to calculate regulation timings (should be small : <0.2 ) */
#define LARGE_TEMP_DIFFERENCE 1  /* for more than "1" degree, use the Large setting (Small otherwise)*/

  //commonly used functions **************************************************************************
    AdaptiveRegulator(uint8_t pin, float* Input, float* Setpoint);        // * constructor.  links the PID to the Input, Output, and Setpoint.
	
   
    bool Compute();                       // * performs the regulator calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively
	


  //available but not commonly used functions ********************************************************
    
										  
  //Display functions ****************************************************************
    enum operatingState { INITIAL_WAIT = 0, TEMP_DROP, TEMP_RISE, FIRST_RAMP, BOOST_TEMP, COUNTER_FALL, WAIT_NATURAL_DROP, REGULATE};
    operatingState opState = INITIAL_WAIT;
    boolean isHeatOn = false;
  private:	
	void ResetVariablesForRegulationCalculation();
	void EnterRegulateStateOrWaitSmoothLowering();
	void WaitForNaturalDrop();
	void Regulate();
	void PerformRegulationCalculations();
	bool checkDerivativeReliable();
	void SetActualDerivative();
	void GetTemperatureAndEnforceSecurity();
	void WatchForTempFalling();
	void StartBoostToTarget();
	void StartBoostToTarget(float offset);
	float HeatingTimeNeeded(float degreeOffset);
	void HeatForDegrees(float degrees);
	void PerformBoostTemp();
	void FinishBoostTemp();
	float predictTemp(unsigned long horizon);
	void AdaptGain(float resultingTemp);
	void StartInitialRamping();
	void setupCutOffTempForInitialRamping();
	void PerformFirstRamp();
	void FinishInitialRamping();
	void turnOnRelay();
	void turnOffRelay();

	// Security checks    
	void checkShutdownConditions();
	void shutdownDevice();
	void SetApproximatePulseDurationsForREgulation(float tempLost, unsigned long regDelay );
	void SetPulseDurationsForREgulation(unsigned long neededUptimeForCompensate, unsigned long regDelay );
	/**************************************************************************************/
	/*                                                                                    */
	/*                                    UTILITIES                                       */
	/*                                                                                    */
	/**************************************************************************************/
	// ------------------------- temperature array UTILITIES
	void tempPreviousArrayPushValue(float val);
	bool IsStabilizingOrDropping();
	bool IsStabilizingOrGrowing();
	bool IsStabilizing();
	bool IsFallingNaturally();
	bool IsFalling();
	bool IsAcceleratingFall();
	// ------------------------- other UTILITIES
	void soundAlarm();
	void alertTemperatureNearlySet();
	float getTemperature();
		
	
	float *myInput;              // * Pointers to the Input, Output, and Setpoint variables
								  //   This creates a hard link between the variables and the
	float *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
			  
	unsigned long lastTime;
	uint8_t _pin;
	
// ------------------------- DEFINITIONS & INITIALISATIONS

// temperatures
float environmentTemp = 0;
float actualTemp = 0;
float storedTargetTemp = 0;
float initialTemp = 0;
float firstRampCutOffTemp = 0;
float maxRegTEmp = 0;
float minRegTEmp = 0;
float tempBeforeDrop = 0;
float tempBeforeHeating = 0;
float parametersRegulationSetForTemp = 0;
float actualTempAtBoostStart = 0;
float expectedTempChange = 0;
float tempPreviousArray[6]= {0, 0, 0, 0, 0, 0};

// derivatives
float currentTempDerivative;
float previousDerivative;

// gains
float secondPerDegreeGainRef = 0;
float secondPerDegreeGainLarge = 0;
float secondPerDegreeGainSmall = 0;

// booleans & states
bool isNewSample = false;
boolean isWaitingForTempAlert = false;
boolean waitForSuddenRise = false;
boolean isDerivativeReliable = false;
boolean waitingForStabilization = false;
boolean doBackToFirstRampWhenStabilizing = false;

boolean isCounteracting = false;

enum boostTypes {HIGHBOOST = 0, LOWBOOST};
boostTypes boostType = HIGHBOOST;
int warningsBeforeCounterFall;

// timings
unsigned long tcurrent = 0;
unsigned long tStartFirstRamp = 0;
unsigned long tStartBoostTemp = 0;
unsigned long tStartRealRegulation = 0;
unsigned long tFirstRampCutOff = 0;
unsigned long tEndFirstRamp = 0;
unsigned long tOperationalDelay = 0;
unsigned long burnupTime = 0;
unsigned long tMinReg = 0;
unsigned long tMaxReg = 0;
unsigned long tLastTurnOffRelay = 0;
unsigned long durationOnPulse = 0;
unsigned long durationOffPulse = 0;
unsigned long tGetTemperatureSample  = 0;
unsigned long tCheckStabilize  = 0;
unsigned long tCheckTakeOff = 0;
unsigned long tBackToLow = 0;
unsigned long tBackToHigh = 0;
unsigned long delaytime=100;

// security variables
unsigned long  maxUptimeMillis;
unsigned long  tCheckNotHeatingWildly;
};
#endif


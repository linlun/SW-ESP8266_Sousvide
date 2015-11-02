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
#include <AdaptiveRegulator.h>
// ------------------------- CONSTANTS


/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up 
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
AdaptiveRegulator::AdaptiveRegulator(uint8_t pin, float* Input, float* Setpoint)
{
    myInput = Input;
    mySetpoint = Setpoint;
	_pin = pin;
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin,0);
	/*
	Read temperature
	*/
	actualTemp =  *myInput;
	tcurrent = millis();
	tGetTemperatureSample = 0;
	maxUptimeMillis = MAX_UPTIME_HOURS * (unsigned long)3600 * (unsigned long)1000;

	// Initial State  
	warningsBeforeCounterFall = 3;
	opState = INITIAL_WAIT;

	lastTime = millis()-delaytime;
}



/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/


bool AdaptiveRegulator::Compute() {   
    
	tcurrent = millis();
   unsigned long timeChange = (tcurrent - lastTime);
   if(timeChange>=delaytime)
   {

	  // get temperature every few seconds and output it to serial if needed. Alert if we are within range
	  GetTemperatureAndEnforceSecurity();
	  // compute current temperatue Derivative
	  SetActualDerivative();
	  //Serial.print(".");
	  
	  switch (opState)
	   {
		case INITIAL_WAIT:			
			// wait for initial temperature stability
			if (abs(actualTemp - tempPreviousArray[1] ) < 0.1)
			{		
				if (environmentTemp == 0)
				{				
					// store initial temp, but not more than 30 degrees
					environmentTemp = min(actualTemp, 30);			
				}
				// check if target temp is in acceptable range and switch to first ramp if so
				if(*mySetpoint >= MIN_TARGET_TEMP)
				{
					StartInitialRamping();        
				}
			}		
			break;
		case TEMP_DROP:
			// wait for stabilization or for sudden rise
			if (waitForSuddenRise == false && IsStabilizing())
			{
				if (abs(actualTemp - environmentTemp) < abs(actualTemp - tempBeforeDrop))
				{
					// we are close to environmentTemp. The temp probe is probably off-water; wait till temperature rises again sharply then stablilizes
					waitForSuddenRise = true;
					
					Serial.println("TEMP_DROP : wait temprise");
				} else {
					// something very cold was inserted in the cooker; or not. either way, the temp probe is back. let's regulate 							
					if (doBackToFirstRampWhenStabilizing)
					{
						Serial.println("TEMP_RISE : initial ramping");
						opState = FIRST_RAMP;
					} 
					else 
					{				
						Serial.println(" TEMP_DROP : Cold ! reg");
						EnterRegulateStateOrWaitSmoothLowering();
					}
				}
			}
			WatchForTempFalling();
			break;
			
		case TEMP_RISE:
			// wait for stabilization, then Regulate
			if ( IsStabilizingOrDropping() )
			{			
				if (doBackToFirstRampWhenStabilizing)
				{
					Serial.println(" TEMP_RISE : back to initial ramping");
					opState = FIRST_RAMP;
				} 
				else 
				{	
					Serial.println(" TEMP_RISE : back to normal : reg");
					EnterRegulateStateOrWaitSmoothLowering();
				}
			}
			WatchForTempFalling();
			break;
			
		case FIRST_RAMP:
			PerformFirstRamp();
			break;   

		case COUNTER_FALL:	
			// START CONDITION : temp well below target && important negative derivative , but not freefall : -0.1 < d < -0.01,  3 times in a row
		
			// ON, until deriv == 0 then cut and wait stabilization
			if (isNewSample) 
			{		
				Serial.println(" Counterfall check");
				if (waitingForStabilization == false)	
				{			
					// check derivative
					//if(isDerivativeReliable && currentTempDerivative > -0.005)
					float predicted = predictTemp(tOperationalDelay) ;
					Serial.print(" predicted temp : ");
					Serial.println(predicted);
					
					if ( predicted >= (*mySetpoint - 1)  && isDerivativeReliable && currentTempDerivative > 0.001)  // targetTemp - 1 is to avoid overshoot because prediction is not precise enough
					{
						Serial.println(" TURNOFFRELAY !");
						turnOffRelay();
						waitingForStabilization = true;
					}					
				} 
				else 
				{
					if ( IsStabilizingOrDropping() )
					{
						Serial.println(" COUNTER_FALL finished : reg");
						
						//reset counter
						warningsBeforeCounterFall = 3;
						EnterRegulateStateOrWaitSmoothLowering();
					}
					if( isDerivativeReliable && currentTempDerivative < -0.005)
					{
						turnOnRelay();
						waitingForStabilization = false;
					}				
				}		
			}
			break;		
		case BOOST_TEMP:		
			PerformBoostTemp();
			WatchForTempFalling();
			break; 


		case WAIT_NATURAL_DROP:
			if (isNewSample) 
			{	
				// when temp is close enough to target, try to calculate regulation values if they are not already set
				if (isCounteracting == false && parametersRegulationSetForTemp != *mySetpoint && abs(actualTemp - *mySetpoint) < 3 )
				{
					PerformRegulationCalculations();
				}	
				
				// predict temp at t + tOperationalDelay
				float futureTemp = predictTemp(tOperationalDelay);
				// counter act to stabilize near targetTemp
				if (isCounteracting == false && futureTemp < *mySetpoint)
				{
					isCounteracting = true;
					HeatForDegrees(actualTemp - futureTemp);
				}
				// check for stabilization
				if ( ((long) (millis() - tCheckStabilize) >= 0) && isCounteracting )
				{
					if(IsStabilizingOrGrowing())
					{ 
						Serial.println("NATURAL_DROP ended: wait stabilize");
						opState = TEMP_RISE; // make sure we stabilize before regulating again
					} 
					
					if(IsAcceleratingFall())
					{
						Serial.println("fall:tryagain!");
						isCounteracting = false;
					}			
				}
				// we fell too much
				if (actualTemp < *mySetpoint - 0.1)
				{
					StartBoostToTarget();	
				}
			}	
			WatchForTempFalling();
			break;  
		case REGULATE:
			Regulate();
			WatchForTempFalling();
			break;  
	   }
	   
	   if (opState != FIRST_RAMP && opState != COUNTER_FALL)
	   {
			// check each time if relay needs to be turned off (except during initial ramping or counter action)
			if ( (long) (millis() - tBackToLow) >= 0)
			{
				turnOffRelay();
			}
	   }
	   lastTime = tcurrent;
	   return true;
  }
  else
	  return false;
}




/**************************************************************************************/
/*                                                                                    */
/*                                  HELPER FUNCTIONS                                  */
/*                                                                                    */
/**************************************************************************************/


void AdaptiveRegulator::ResetVariablesForRegulationCalculation()
{
	maxRegTEmp = 0;
	minRegTEmp = 1000;	
}

void AdaptiveRegulator::EnterRegulateStateOrWaitSmoothLowering()
{

	if (actualTemp < *mySetpoint + 0.3)
	{
		Serial.println("EnterRegulateState !");
		ResetVariablesForRegulationCalculation();
		
		tBackToHigh = 0;	
		// make sure we do not start heating right away when entering regulation over target value
		if (parametersRegulationSetForTemp == *mySetpoint && actualTemp > *mySetpoint )
		{	
			tBackToHigh = 	millis() + durationOffPulse;
		} 
		tBackToLow = 0;
		tMinReg = 0;
		tMaxReg = 0;
		tStartRealRegulation = 0;

		opState = REGULATE;
	} 
	else 
	{
		WaitForNaturalDrop();
	}
}

void AdaptiveRegulator::WaitForNaturalDrop()
{
	opState = WAIT_NATURAL_DROP;
	isCounteracting = false;
	Serial.println("WAIT_NATURAL_DROP!");
	ResetVariablesForRegulationCalculation();	
}

void AdaptiveRegulator::Regulate()
{  
	if (actualTemp > ( *mySetpoint + 0.2 ))
	{
		// adapt regul values : they are too high
		if ( IsStabilizing() && parametersRegulationSetForTemp == *mySetpoint && tStartRealRegulation > 0 && (millis() - tStartRealRegulation) > tOperationalDelay )
		{
			durationOnPulse = durationOnPulse / 1.3;
			while ( durationOnPulse < MIN_SWITCHING_TIME )
			{
				durationOffPulse = durationOffPulse * 1.2;
				durationOnPulse = durationOnPulse * 1.2 ;
			}
			tStartRealRegulation = millis();
			tBackToHigh = millis() + durationOffPulse;
			Serial.print("durationOffPulse = ");
			Serial.print(durationOffPulse);
			Serial.print("durationOnPulse = ");
			Serial.println(durationOnPulse);
			WaitForNaturalDrop();
		}
	}
	
	// try to regulate temperature when we are at a stable targetTemp

	// Maybe we are far below the goal ; time for a boost ?
	if((*mySetpoint - *mySetpoint) >= 0.25)
	{
		// adapt regul values : they are too low
		if ( IsStabilizing() && parametersRegulationSetForTemp == *mySetpoint && (millis() - tStartRealRegulation) > tOperationalDelay )
		{
			durationOffPulse = durationOffPulse / 1.3;
			while ( durationOffPulse < MIN_SWITCHING_TIME )
			{
				durationOffPulse = durationOffPulse * 1.2;
				durationOnPulse = durationOnPulse * 1.2 ;
			}
			Serial.print("durationOffPulse = ");
			Serial.print(durationOffPulse);
			Serial.print("   durationOnPulse = ");
			Serial.println(durationOnPulse);
		}	  
		StartBoostToTarget();		
	} 
	else 
	{			
		if (parametersRegulationSetForTemp == *mySetpoint )
		{			
			if (tStartRealRegulation == 0)
			{
				tStartRealRegulation = millis();
				tBackToHigh = 0;
			}
			// We already have ON and OFF durations
			// perform regulation
			if (digitalRead(_pin) == LOW) {
				// check if downtime over
				if ( (long) (millis() - tBackToHigh) >= 0)
				{
					turnOnRelay();
					tBackToLow = millis() + durationOnPulse + burnupTime;
					tBackToHigh = millis() + durationOnPulse + burnupTime + durationOffPulse;
				}
			}			
		} 
		else
		{
			if ((*mySetpoint - actualTemp) >= 0.1)
			{
				//perform a boost with slight overshoot first
				StartBoostToTarget(0.1);	
			} 
			else
			{
				// find suitable ON and OFF durations					 
				PerformRegulationCalculations();
			}			
		}
	}  
}

void AdaptiveRegulator::PerformRegulationCalculations()
{
	if (isNewSample && IsFallingNaturally() && tempPreviousArray[0] != 0 && tempPreviousArray[1] != 0 && tempPreviousArray[2] != 0)
	{
		// calc average of 3 last samples
		
		float averageTemp3 = (tempPreviousArray[0] + tempPreviousArray[1] +tempPreviousArray[2]) / 3;
	 
		// find max and min temperatures
		if (averageTemp3 > maxRegTEmp)
		{
			maxRegTEmp = averageTemp3;
			tMaxReg = millis();
		}					
		
		if (averageTemp3 < minRegTEmp)
		{
			minRegTEmp = averageTemp3;
			tMinReg = millis();
		}
		
		Serial.print(" --- avgTemp3 = ");
		Serial.print(averageTemp3, DEC);
		Serial.print(" --- maxRegTEmp = ");
		Serial.print(maxRegTEmp, DEC);
		Serial.print(" --- minRegTEmp = ");
		Serial.print(minRegTEmp, DEC);
		Serial.print(" --- tMaxReg = ");
		Serial.print(tMaxReg);
		Serial.print(" --- tMinReg = ");
		Serial.println(tMinReg);
		
		
		// wait till we lost DROP_DEGREES_FOR_CALC_REGULATION degrees
		if (maxRegTEmp > 0 && minRegTEmp > 0 && (((long)(tMinReg - tMaxReg)) > 0) && ((maxRegTEmp - minRegTEmp) > DROP_DEGREES_FOR_CALC_REGULATION))
		{											
			// Try to come up with Pulse durations (ON and OFF) to counteract temperature loss
			SetApproximatePulseDurationsForREgulation(maxRegTEmp - minRegTEmp, tMinReg - tMaxReg);		

			// back to target temp
			StartBoostToTarget();							
		}
	}	
}

bool AdaptiveRegulator::checkDerivativeReliable()
{
	for(int i = 0; i < 6 ; i++)
	{
		if(tempPreviousArray[i]==0)
		{
			return false;
		}
	}
	return true;
}


void AdaptiveRegulator::SetActualDerivative()
{
	if (isNewSample)
	{
		isDerivativeReliable = checkDerivativeReliable();		
		Serial.print("d = ");
		if (isDerivativeReliable)
		{
			//remove biggest and lowest values (get rid off irregularities)
			
			// identify lowest and highest
			float lowest =  1000;
			float highest =  0;
			int i=0;
			for(i=0;i<6;i++) {
				if(tempPreviousArray[i] > highest)
				highest = tempPreviousArray[i];
				
				if(tempPreviousArray[i] < lowest)
				lowest = tempPreviousArray[i];
			}
			
			float tempTemp[6];
			float filteredValues[4];
			bool isHighestRemoved = false;
			bool isLowestRemoved = false;
			//
			if (currentTempDerivative > 0)
			{
				//ascending trend : remove lowest value to the end
				for(i=5;i>=0;i--) {
					if(tempPreviousArray[i] == lowest && !isLowestRemoved)
					{
						tempTemp[i] = 0;
						isLowestRemoved = true;
					} else {
						tempTemp[i] = tempPreviousArray[i];
					}					
				}
				// remove highest value to the starts of the array
				for(i=0;i<6;i++) {
					if(tempTemp[i] == highest && !isHighestRemoved)
					{
						tempTemp[i] = 0;
						isHighestRemoved = true;
					} 				
				}				
			} 
			else			
			{
				//descending trend : remove lowest value to the starts of the array
				for(i=0;i<6;i++) {
					if(tempPreviousArray[i] == lowest && !isLowestRemoved)
					{
						tempTemp[i] = 0;
						isLowestRemoved = true;
					} else {
						tempTemp[i] = tempPreviousArray[i];
					}					
				}
				// remove highest value to the end
				for(i=5;i>=0;i--) {
					if(tempTemp[i] == highest && !isHighestRemoved)
					{
						tempTemp[i] = 0;
						isHighestRemoved = true;
					} 				
				}
			}
			int j = 0;
			for(i=0;i<6;i++) {
				if(tempTemp[i] != 0)
				{
					filteredValues[j] = tempTemp[i];
					j++;
				}					
			}
			
			float pastValues[2];
			pastValues[0] = ( filteredValues[0] + filteredValues[1] ) / 2;
			pastValues[1] = ( filteredValues[2] + filteredValues[3] ) / 2;
			// calculate last derivative
			previousDerivative = currentTempDerivative;
			currentTempDerivative = ((pastValues[0] - pastValues[1]) / (3* SAMPLE_DELAY/1000));
			Serial.println(currentTempDerivative, DEC);
		}	else
		{
			Serial.println("NC!");
		}
	}	
}

void AdaptiveRegulator::GetTemperatureAndEnforceSecurity()
{
	if ( (long) (tcurrent - tGetTemperatureSample) >= 0)
	{
		tGetTemperatureSample = tcurrent + SAMPLE_DELAY;
		actualTemp = *myInput;
		
		if (opState != TEMP_DROP && (tempPreviousArray[0] - actualTemp > 2))
		{
			//sudden drop in temperature -> temp probe off-water			
			if(opState == COUNTER_FALL || opState == FIRST_RAMP)
			{
				tBackToLow = 0;
				if (opState == FIRST_RAMP)
				{
					firstRampCutOffTemp = tempPreviousArray[0];
					doBackToFirstRampWhenStabilizing = true;
				}
			}
			
			opState = TEMP_DROP;
			tempBeforeDrop = tempPreviousArray[0];
			waitForSuddenRise = false;	
			Serial.println("REMOVED TEMP PROBE!");
			
			if (tStartBoostTemp - millis() <= 3 * SAMPLE_DELAY)
			{
				// we probably boosted temp wrongly as temp probe was off-water
				// cancel boost
				tBackToLow = 0;
			}
			
			
		}	
		if (opState == TEMP_DROP && (actualTemp - tempPreviousArray[0] > 2))
		{
			//sudden rise in temperature -> temp probe back in water
			opState = TEMP_RISE;
			// erase previous values in history of temperature -> prevent calculated negative derivative even if we are climbing
			tempPreviousArray[1]=0;
			tempPreviousArray[2]=0;
			tempPreviousArray[3]=0;
			tempPreviousArray[4]=0;
			tempPreviousArray[5]=0;
				
			Serial.println("PROBE BACK");
		}	
		if (opState == BOOST_TEMP && (actualTemp - tempPreviousArray[0] > 1))
		{
			//sudden rise in temperature during BOOST_TEMP -> maybe temp probe was just put back in water			
			if (tStartBoostTemp - millis() <= 3 * SAMPLE_DELAY)
			{
				// we probably boosted temp wrongly as temp probe was off-water
				// cancel boost
				tBackToLow = 0;
			}
			// erase previous values in history of temperature -> prevent calculated negative derivative even if we are climbing
			tempPreviousArray[1]=0;
			tempPreviousArray[2]=0;
			tempPreviousArray[3]=0;
			tempPreviousArray[4]=0;
			tempPreviousArray[5]=0;
		}
		
		tempPreviousArrayPushValue(actualTemp); 
		isNewSample = true;
		if (OUTPUT_TO_SERIAL) {
		  Serial.print(tcurrent/1000, DEC);
		  Serial.print(";  ");
		  Serial.println(actualTemp, 3);
		}    
		if (actualTemp > *mySetpoint + 0.15)
		{
			//	force to turn off when no need to be ON (0.15 offset accounts for regulation conditions)
			tBackToLow = 0;
		}
		
		alertTemperatureNearlySet();
		checkShutdownConditions();		
	} else {
		isNewSample = false;
	}
}

void AdaptiveRegulator::WatchForTempFalling()
{
	if (isNewSample)
	{
		// START CONDITION : temp well below target && important negative derivative , but not freefall : -0.1 < d < -0.007,  3 times in a row
		if ( (*mySetpoint - *mySetpoint) > 1 && IsFalling() )
		{
			// must happen 3 times in a row
			warningsBeforeCounterFall--;
			if (warningsBeforeCounterFall == 0)
			{
				turnOnRelay();
				waitingForStabilization = false;
				opState = COUNTER_FALL;
			}
		}	
		else 
		{
			warningsBeforeCounterFall = 3;
		}	
	}
}


void AdaptiveRegulator::StartBoostToTarget()
{
	StartBoostToTarget(0);
}

void AdaptiveRegulator::StartBoostToTarget(float offset)
{
	// predict value at t + tOperationalDelay
	actualTempAtBoostStart = actualTemp;	
	float realTargetTemp = *mySetpoint + offset;
	if (realTargetTemp > actualTempAtBoostStart)
	{	
		expectedTempChange = realTargetTemp - actualTempAtBoostStart;
		Serial.print("BOOST_TEMP! expectedTempChange = ");
		Serial.println(expectedTempChange);
		HeatForDegrees(expectedTempChange);	
		// change state
		opState = BOOST_TEMP;
		storedTargetTemp = *mySetpoint;
		tStartRealRegulation = 0;
	}
}


float AdaptiveRegulator::HeatingTimeNeeded(float degreeOffset)
{
	float secondPerDegreeGain;
	if (degreeOffset > LARGE_TEMP_DIFFERENCE)
	{
		secondPerDegreeGain = secondPerDegreeGainLarge;
		boostType = HIGHBOOST;
	} else {
		secondPerDegreeGain = secondPerDegreeGainSmall;
		boostType = LOWBOOST;
	}
	return max(degreeOffset * secondPerDegreeGain * 1000, MIN_SWITCHING_TIME) + burnupTime;
}

void AdaptiveRegulator::HeatForDegrees(float degrees)
{
	if (degrees > 0)
	{
		tBackToLow = 0;
		tCheckStabilize = 0;
		tStartBoostTemp = millis();
		tBackToLow = millis() +  HeatingTimeNeeded(degrees);
		tCheckStabilize = tBackToLow + tOperationalDelay;		

		if ( (long) (millis() - tBackToLow) < 0)
		{  
		  turnOnRelay();
		  Serial.print("HEAT ON ! tBackToLow = ");
		  Serial.println(tBackToLow, DEC);
		  Serial.print("tCheckStabilize = ");
		  Serial.println(tCheckStabilize);
		}  	
	}
}

void AdaptiveRegulator::PerformBoostTemp()
{		
   if ( (long) (millis() - tBackToLow) >= 0)
   {  	    
		//check if target temp changed and adapt timings
		if (*mySetpoint > storedTargetTemp)
		{
			StartBoostToTarget();
		}
		 // wait for stabilization		 	 
				 
		// perform following checks every SAMPLE_DELAY when we reached tOperationalDelay since the temperature boost was started
		if ( ((long) (millis() - tCheckStabilize) >= 0)  && isNewSample && isDerivativeReliable)
		{ 	
			// check if stabilizing 
			if  (IsStabilizingOrDropping())
			{			
				Serial.println("STabilized !");
				FinishBoostTemp();
			}   
		}		 
   } else {  
		// switch ON heat and wait for tBackToLow
		 if (digitalRead(_pin) == LOW) {
		   turnOnRelay();
		 }	 		 
		 
		//check if target temp changed and adapt timings
		if (*mySetpoint != storedTargetTemp)
		{		
			float changeOffset =  *mySetpoint - storedTargetTemp;
			float newExpectedTempChange = expectedTempChange + changeOffset;
			
			tBackToLow = tStartBoostTemp + HeatingTimeNeeded(newExpectedTempChange);
			tCheckStabilize = tBackToLow + tOperationalDelay;
			storedTargetTemp = *mySetpoint;
			expectedTempChange = expectedTempChange + changeOffset;
			
			Serial.print("target temp changed, new tBackToLow = ");
			Serial.println(tBackToLow);
			Serial.print("target temp changed, new expectedTempChange = ");
			Serial.println(expectedTempChange);
		}
   }
}


void AdaptiveRegulator::FinishBoostTemp()
{      
	AdaptGain(actualTemp);
   
   Serial.println("FinishBoostTemp !");
   
   // enter REGULATE state
   EnterRegulateStateOrWaitSmoothLowering();
}


float AdaptiveRegulator::predictTemp(unsigned long horizon)
{
	float horizonSeconds = horizon/1000;
		
	// compute predicted value	
	return ((( tempPreviousArray[0] + tempPreviousArray[1] + tempPreviousArray[2] ) / 3 ) + (currentTempDerivative * horizonSeconds));
}

void AdaptiveRegulator::AdaptGain(float resultingTemp)
{
	// only take account of ON_Durations > burnupTime and make sure we waited tOperationalDelay
	unsigned long boostTempDuration = millis() - tStartBoostTemp;   
	unsigned long boostOnTempDuration = tLastTurnOffRelay - tStartBoostTemp;   
    if ( boostTempDuration > tOperationalDelay && boostOnTempDuration > burnupTime )
	{	
        float gain;
		if (boostType == LOWBOOST)
		{
			gain = secondPerDegreeGainSmall;
		}
		else
		{
			gain = secondPerDegreeGainLarge;
		}
			
			
		float actualTempChange = resultingTemp - actualTempAtBoostStart;
			
		if (actualTempChange < (expectedTempChange / 5) )
		{
			gain = gain * 1.8;
		} 
		else
		{
		   if (actualTempChange < (expectedTempChange / 2) )
		   {
				gain = gain * 1.4;
		   } 
		   else
		   {
			   if (expectedTempChange > 0.2 && actualTempChange > 0.1)
			   {
					// expectedTempChange > 0.2 serves to avoid big errors due to small changes
					gain = gain * expectedTempChange / actualTempChange;
			   } 		
		   }
		}
		
		// Make sure adapted gain stays in acceptable boundaries  (from secondPerDegreeGainRef/3 to secondPerDegreeGainRef*3)
			
		if (gain > secondPerDegreeGainRef*3)
			gain = secondPerDegreeGainRef*3;			
			
		if (gain < secondPerDegreeGainRef/3)
			gain = secondPerDegreeGainRef/3;
					
		switch (boostType)
		{
		case LOWBOOST:				
			secondPerDegreeGainSmall = gain;
			Serial.print("secondPerDegreeGainSmall =");
			Serial.println(secondPerDegreeGainSmall);
			break;
		case HIGHBOOST:	
			secondPerDegreeGainLarge = gain;
			Serial.print("secondPerDegreeGainLarge =");
			Serial.println(secondPerDegreeGainLarge);
			break;
		}
   }
}


void AdaptiveRegulator::StartInitialRamping()
{
   // enter FIRST RAMP state
   opState = FIRST_RAMP;

   // store initial temperature
   initialTemp = actualTemp;
   tStartFirstRamp = millis();

   setupCutOffTempForInitialRamping();
}


void AdaptiveRegulator::setupCutOffTempForInitialRamping()
{
	// calculate turn-off temperature
   firstRampCutOffTemp = initialTemp +  (*mySetpoint - initialTemp) * FIRST_RAMP_CUTOFF_RATIO;
   storedTargetTemp = *mySetpoint;
   
   Serial.print("firstRampCutOffTemp = ");
   Serial.println(firstRampCutOffTemp, DEC);
}

void AdaptiveRegulator::PerformFirstRamp()
{
    if (*mySetpoint != storedTargetTemp)
    {
		// target temp was changed ! Update firstRampCutOffTemp
		setupCutOffTempForInitialRamping();
    }
  
    if (actualTemp > firstRampCutOffTemp) 
    {
		// switch off heat and wait for stabilization
       if (digitalRead(_pin) == HIGH) {
         Serial.print("STOP at actualTemp = ");
         Serial.println(actualTemp, DEC);
         turnOffRelay();  
         tFirstRampCutOff = millis();
       }
       
       if ( isNewSample )
       {            
          // check if stabilizing near setpoint
          if  ((abs(actualTemp - initialTemp) > abs(*mySetpoint - actualTemp)) && IsStabilizingOrDropping())
          {
            FinishInitialRamping();
          }               
       }        
    } else {
      // heat fullsteam ahead
       if (digitalRead(_pin) == LOW)     turnOnRelay();
       
       // try to find how much time is needed for system to react to heat
       if (((long) (millis() - tCheckTakeOff) >= 0) && (tOperationalDelay == 0))
       {         
         tCheckTakeOff = millis() + SAMPLE_DELAY;
         
         // try to find how much time is needed for system to react to heat
         if(tempPreviousArray[0] > tempPreviousArray[1] && tempPreviousArray[1] > tempPreviousArray[2] && tempPreviousArray[2] > tempPreviousArray[3] && tempPreviousArray[3] > tempPreviousArray[4])
         {
           tOperationalDelay = (millis() - tStartFirstRamp - 3*SAMPLE_DELAY);
		   burnupTime = tOperationalDelay / 20; // arbitrary... to be perfected
           Serial.print("tOperationalDelay = ");
           Serial.println(tOperationalDelay, DEC);
         }          
       }
    }
}


void AdaptiveRegulator::FinishInitialRamping()
{
  // Return to normal control after we detected stabilization
  tEndFirstRamp = millis();
  
  // find top temperature before stabilization or drop
  float finalTemp = 0;
  for(int i=0;i<6;i++)
  {
	if (tempPreviousArray[i] > finalTemp)
	{
		finalTemp = tempPreviousArray[i];
	}
  }
  
  secondPerDegreeGainRef = (tFirstRampCutOff - tStartFirstRamp) / (1000*(finalTemp - initialTemp));
  secondPerDegreeGainLarge = secondPerDegreeGainRef;  
  secondPerDegreeGainSmall = secondPerDegreeGainLarge;

   Serial.print("FinishInitialRamping !   tEndFirstRamp = ");
   Serial.println(tEndFirstRamp, DEC);
   Serial.print("secondPerDegreeGainLarge = ");
   Serial.println(secondPerDegreeGainLarge);

   
  // enter REGULATE state
   EnterRegulateStateOrWaitSmoothLowering();
}


void AdaptiveRegulator::turnOnRelay()
{
  	Serial.println("HEAT ON !");
  //digitalWrite(RELAY_OUT_PIN,HIGH);
  digitalWrite(_pin,1);
  tCheckNotHeatingWildly = millis() + ((unsigned long)60000 * MAX_HEATINGTIME_NO_TEMP_CHANGE_MINUTES);
  Serial.println("tCheckNotHeatingWildly =");
  Serial.println(tCheckNotHeatingWildly, DEC);
  tempBeforeHeating = actualTemp;
  isHeatOn = true;
}
    
void AdaptiveRegulator::turnOffRelay()
{
  //digitalWrite(RELAY_OUT_PIN,LOW);
  digitalWrite(_pin,0);
  tLastTurnOffRelay = millis();
  tCheckNotHeatingWildly = 0;
  isHeatOn = false;
}    
    
// Security checks    
void AdaptiveRegulator::checkShutdownConditions(){
  boolean doShutdown = false;
  
  // check for too long uptime
  if ( (long) (millis() - maxUptimeMillis) >= 0)
  {
    Serial.println(maxUptimeMillis);
    doShutdown = true;
  }
  
  // check for too high temperature
  if (actualTemp > SHUTDOWN_TEMP)
  {
    Serial.println(actualTemp);
    doShutdown = true;
  }
  
  // check for too long heating time with no temperature increase (temp probe can't be not trusted anymore so stop the device)
  if (tCheckNotHeatingWildly > 0 && isHeatOn && ( (long) (millis() - tCheckNotHeatingWildly) >= 0))
  {
    if (actualTemp <= tempBeforeHeating)
    {
      // temperature did not increase even if we kept on heating during MAX_HEATINGTIME_NO_TEMP_CHANGE_MINUTES
      Serial.println("MAX_HEATINGTIME_NO_TEMP_CHANGE_MINUTES !");
      doShutdown = true;
    }
    // plan next check
    tempBeforeHeating = actualTemp;
	tCheckNotHeatingWildly = millis() + ((unsigned long)60000 * MAX_HEATINGTIME_NO_TEMP_CHANGE_MINUTES);
  }
  
  if (doShutdown == true)
  {
    shutdownDevice();
  }
}


void AdaptiveRegulator::shutdownDevice() 
{
    Serial.println("SHUTDOWN");
    // turn off relay !
    //digitalWrite(RELAY_OUT_PIN,LOW);
	digitalWrite(_pin,0);
	isHeatOn = false;
}

void AdaptiveRegulator::SetApproximatePulseDurationsForREgulation(float tempLost, unsigned long regDelay )
{
	// calculate needed uptime to compensate
	unsigned long neededUptimeForCompensate = tempLost * secondPerDegreeGainRef * 1000;
	SetPulseDurationsForREgulation(neededUptimeForCompensate, regDelay );
}

void AdaptiveRegulator::SetPulseDurationsForREgulation(unsigned long neededUptimeForCompensate, unsigned long regDelay )
{	
	Serial.print(" --- neededUptimeForCompensate = ");
	Serial.println(neededUptimeForCompensate);
	
	// evenly distribute needed uptime						
	if (neededUptimeForCompensate >= regDelay) 
	{
		// we would need full ontime! Call for a temp boost instead with slight overshoot
		StartBoostToTarget(0.2);					
	} 
	else 
	{
		// ensure pulses (ON and OFF periods) will not violate MIN_SWITCHING_TIME
		while ( (regDelay / 2) < MIN_SWITCHING_TIME )
		{
			neededUptimeForCompensate = neededUptimeForCompensate * 2;
			regDelay = regDelay *2 ;
		}
		while ( (neededUptimeForCompensate / 2) < MIN_SWITCHING_TIME )
		{
			neededUptimeForCompensate = neededUptimeForCompensate * 2;
			regDelay = regDelay *2 ;
		}
		while ( (regDelay - neededUptimeForCompensate ) < MIN_SWITCHING_TIME )
		{
			neededUptimeForCompensate = neededUptimeForCompensate * 2;
			regDelay = regDelay *2 ;
		}
		
		// 
		int nbOnPulsePerRegPeriod = (int) neededUptimeForCompensate / MIN_SWITCHING_TIME;
		int remainder = (int) neededUptimeForCompensate % MIN_SWITCHING_TIME;
		durationOnPulse = MIN_SWITCHING_TIME + ((unsigned long)(remainder / nbOnPulsePerRegPeriod));
		durationOffPulse = (regDelay - neededUptimeForCompensate) / nbOnPulsePerRegPeriod;
		
		// make sure OFF time is also greater than minimum switching time
		while ( durationOffPulse < MIN_SWITCHING_TIME )
		{
			durationOffPulse = durationOffPulse * 2;
			durationOnPulse = durationOnPulse *2 ;
		}
		
		// store that we have good parameters for this temperature
		parametersRegulationSetForTemp = *mySetpoint;
		
		Serial.print("durationOffPulse = ");
		Serial.print(durationOffPulse);
		Serial.print("   durationOnPulse = ");
		Serial.println(durationOnPulse);
	}
}
// 

/**************************************************************************************/
/*                                                                                    */
/*                                    UTILITIES                                       */
/*                                                                                    */
/**************************************************************************************/


// ------------------------- temperature array UTILITIES

void AdaptiveRegulator::tempPreviousArrayPushValue(float val)
{
		tempPreviousArray[5] = tempPreviousArray[4];
		tempPreviousArray[4] = tempPreviousArray[3];
	    tempPreviousArray[3] = tempPreviousArray[2];
	    tempPreviousArray[2] = tempPreviousArray[1];
	    tempPreviousArray[1] = tempPreviousArray[0];
	    tempPreviousArray[0] = val;
}

// ------------------------- derivative and temperature trend UTILITIES

bool AdaptiveRegulator::IsStabilizingOrDropping()
{
	bool toReturn = false;
	if (isDerivativeReliable && (tempPreviousArray[0] <= tempPreviousArray[1] && tempPreviousArray[1] <= tempPreviousArray[2] && tempPreviousArray[2] <= tempPreviousArray[3] && tempPreviousArray[3] <= tempPreviousArray[4]  && tempPreviousArray[4] <= tempPreviousArray[5])) toReturn = true;	
	//(currentTempDerivative < 0.001)
	return toReturn;
}


bool AdaptiveRegulator::IsStabilizingOrGrowing()
{
	bool toReturn = false;
	if (isDerivativeReliable && (currentTempDerivative >= 0)) toReturn = true;
	return toReturn;
}

bool AdaptiveRegulator::IsStabilizing()
{
	bool toReturn = false;
	if (isDerivativeReliable && (abs(currentTempDerivative) <= 0.001)) toReturn = true;
	return toReturn;
}

bool AdaptiveRegulator::IsFallingNaturally()
{
	bool toReturn = false;
	if (isDerivativeReliable && currentTempDerivative > -0.006 && currentTempDerivative <= 0 ) toReturn = true;
	return toReturn;
}

bool AdaptiveRegulator::IsFalling()
{
	bool toReturn = false;
	if (isDerivativeReliable && currentTempDerivative > -0.1 && currentTempDerivative < -0.007 ) toReturn = true;
	return toReturn;
}

bool AdaptiveRegulator::IsAcceleratingFall()
{
	bool toReturn = false;
	if (isDerivativeReliable && currentTempDerivative < previousDerivative &&  previousDerivative < 0 ) toReturn = true;
	return toReturn;
}



// ------------------------- other UTILITIES

void AdaptiveRegulator::soundAlarm()
{
  Serial.println("Alarm!");
}

void AdaptiveRegulator::alertTemperatureNearlySet()
{
  if (isWaitingForTempAlert == true && abs(*mySetpoint - actualTemp) < 0.3)
  {
    soundAlarm();
    isWaitingForTempAlert = false;
  }  
}



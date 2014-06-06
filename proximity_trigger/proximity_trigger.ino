//  Copyright (c) 2010 seeed technology inc.
//  Author: Steve Chang
//  Date: September 2, 2010
//  Author: Brian Fiegel
//  Date: May 22. 2014
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <avr/wdt.h>

#define USE_ENABLE_ULTRASONIC_SENSOR 0
#define USE_ENABLE_LIGHT_SENSOR      1
#define numberof(x) (sizeof((x))/sizeof 0[(x)])

#if USE_ENABLE_LIGHT_SENSOR
#define PHOTO_RESISTOR_PIN A0
#endif

#define ENABLE_DELAY_MS    (1 * 60 * 1000)
#define ENABLE_DISTANCE_CM (10)
#define ENABLE_PRESENCE_COUNTER_RESET (100)
#define ENABLE_PRESENCE_COUNTER_INIT (100)

#define TRIGGER_DISTANCE_CM (20)
#define TRIGGER_PRESENCE_COUNTER_RESET (5)
#define TRIGGER_PRESENCE_COUNTER_INIT (0)

#define TRIGGER_ABSENCE_COUNTER_RESET (10)
#define TRIGGER_ABSENCE_COUNTER_INIT (100)/* Initially assume that nothing has been around */

unsigned long enableStartTime;
unsigned int  enablePresenceCounter  = ENABLE_PRESENCE_COUNTER_INIT;
unsigned int  triggerPresenceCounter = TRIGGER_PRESENCE_COUNTER_INIT;
unsigned int  triggerAbsenceCounter  = TRIGGER_ABSENCE_COUNTER_INIT;
unsigned char triggerPin[] = {4};

class Ultrasonic
{
	public:
		Ultrasonic(int pin);
        void DistanceMeasure(void);
		long microsecondsToCentimeters(void);
		long microsecondsToInches(void);
	private:
		int _pin;//pin number of Arduino that is connected with SIG pin of Ultrasonic Ranger.
        long duration;// the Pulse time received;
};
Ultrasonic::Ultrasonic(int pin)
{
	_pin = pin;
}
/*Begin the detection and get the pulse back signal*/
void Ultrasonic::DistanceMeasure(void)
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(_pin,LOW);
  pinMode(_pin,INPUT);
  duration = pulseIn(_pin,HIGH);
}
/*The measured distance from the range 0 to 400 Centimeters*/
long Ultrasonic::microsecondsToCentimeters(void)
{
	return duration/29/2;	
}
/*The measured distance from the range 0 to 157 Inches*/
long Ultrasonic::microsecondsToInches(void)
{
	return duration/74/2;	
}

#if USE_ENABLE_ULTRASONIC_SENSOR
Ultrasonic enableUltrasonic(10);
#endif
Ultrasonic triggerUltrasonic(9);

void setup()
{
  int i;
  
  Serial.begin(115200);
  
  for(i = 0; i < numberof(triggerPin); ++i)
  {
    pinMode(triggerPin[i],OUTPUT);
    digitalWrite(triggerPin[i],LOW);
  }
  
  enableStartTime = millis();
  wdt_enable (WDTO_2S);  // reset after two seconds, if no "pat the dog" received
}

void setTrigger()
{
  int i;
  for(i = 0; i < numberof(triggerPin); ++i)
  {
    digitalWrite(triggerPin[i],HIGH);
    delay(20); //delay between turning on each relay, 20 milliseconds
    Serial.print("set relay ");
    Serial.println(triggerPin[i]);
  }
}

void resetTrigger()
{
  int i;
  for(i = 0; i < numberof(triggerPin); ++i)
  {
    digitalWrite(triggerPin[i],LOW);
    delay(20); //delay between turning on each relay, 20 milliseconds
    Serial.print("reset relay ");
    Serial.println(triggerPin[i]);
  }
}

void loop()
{
  unsigned long time;
  int      lightLevel;
  
  int i=0;
  long EnableRangeInCentimeters, TriggerRangeInCentimeters;
  
  wdt_reset ();
#if USE_ENABLE_LIGHT_SENSOR
  lightLevel = analogRead(PHOTO_RESISTOR_PIN);
#endif
#if USE_ENABLE_ULTRASONIC_SENSOR
  enableUltrasonic.DistanceMeasure();
  EnableRangeInCentimeters = enableUltrasonic.microsecondsToCentimeters();//convert the time to centimeters
#endif
  triggerUltrasonic.DistanceMeasure();
  TriggerRangeInCentimeters = triggerUltrasonic.microsecondsToCentimeters();//convert the time to centimeters
#if 1
  Serial.print("Enable Start Time: ");
  Serial.println(enableStartTime);
  Serial.print("Now: ");
  Serial.println(millis());
#if USE_ENABLE_ULTRASONIC_SENSOR
  Serial.print("Distance to enable object: ");
  Serial.print(EnableRangeInCentimeters);//0~400cm
  Serial.println(" cm");
  Serial.print("Enable Counts: ");
  Serial.println(enablePresenceCounter);
#endif
#if USE_ENABLE_LIGHT_SENSOR
  Serial.print("Light Level is: ");
  Serial.println(lightLevel);
#endif
  Serial.print("Distance to trigger object: ");
  Serial.print(TriggerRangeInCentimeters);//0~400cm
  Serial.println(" cm");
  Serial.print("Trigger Counts: ");
  Serial.println(triggerPresenceCounter);
#endif

  delay(10);
  if (
#if !USE_ENABLE_ULTRASONIC_SENSOR && !USE_ENABLE_LIGHT_SENSOR
  1
#else
# if USE_ENABLE_ULTRASONIC_SENSOR
  (EnableRangeInCentimeters <= ENABLE_DISTANCE_CM)
# elif USE_ENABLE_LIGHT_SENSOR
  (lightLevel < 100)
# else
  0
# endif
#endif
  )
  {
    if(0 == enablePresenceCounter)
    {
      time = millis();
  
      if (time < enableStartTime) 
      {/* time has wrapped, just reset the start to 0 */
        enableStartTime = 0;
      }
      
      if (time > (enableStartTime + ENABLE_DELAY_MS))
      {
        if (TriggerRangeInCentimeters <= TRIGGER_DISTANCE_CM)
        {
          /* under the trigger threshold, set the trigger once this condition appears stable */
          if (0 == triggerAbsenceCounter)
          {
            triggerPresenceCounter = TRIGGER_PRESENCE_COUNTER_RESET;
            setTrigger();
          }
          else
          {
            --triggerAbsenceCounter; 
          }
        }
        else
        {
          /* above the trigger theshold, reset the trigger once this condition appears stable */
          if (0 == triggerPresenceCounter)
          {
            triggerAbsenceCounter = TRIGGER_ABSENCE_COUNTER_RESET;
            resetTrigger();
          } 
          else
          {
            --triggerPresenceCounter; 
          }
        }
      }
      else
      {
        /* Don't do anything until enabled for specified time period */  
      }
    }
    else
    {
      --enablePresenceCounter;
    }
  }
  else
  {
    resetTrigger();
    enableStartTime       = millis();
    enablePresenceCounter = ENABLE_PRESENCE_COUNTER_RESET;
  }  
}

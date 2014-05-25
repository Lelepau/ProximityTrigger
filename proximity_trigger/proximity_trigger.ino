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

#define numberof(x) (sizeof((x))/sizeof 0[(x)])

#define PHOTO_RESISTOR_PIN A0

#define ENABLE_DELAY_MS    (60 * 1000)
#define ENABLE_DISTANCE_CM (10)
#define ENABLE_PRESENCE_COUNTER_RESET (10)
#define ENABLE_PRESENCE_COUNTER_INIT (0)

#define TRIGGER_DISTANCE_CM (30)
#define TRIGGER_PRESENCE_COUNTER_RESET (100)
#define TRIGGER_PRESENCE_COUNTER_INIT (0)

#define TRIGGER_ABSENCE_COUNTER_RESET (10)
#define TRIGGER_ABSENCE_COUNTER_INIT (100)/* Initially assume that nothing has been around */

unsigned long enableStartTime;
unsigned int  enablePresenceCounter  = ENABLE_PRESENCE_COUNTER_INIT;
unsigned int  triggerPresenceCounter = TRIGGER_PRESENCE_COUNTER_INIT;
unsigned int  triggerAbsenceCounter  = TRIGGER_ABSENCE_COUNTER_INIT;
unsigned char relayPin[] = {7};

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

Ultrasonic enableUltrasonic(10);
Ultrasonic triggerUltrasonic(9);

void setup()
{
  int i;
  
  Serial.begin(115200);
  
  for(i = 0; i < numberof(relayPin); ++i)
  {
    pinMode(relayPin[i],OUTPUT);
    digitalWrite(relayPin[i],LOW);
  }
  
  enableStartTime = millis();
  wdt_enable (WDTO_2S);  // reset after two seconds, if no "pat the dog" received
}

void loop()
{
  unsigned long time;
  int      lightLevel;
  
  int i=0;
  long EnableRangeInCentimeters, TriggerRangeInCentimeters;
  
  wdt_reset ();
  lightLevel = analogRead(PHOTO_RESISTOR_PIN); 
  enableUltrasonic.DistanceMeasure();
  triggerUltrasonic.DistanceMeasure();
  EnableRangeInCentimeters = enableUltrasonic.microsecondsToCentimeters();//convert the time to centimeters
  TriggerRangeInCentimeters = triggerUltrasonic.microsecondsToCentimeters();//convert the time to centimeters
  Serial.print("Distance to enable object: ");
  Serial.print(EnableRangeInCentimeters);//0~400cm
  Serial.println(" cm");
  Serial.print("Distance to trigger object: ");
  Serial.print(EnableRangeInCentimeters);//0~400cm
  Serial.println(" cm");
  Serial.print("Enable Counts: ");
  Serial.println(enablePresenceCounter);
  Serial.print("Trigger Counts: ");
  Serial.println(triggerPresenceCounter);

  delay(10);
  if ((EnableRangeInCentimeters <= ENABLE_DISTANCE_CM) || (lightLevel > 500))
  {
    if(0 == enablePresenceCounter)
    {
      time = millis();
  
      if (time < enableStartTime) 
      {
        enableStartTime = 0;
      }
      
      if (time > enableStartTime)
      {
        if (TriggerRangeInCentimeters <= TRIGGER_DISTANCE_CM)
        {
          if (0 == triggerAbsenceCounter)
          {
            triggerPresenceCounter = TRIGGER_PRESENCE_COUNTER_RESET;
            for(i = 0; i < numberof(relayPin); i++)
            {
              digitalWrite(relayPin[i],HIGH);
              delay(20); //delay between turning on each relay, 20 milliseconds
            }
          }
          else
          {
            --triggerAbsenceCounter; 
          }
        }
        else
        {
          if (0 == triggerPresenceCounter)
          {
            triggerAbsenceCounter = TRIGGER_ABSENCE_COUNTER_RESET;
            for(i = 0; i < numberof(relayPin); i++)
            {
              digitalWrite(relayPin[i],LOW);
              delay(20); //delay between turning off each relay, 20 milliseconds
            }
          } 
          else
          {
            --triggerPresenceCounter; 
          }
        }
      }
      else
      {
        
      }
    }
    else
    {
      --enablePresenceCounter;
    }
  }
  else
  {
    enableStartTime       = 0;
    enablePresenceCounter = ENABLE_PRESENCE_COUNTER_RESET;
  }  
}

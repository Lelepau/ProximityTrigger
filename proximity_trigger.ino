//  RelayShieldDemoCode.pde  to control seeed relay shield by arduino.
//  Copyright (c) 2010 seeed technology inc.
//  Author: Steve Chang
//  Version: september 2, 2010
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

unsigned int  presenseCounter = 0;
unsigned char relayPin[4] = {7};//4,5,6,7};

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

Ultrasonic ultrasonic(9);

void setup()
{
  int i;
  
  Serial.begin(115200);
  
  for(i = 0; i < 4; i++)
  {
    pinMode(relayPin[i],OUTPUT);
    digitalWrite(relayPin[i],LOW);
  }
}

void loop()
{

  int i=0;
  long RangeInInches;
  long RangeInCentimeters;
  
  ultrasonic.DistanceMeasure();// get the current signal time;
  RangeInInches = ultrasonic.microsecondsToInches();//convert the time to inches;
  RangeInCentimeters = ultrasonic.microsecondsToCentimeters();//convert the time to centimeters
  Serial.println("The distance to obstacles in front is: ");
  Serial.print(RangeInInches);//0~157 inches
  Serial.println(" inch");
  Serial.print(RangeInCentimeters);//0~400cm
  Serial.println(" cm");
  Serial.print(presenseCounter);
  Serial.println(" Counts");
  delay(10);
  
  if (RangeInInches <= 48)
  {
    presenseCounter = 100;
    for(i = 0; i < 4; i++)
    {
      digitalWrite(relayPin[i],HIGH);
      delay(20); //delay between turning on each relay, 200 milliseconds
    }
  }
  else
  {
    if (0 == presenseCounter)
    {
      for(i = 0; i < 4; i++)
      {
        digitalWrite(relayPin[i],LOW);
        delay(20); //delay between turning off each relay, 200 milliseconds
      }
    } 
    else
    {
      --presenseCounter; 
    }
  }  
}

/*
 * Temperature control software for a Reflow Soldering Oven
 * This firmware can be used on a ATtiny 45 µController
 * For schematics of the hardware, please see
 * http://www.pleasantsoftware.com/developer/3d/reflow
 *
 * (c) 2011 Eberhard Rensch, Pleasant Software
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#include <Bounce.h>
#include "Thermistor.h"
#include "Heater.h"

// Use these Pins on a ATtiny
  uint8_t sensorPin = 3;
  uint8_t heaterPin = 0;
  uint8_t ledPin = 1;
  uint8_t buttonPin= 2;
    
// These pins might be used for debugging with an Arduino instead
//  uint8_t sensorPin = 3;
//  uint8_t heaterPin = 2;
//  uint8_t ledPin = 3;
//  uint8_t buttonPin= 4;


Thermistor  thermistor(sensorPin, 0);
Heater heater;
Bounce button = Bounce(buttonPin, 10);

void setup() {
#if !defined(__AVR_ATtiny45__)    
  Serial.begin(115200); 
  Serial.println("Init");
#endif

  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  digitalWrite(heaterPin, LOW);

  heater.set_target_temperature(0);
  
  // Signal ready for duty
//  for(uint8_t i=0;i<5;i++) {
//     delay(100);
//     digitalWrite(ledPin,HIGH);
//     delay(150);
//     digitalWrite(ledPin,LOW);
//   }

}

uint8_t phase = 0;
uint8_t program = 0;

boolean running = false;
boolean programFinished = false;

uint8_t ledState=0;
uint8_t ledPhase = 0;

uint32_t blinkTimer=0L;
uint32_t tempManagerTimer=0L;
uint32_t currentTime=0L;
uint32_t nextPhaseChange=0;
boolean buttonHandled=true;

void loop() 
{ 
  boolean buttonChanged = button.update();
  currentTime = millis();
  if(running)
  {
    if(blinkTimer<currentTime)
    {
      ledPhase+=ledState;
      ledState = 1-ledState;
      digitalWrite(ledPin, ledState);
      blinkTimer=currentTime+100UL;
      if(ledPhase>(phase-1)/2) {
        blinkTimer+=500UL;
        ledPhase = 0;
      }
    }
    if(tempManagerTimer<currentTime)
    {
      tempManagerTimer = currentTime+500UL;
      heater.manage_temperature();

#if !defined(__AVR_ATtiny45__)    
      Serial.print("Phase ");
      Serial.print(phase);
      Serial.print(" SollTemp: ");
      Serial.print(heater.get_target_temperature(),DEC);
      Serial.print(" IstTemp: ");
      Serial.print(thermistor.getTemperature(),DEC);
      Serial.print(" nextTime: ");
      Serial.print(nextPhaseChange/1000L,DEC);
      Serial.print(" current Time: ");
      Serial.println(currentTime/1000L,DEC);
#endif
    }
    if(buttonChanged && button.read()==LOW) {
      heater.set_target_temperature(0);
      digitalWrite(heaterPin, LOW);
      digitalWrite(ledPin, LOW);
#if !defined(__AVR_ATtiny45__)    
      Serial.print("Program Abort");
#endif
      running=false;
    }
    else {
      if( (nextPhaseChange!=-1UL && nextPhaseChange<currentTime) || (nextPhaseChange==-1UL && heater.hasReachedTargetTemperature()) )
      {
 #if !defined(__AVR_ATtiny45__)    
       Serial.print("PhaseChange to ");
       Serial.println(phase, DEC);
 #endif
        switch(phase++)
        {
        case 0:  
          heater.set_target_temperature(150);
          nextPhaseChange = -1UL;
          break;
        case 1:  
          nextPhaseChange = currentTime + 40000L; // Hold Temperature for 40 Seconds
          break;
        case 2:  
          heater.set_target_temperature(190);
          nextPhaseChange = -1UL;
          break;
        case 3:  
          nextPhaseChange = currentTime + 10000L; // Hold Temperature for 10 Seconds
          break;
        case 4:  
          heater.set_target_temperature(245);
          nextPhaseChange = -1UL;
          break;
        case 5:  
          nextPhaseChange = currentTime + 10000L; // Hold Temperature for 20 Seconds
          break;
        case 10: // Always on
          heater.set_target_temperature(500);
          nextPhaseChange = -1UL; // Hold Temperature for 10 Seconds
          // This phase never ends (500°C are never reached in a usual toaster oven!)
          break;
        case 20: // One hour 110°C
          heater.set_target_temperature(110);
          nextPhaseChange = currentTime + 3600000UL; // Hold Temperature for 1 hour
          break;
        default: 
          heater.set_target_temperature(0);
          programFinished = true;
          running=false;
#if !defined(__AVR_ATtiny45__)    
          Serial.println("Program End");
#endif
          digitalWrite(heaterPin, LOW); 
          digitalWrite(ledPin, HIGH);
          break;
        }
      }
    }

  }
  else {
    if(tempManagerTimer<currentTime)
    {
      tempManagerTimer = currentTime+1000L;
      thermistor.update();
#if !defined(__AVR_ATtiny45__)    
      Serial.print("IstTemp: ");
      Serial.println(thermistor.getTemperature());
#endif
      if(programFinished && thermistor.getTemperature()<55)
         resetProgram();
    }

    if( button.read()==LOW ) {
      if(button.duration() > 1000) {
        running = true;
        programFinished = false;
        switch(program)
        {
           case 0: phase=0;
                   break;
           case 1: phase=20;
                   break;
           case 2: phase=10;
                   break;
        }
        //phase = (program==0)?0:10; // 10 -> Always on!
        nextPhaseChange = 0L;
        buttonHandled=true;
 #if !defined(__AVR_ATtiny45__)    
        Serial.println("Start Program");
 #endif
      }
      else if(buttonChanged && programFinished)
      {
        resetProgram();
        buttonHandled=true;
      }
    }  
    if(buttonChanged && button.read()==HIGH)
    {
      if(!buttonHandled) {
        program = (program+1)%3;
        for(uint8_t i=0;i<=program;i++)
        {
           delay(200);
           digitalWrite(ledPin,HIGH);
           delay(250);
           digitalWrite(ledPin,LOW);
        }

#if !defined(__AVR_ATtiny45__)    
        Serial.println("Change Program");
#endif
      }
      buttonHandled=false;
    }  
  }
}

void resetProgram()
{
  programFinished = false;
  digitalWrite(ledPin, LOW);
#if !defined(__AVR_ATtiny45__)    
  Serial.println("Reset Program");
#endif
}

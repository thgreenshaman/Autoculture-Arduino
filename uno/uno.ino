///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Autoculture-Arduino v0.01                                                 //
//                                                                           //
//    Copyright (C) 2021 Damian Mair(thgreenshaman) ->/dmTech                //
//                                                                           //
//    This program is free software: you can redistribute it and/or modify   //
//    it under the terms of the GNU General Public License as published by   //
//    the Free Software Foundation, either version 3 of the License, or      //
//    (at your option) any later version.                                    //
//                                                                           //
//    This program is distributed in the hope that it will be useful,        //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                           //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.  //
//                                                                           //
//---------------------------------------------------------------------------//
//
//**Commented out Serial commands are for debugging
//
//**If you are using this on an Uno the last uncommented serial line is not needed
//**as it is sending current data via serial communication to ESP8266.

#include "RTClib.h"
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPIN 2
#define fanRelay 6
#define lightRelay 7
#define heatpadRelay 8
#define peltierRelay 9

int tempLowOn = 18;
int tempHighOn = 28;
int tempLowOff = 12;
int tempHighOff = 22;
int shutDown = 40;
bool lights;
float t;
float h;

String curMin;
String curHour;
String curTime;

DHT dht(DHTPIN, DHTTYPE);

RTC_DS1307 rtc;
DateTime now;

void setup() {
  Serial.begin(57600);
  dht.begin();
  delay(1000);

  pinMode(fanRelay, OUTPUT);
  pinMode(lightRelay, OUTPUT);
  pinMode(heatpadRelay, OUTPUT);
  pinMode(peltierRelay, OUTPUT);

  if (! rtc.begin()) {
    Serial.println("RTC not connected or malfunctioning!!!");
    Serial.flush();
    abort();
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (! rtc.isrunning()) {
    Serial.println("RTC is not running.");
    Serial.println("Setting time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}


void safetyCheck() {
  t = dht.readTemperature();
  while (t >= shutDown) { ///Shutdown lights and heatpad in case of overheating, keep fans and peltier on to cool unit.
    digitalWrite(lightRelay, LOW);
    //Serial.println("Lights OFF");
    digitalWrite(heatpadRelay, LOW);
    //Serial.println("Heatpad OFF");
    digitalWrite(fanRelay, HIGH);
    //Serial.println("Fan ON");
    digitalWrite(peltierRelay, HIGH);
    //Serial.println("Fan ON");
  }
  return;
}
void tempCheck() {

  h = dht.readHumidity();
  t = dht.readTemperature();
  //Serial.print("Temperature:\t");
  //Serial.println(t);
  //Serial.print("Humidity:\t");
  //Serial.println(h);
  //Serial.println("-------------------------");

  safetyCheck();
  if (lights == true) {
    if (t <= tempLowOn) { //Turn on heatpad and turn on fans and peltier if temps are below tempLow
      digitalWrite(fanRelay, LOW);
      //Serial.println("Fan OFF");
      digitalWrite(heatpadRelay, HIGH);
      //Serial.println("Heatpad ON");
      digitalWrite(peltierRelay, LOW);
      //Serial.println("Peltier OFF");

    } else if (t >= tempHighOn) { //Turn of heatpad and turn on peltier and fans if temp rises above tempHigh
      digitalWrite(fanRelay, HIGH);
      //Serial.println("Fan ON");
      digitalWrite(peltierRelay, HIGH);
      //Serial.println("Peltier ON");
      digitalWrite(heatpadRelay, LOW);
      //Serial.println("Heatpad OFF");

    } else if (t > tempLowOn + 2 && t < tempHighOn - 1) {
      digitalWrite(fanRelay, HIGH);
      //Serial.println("Fan ON");
      digitalWrite(peltierRelay, LOW);
      //Serial.println("Peltier OFF");
      digitalWrite(heatpadRelay, LOW);
      //Serial.println("Heatpad OFF");
    }
  } else if (lights == false) {
    if (t <= tempLowOff) { //Turn on heatpad and turn on fans and peltier if temps are below tempLow
      digitalWrite(fanRelay, LOW);
      //Serial.println("Fan OFF");
      digitalWrite(heatpadRelay, HIGH);
      //Serial.println("Heatpad ON");
      digitalWrite(peltierRelay, LOW);
      //Serial.println("Peltier OFF");

    } else if (t >= tempHighOff) { //Turn of heatpad and turn on peltier and fans if temp rises above tempHigh
      digitalWrite(fanRelay, HIGH);
      //Serial.println("Fan ON");
      digitalWrite(peltierRelay, HIGH);
      //Serial.println("Peltier ON");
      digitalWrite(heatpadRelay, LOW);
      //Serial.println("Heatpad OFF");

    } else if (t > tempLowOff + 2 && t < tempHighOff - 2) {
      digitalWrite(fanRelay, HIGH);
      //Serial.println("Fan ON");
      digitalWrite(peltierRelay, LOW);
      //Serial.println("Peltier OFF");
      digitalWrite(heatpadRelay, LOW);
      //Serial.println("Heatpad OFF");
    }
  }
}

void loop() {

  while (isnan(dht.readHumidity()) || isnan(dht.readTemperature())) {

    Serial.println(F("Failed to read from DHT sensor!"));

  }
  now = rtc.now();
  if (int(now.hour()) >= 11 and int(now.hour()) < 17 ) {
    lights = false;
    digitalWrite(lightRelay, LOW);
  }
  else if (int(now.hour()) < 11 or int(now.hour()) >= 17 ) {
    lights = true;
    digitalWrite(lightRelay, HIGH);
  }

  safetyCheck();
  tempCheck();
  if (String(now.minute()).length() == 2) {
    curMin = String(now.minute());
  } else if (String(now.minute()).length() == 1) {
    curMin = "0" + String(now.minute());
  }
  if (String(now.hour()).length() == 2) {
    curHour = String(now.hour());
  } else if (String(now.hour()).length() == 1) {
    curHour = "0" + String(now.hour());
  }

  Serial.print(String(digitalRead(lightRelay))
               + ":" + String(digitalRead(fanRelay))
               + ":" + String(digitalRead(heatpadRelay))
               + ":" + String(digitalRead(peltierRelay))
               + ":" + String(t)
               + ":" + String(h)
               + curHour + ":" + curMin
               + ";");


  Serial.flush();
  delay(2999);

}

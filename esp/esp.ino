///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Autoculture-Arduino v0.01                                                     //
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


#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "config.h"


AsyncWebServer server(80);

//Set your WiFi crendentials in config.h file.
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

String temp;
String hum;
String lightState;
String heatpadState;
String fanState;
String peltierState;
String unoData;
String unoTime;

String processor(const String& var) {
  Serial.println(var);
  if (var == "TEMP") {
    return temp;
  } else if (var == "HUM") {
    return hum;
  } else if (var == "PELTIERSTATE") {
    return peltierState;
  } else if (var == "LIGHTSTATE") {
    return lightState;
  } else if (var == "HEATPADSTATE") {
    return heatpadState;
  } else if (var == "FANSTATE") {
    return fanState;
  } else if (var == "UNOTIME") {
    return unoTime;
  }
}

void readUno() {
  unoData = Serial.readStringUntil(';');
  if (unoData.length() == 24){

    if (unoData.substring(0,1) == "0"){
      lightState = "OFF";
    } else if (unoData.substring(0,1) == "1"){
      lightState = "ON";
    }

    if (unoData.substring(2,3) == "0"){
      fanState = "OFF";
    } else if (unoData.substring(2,3) == "1"){
      fanState = "ON";
    }

    if (unoData.substring(4,5) == "0"){
      heatpadState = "OFF";
    } else if (unoData.substring(4,5) == "1"){
      heatpadState = "ON";
    }
    if (unoData.substring(6,7) == "0"){
      peltierState = "OFF";
    } else if (unoData.substring(6,7) == "1"){
      peltierState = "ON";
    }
    temp = unoData.substring(8,13);
    hum = unoData.substring(14,19);
    unoTime = unoData.substring(19);
  }


    delay(3000);


}




void setup() {
  Serial.begin(57600);

  if (!SPIFFS.begin()) {
    Serial.println("!!!_Error mounting SPIFFS_!!!");
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("\nConnected on :");
  Serial.println(WiFi.localIP());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", temp.c_str());
  });

  server.on("/hum", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", hum.c_str());
  });

  server.on("/fs", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", fanState.c_str());
  });

  server.on("/hs", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", heatpadState.c_str());
  });

  server.on("/ls", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", lightState.c_str());
  });

  server.on("/ps", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", peltierState.c_str());
  });

  server.on("/unotime", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", unoTime.c_str());
  });

  server.begin();

  if (!MDNS.begin("autoculture")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }


}


void loop() {
  MDNS.update();
  readUno();
}

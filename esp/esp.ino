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
#include <ArduinoOTA.h>
#include "config.h"


AsyncWebServer server(80);

//Set your WiFi crendentials in config.h file.
const char* wifi_ssid = WIFI_SSID;
const char* wifi_password = WIFI_PASSWORD;

const char* http_user = HTTP_USER;
const char* http_pass = HTTP_PASS;

const char* ota_pass = OTA_PASS;

const char* ap_ssid = AP_SSID;
const char* ap_password = AP_PASSWORD;

String temp;
String hum;
String lightState;
String heatpadState;
String fanState;
String peltierState;
String unoData;
String unoTime;

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<style>
  body {
    background-color:#003d00;
    font-family: Comfortaa, roboto, Verdana, sans-serif;
  }
  #loggedout {
    background-color:#629749;
    border-radius:4em;
    padding:20px;
    width:300px;
  }
  #footer {
    color:white;
  }
  div {
    margin:auto;
  }
</style>
<body align='center'>
    <div id='titleBar'>
      <center>
      <img src=' data:image/png;base64,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LmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNC40LjAtRXhpdjIiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iCiAgICB4bWxuczpzdEV2dD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlRXZlbnQjIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6R0lNUD0iaHR0cDovL3d3dy5naW1wLm9yZy94bXAvIgogICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iCiAgICB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iCiAgIHhtcE1NOkRvY3VtZW50SUQ9ImdpbXA6ZG9jaWQ6Z2ltcDoyNjAyNGM0ZC02NTM5LTRlYTQtODNkMS0wNWIwZDViZmRlNzEiCiAgIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6YTRlMDYwNDctMjMyNi00OTYwLTgwYzYtOTM0ZWUyMDNiMDVhIgogICB4bXBNTTpPcmlnaW5hbERvY3VtZW50SUQ9InhtcC5kaWQ6NjdmNzFlNWYtMzNmYS00MGQ3LWI3ODMtNDRkMjFmNGE2ZTExIgogICBkYzpGb3JtYXQ9ImltYWdlL3BuZyIKICAgR0lNUDpBUEk9IjIuMCIKICAgR0lNUDpQbGF0Zm9ybT0iTGludXgiCiAgIEdJTVA6VGltZVN0YW1wPSIxNjMxNTcwMjAyMDcyNDg0IgogICBHSU1QOlZlcnNpb249IjIuMTAuMjYiCiAgIHRpZmY6T3JpZW50YXRpb249IjEiCiAgIHhtcDpDcmVhdG9yVG9vbD0iR0lNUCAyLjEwIj4KICAgPHhtcE1NOkhpc3Rvcnk+CiAgICA8cmRmOlNlcT4KICAgICA8cmRmOmxpCiAgICAgIHN0RXZ0OmFjdGlvbj0ic2F2ZWQiCiAgICAgIHN0RXZ0OmNoYW5nZWQ9Ii8iCiAgICAgIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6NTRiZmQ2ZGItOThkMy00MzY0LWFkNjktOGQyOWI4YWY1YjY2IgogICAgICBzdEV2dDpzb2Z0d2FyZUFnZW50PSJHaW1wIDIuMTAgKExpbnV4KSIKICAgICAgc3RFdnQ6d2hlbj0iMjAyMS0wOS0xNFQwOTo1Njo0MisxMjowMCIvPgogICAgPC9yZGY6U2VxPgogICA8L3htcE1NOkhpc3Rvcnk+CiAgPC9yZGY6RGVzY3JpcHRpb24+CiA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgCjw/eHBhY2tldCBlbmQ9InciPz6z20pTAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH5QkNFTgpOuffMQAAABl0RVh0Q29tbWVudABDcmVhdGVkIHdpdGggR0lNUFeBDhcAABypSURBVHja7Z17fFzVde+/a0aS3zbGNjjGtkayjU0MMYS34QMhQEDQpqH0ESA0N0pIonDTAH1wGy6kaUmBth8gCUWQ2+qmhFAIISk0iYAQG0gbjC0MmIsDfmhGtuMYG7CNH7Jsadb9Yx/VRp6zZ87MGUkzWt/Pxx+Mz57z2K/fXnuvvTYYhmEYhmEYhmEYhmEYhmEYhmEY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' alt='AUTOCULTURE' height="85em" />
    </center>
    </div>
  <div id='loggedout'>
  <p><a href="http://log:out@%IP%">Click here</a> to log out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
  </div>
  <div id='footer'>
    <center>
      <h6>Autoculture is distributed under GPLv3 by <a href="http://www.github.com/thgreenshaman">thgreenshaman</a> @ <a href='https://www.dm-tech.com'>-\>dm-tech</a><h6>
    </center>
  </div>
</body>
</html>
)rawliteral";
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
  } else if (var == "IP") {
    IPAddress ipAddress = WiFi.localIP();
    return String(String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]));
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

  //Comment out the block below to connect to a WiFi network
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  
  //WiFi.softAP(ap_ssid, ap_password);

  Serial.print("\nConnected on :");
  Serial.println(WiFi.localIP());

  // Set custom port for OTA here. Defaults to port 8266
  // ArduinoOTA.setPort(8266);

  ArduinoOTA.setHostname("autoculture");

  //Uncomment to use SPIFFS authentication.
  //ArduinoOTA.setPassword(ota_pass);


  //If you prefer to use a more secure MD5 hashed password use the next two lines...
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // Uncomment the next line to when upgrading SPIFFS to unmount SPIFFS. 
    //SPIFFS.end();
    
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if(!request->authenticate(http_user, http_pass))
      return request->requestAuthentication();
    request->send(SPIFFS, "/index.html", String(), false, processor);
    
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });

  server.on("/loggedout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
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
  ArduinoOTA.handle();
  readUno();
}

#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>

#define CLIENT
//#define SERVER

#ifdef SERVER

String getQuery();

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

int counter = 0;

#endif

#ifdef CLIENT
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

const char* serverNameDebug = "http://192.168.4.1/debug";
unsigned long previousMillis = 0;
const long interval = 5000;

String httpGETRequest(const char* serverName);

#endif

AsyncWebServer server(80);

void setup(){

Serial.begin(9600);
  #ifdef SERVER
  // Serial port for debugging purposes
  
  Serial.println();
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
  server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    counter++;
    request->send_P(200, "text/plain", getQuery().c_str());
  });
  /*
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumi().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPres().c_str());
  });
  */
  
  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)

  /*
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  */
#endif

#ifdef CLIENT

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
#endif

  
  // Start server
  server.begin();
}

void loop(){

#ifdef SERVER
#endif

#ifdef CLIENT

  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval) {
     // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED ){

      String variable = httpGETRequest(serverNameDebug);
      Serial.println(variable);//
      Serial.println("Ha bu bana gelen şeydir");

      previousMillis = currentMillis;
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }
#endif

}

#ifdef SERVER
String getQuery(){
  return String(counter);
}

#endif

#ifdef CLIENT

String httpGETRequest(const char* serverName){
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
#endif



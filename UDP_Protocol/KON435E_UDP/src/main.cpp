#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>
#include <DHT.h>

//#define CLIENT
#define SERVER

//SERVER -> REMOTE SENSOR
//CLIENT -> REMOTE OUTPUT

#ifdef SERVER

#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN,DHTTYPE);
void readSensor();
String getQuery();
String getTemp();
String getHum();
bool tempRelay;
bool humRelay;
unsigned temperature;
unsigned humidity;

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

int counter = 0;

#endif

#ifdef CLIENT

#define RELAY_1 25
#define RELAY_2 26
#define TEMP_LIMIT 27
#define HUMID_LIMIT 45

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

const char* serverNameTemp = "http://192.168.4.1/temp";
const char* serverNameHumid = "http://192.168.4.1/humid";
const char* svNameSetTemp = "http://192.168.4.1/setTemp";
const char* svNameResetTemp = "http://192.168.4.1/ressetTemp";
const char* svNameSetHum = "http://192.168.4.1/setHum";
const char* svNameResetHum = "http://192.168.4.1/resetHum";

unsigned long previousMillis = 0;
const long interval = 1000;

String httpGETRequest(const char* serverName);

String temp_str;
String humid_str;

unsigned temp;
unsigned humid;

#endif

AsyncWebServer server(80);

void setup(){

Serial.begin(9600);
  #ifdef SERVER
  dht.begin();
  
  Serial.println();
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
    counter++;
    readSensor();
    request->send_P(200, "text/plain", getTemp().c_str());
  });
  server.on("/humid", HTTP_GET, [](AsyncWebServerRequest *request){
    counter++;
    readSensor();
    request->send_P(200, "text/plain", getHum().c_str());
  });
  server.on("/setTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    tempRelay = 1;
    digitalWrite(RELAY_1, LOW);
    request->send_P(200, "text/plain", getHum().c_str());
  });
  server.on("/resetTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    tempRelay = 0;
    digitalWrite(RELAY_1, HIGH);
    request->send_P(200, "text/plain", getHum().c_str());
  });
  server.on("/setHum", HTTP_GET, [](AsyncWebServerRequest *request){
    humRelay = 1;
    digitalWrite(RELAY_2, LOW);
    request->send_P(200, "text/plain", getHum().c_str());
  });
  server.on("/resetHum", HTTP_GET, [](AsyncWebServerRequest *request){
    humRelay = 0;
    digitalWrite(RELAY_2, HIGH);
    request->send_P(200, "text/plain", getHum().c_str());
  });
  
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

  pinMode(RELAY_1,OUTPUT);
  pinMode(RELAY_2,OUTPUT);

  digitalWrite(RELAY_1,HIGH);
  digitalWrite(RELAY_2,HIGH);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(200);
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

      temp_str = httpGETRequest(serverNameTemp);
      Serial.print("Temperature: ");
      Serial.println(temp_str);

      temp = temp_str.toInt();

      if (temp >= TEMP_LIMIT){
        httpGETRequest(svNameSetTemp);
        Serial.println("Temp ON");
      }
      else {
        httpGETRequest(svNameResetTemp);
        Serial.println("Temp OFF");
      }

      humid_str = httpGETRequest(serverNameHumid);
      Serial.print("Humidity: ");
      Serial.println(humid_str);

      humid = humid_str.toInt();

      if (humid >= HUMID_LIMIT){
        httpGETRequest(svNameSetHum);
        Serial.println("Humid ON");
      }
      else {
        httpGETRequest(svNameResetHum);
        Serial.println("Humid OFF");
      }

      previousMillis = currentMillis;

    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }
#endif

}

#ifdef SERVER
void readSensor(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

}
String getQuery(){
  return String(counter);
}

String getTemp(){
  return String(temperature);
}

String getHum(){
  return String(humidity);
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
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
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
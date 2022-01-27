#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"




#define mqtt_server "192.168.43.55"
WiFiClient espClient;
PubSubClient client(espClient);
#define mqttTemp "sensor/temp"
#define mqttHum "sensor/hum"

#define SENSOR
//#define OUTPUT_ESP

long lastMsg = 0;



#define RELAY_1 25
#define RELAY_2 26
#define DHTTYPE DHT11
#define DHTPIN 4

const char* ssid = "AndroidAPBDA5";
const char* password = "awiw6324";
void setupWifi();
void reconnect();
void callback(char* topic, byte* message, unsigned int length);

#ifdef SENSOR

DHT dht(DHTPIN,DHTTYPE);

void readSensor();
unsigned temperature;
unsigned humidity;
int a;



#endif

#ifdef OUTPUT_ESP
#endif

void setup() {
  Serial.begin(115200);
  setupWifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

#ifdef SENSOR
  
  dht.begin();
  
#endif

#ifdef OUTPUT_ESP
  pinMode(RELAY_1,OUTPUT);
  pinMode(RELAY_2,OUTPUT);
#endif

delay(1000);
}

void controlRelays();

void loop() {

  if (client.connected()==0){
     reconnect();
  }
 


#ifdef SENSOR


client.loop();

long now = millis();
if (now - lastMsg > 5000){
  lastMsg = now;
}

readSensor();

client.publish(mqttTemp, String(temperature).c_str(),true);
client.publish(mqttHum, String(humidity).c_str(),true);


#endif

#ifdef OUTPUT_ESP

client.loop();
#endif
}

void setupWifi(){
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
    // Loop until we're reconnected
  int counter = 0;
  while (!client.connected()) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("remoteSensor")) {

      client.subscribe("output/relay1");
      client.subscribe("output/relay2");
      Serial.println("connected");
      
    }    
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    client.connect("remoteSensor");

    client.subscribe("output/relay1");
    client.subscribe("output/relay2");
  }
}

void callback(char* topic, byte* message, unsigned int length)
{

  String msg_tem;
  String msg_hum;
  if (strcmp(topic,"output/relay1")==0){
    for (int i = 0; i < length; i++){
      msg_tem += (char)message[i];
    }
    Serial.println(msg_tem);
    Serial.println(topic);


  }

  if (strcmp(topic,"output/relay2")==0){
    
    for (int i = 0; i < length; i++){
      msg_hum += (char)message[i];
    }
    Serial.println(msg_hum);
    
    Serial.println(topic);
  }
  digitalWrite(RELAY_1, msg_tem.toInt());
  digitalWrite(RELAY_2, msg_hum.toInt());
}

void controlRelays(){

}

#ifdef SENSOR
void readSensor(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  delay(1000);
}
#endif

#ifdef OUTPUT_ESP
#endif
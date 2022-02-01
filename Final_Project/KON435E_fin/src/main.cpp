#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"


#define mqtt_server "160.75.154.101"
const char * mqtt_password = "963258741";
const char * mqtt_username = "iturockwell";
WiFiClient espClient;
PubSubClient client(espClient);
#define mqttTemp "sensor/temp"
#define mqttHum "sensor/hum"
#define mqttTime "sensor/time"

#define SENSOR
//#define OUTPUT_ESP

long lastMsg = 0;


#define RELAY_1 25
#define RELAY_2 26
#define DHTTYPE DHT11
#define DHTPIN 4

const char* ssid = "ZyXEL_3YA3";
const char* password = "qwerty12345";
void setupWifi();
void reconnect();
void callback(char* topic, byte* message, unsigned int length);

#ifdef SENSOR

#define NODE_NAME "remoteSensor"

DHT dht(DHTPIN,DHTTYPE);

void readSensor();
unsigned temperature;
unsigned humidity;
int a;

#endif

#ifdef OUTPUT_ESP

#define NODE_NAME "remoteOutput"

#endif

void setup() {
  Serial.begin(115200);
  setupWifi();
  client.setServer(mqtt_server, 1884);
  

#ifdef SENSOR
  
  dht.begin();
  
#endif

#ifdef OUTPUT_ESP
  pinMode(RELAY_1,OUTPUT);
  pinMode(RELAY_2,OUTPUT);
  client.setCallback(callback);
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
    if (client.connect(NODE_NAME,mqtt_username,mqtt_password)) {

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
    client.connect(NODE_NAME);

#ifdef OUTPUT_ESP
    client.subscribe("output/relay1");
    client.subscribe("output/relay2");
#endif
  }
}



#ifdef SENSOR
void readSensor(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  delay(500);
  Serial.print("Temperature: ");
  Serial.println(String(temperature));
  Serial.print("Humidity: ");
  Serial.println(String(humidity));
}
#endif

#ifdef OUTPUT_ESP

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

#endif
#include <AsyncMqttClient.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

#include "arduino_secrets.h"

String ssid = WIFI_SSID;
String password = WIFI_PASSWORD;


//#define MQTT_HOST IPAddress(192, 168, 1, XXX)
// For a cloud MQTT broker, type the domain name
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883

//MQTT Topics
#define MQTT_PUB_TOPIC "soil/humidity/default"


int humidity;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.disconnect();
  //delay(1000);
  WiFi.begin(ssid, password);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

/*void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}*/

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

unsigned long previousMillis = 0;   // Stores last time temperature was published
long interval = 1000;        // Interval at which to publish sensor readings

#define AOUT_PIN A0 // Arduino pin that connects to AOUT pin of moisture sensor

String input = "";
bool things_to_set = false;
int delimiter;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println('\n');

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();  
}

void loop() {
  while (Serial.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial.available() >0) {
      char c = Serial.read();  //gets one byte from serial buffer
      input += c; //makes the string readString
    } 
    things_to_set = true;
  }
  if(things_to_set){
    if(input.substring(0,4) == "WiFi"){
      delimiter = input.indexOf(':');
      if (delimiter >= 0) {
        ssid = input.substring(4, delimiter); // Extract the substring
        password = input.substring(delimiter+1, input.length()-1);
        Serial.println("New settings:" + ssid + ":" + password);
        connectToWifi();
      } else {
        // Handle the case where ':' is not present in the input string
      }
    }
    things_to_set = false;
    input = "";
  }
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

    humidity = analogRead(AOUT_PIN);
    
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_TOPIC, 1, true, ("Humidity"+String(humidity)).c_str());

  }
}

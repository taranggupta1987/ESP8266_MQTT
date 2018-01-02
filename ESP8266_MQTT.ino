#include <ESP8266WiFi.h>
#include <MQTT.h>
#define AP_SSID     "SSID"
#define AP_PASSWORD "PASSWORD"
#define EIOTCLOUD_USERNAME "MQTT_CLOUD_USERNAME"
#define EIOTCLOUD_PASSWORD "MQTT_CLOUD_PASSWORD"

// create MQTT object

#define EIOT_CLOUD_ADDRESS "CLOUD_IP"
#define DO_TOPIC           "/topic"
#define PIN_DO_1           D0                  // DO pin1 
#define MODULE_ID_1        1
#define PORT               12345

MQTT myMqtt("RANDOM_KEY", EIOT_CLOUD_ADDRESS, PORT);
void setup() {
  Serial.begin(115200);

  pinMode(PIN_DO_1, OUTPUT);

  digitalWrite(PIN_DO_1, LOW);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  Serial.println();
  Serial.println();
  
  Serial.print("Connecting to ");
  Serial.println(AP_SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  };
  
  Serial.println("WiFi connected");
  
  Serial.println("Connecting to MQTT server");
  
  String clientName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  
  myMqtt.setClientId((char*) clientName.c_str());
  
  Serial.print("MQTT client id:");
  Serial.println(clientName);
  
  // setup callbacks
  myMqtt.onConnected(myConnectedCb);
  myMqtt.onDisconnected(myDisconnectedCb);
  
  //myMqtt.onPublished(myPublishedCb);
  myMqtt.onData(myDataCb);
  
  //Serial.println("connect mqtt...");
  myMqtt.setUserPwd(EIOTCLOUD_USERNAME, EIOTCLOUD_PASSWORD);
  myMqtt.connect();
  delay(500);

  subscribe();
  
  String value(-1);
  myMqtt.publish("status", value);
  
}
void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(myMqtt.isConnected()){
    delay(5000);
    myPublishedCb();
  }
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void subscribe()
{
  Serial.print("Subscribe to : ");
  Serial.print("/" + String(MODULE_ID_1) + DO_TOPIC);

  myMqtt.subscribe("/" + String(MODULE_ID_1) + DO_TOPIC); //DO 1
  myMqtt.subscribe("/" + String(MODULE_ID_1) + DO_TOPIC + "/getstatus"); //DO 1

}

void myConnectedCb() {
  Serial.println("connected to MQTT server");
  subscribe();
}

void myDisconnectedCb() {
  Serial.println("disconnected. try to reconnect...");
  delay(500);
  myMqtt.connect();
}

void myPublishedCb() {
  //Serial.println("published.");
  int value = digitalRead(PIN_DO_1);
  String valueStr(value);
  boolean rc = myMqtt.publish("status", valueStr);
  Serial.println("Published..");

}

void myDataCb(String& topic, String& data) {
  Serial.print("My Data : " + topic);
  if (topic == String("/" + String(MODULE_ID_1) + DO_TOPIC))
  {
    if (data == String("1"))
      digitalWrite(PIN_DO_1, HIGH);
    else
      digitalWrite(PIN_DO_1, LOW);

    myPublishedCb();
    
    Serial.print("Do 1:");
    Serial.println(data);
  }
  

  if (topic == String("/" + String(MODULE_ID_1) + DO_TOPIC + "/getstatus"))
  {
    myPublishedCb();
  }

}

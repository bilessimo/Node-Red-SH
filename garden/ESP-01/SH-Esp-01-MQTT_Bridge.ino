#include <Arduino.h> // Only needed if you're using VSCode.

/*
The ESP-01 connected a Arduino Nano (Rev. 3) to the WiFi Network and act like a bridge with the MQTT Server running on a Raspeberry Pi, working both ways.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid         = "SSID";// WiFi Network SSID.
const char* password     = "password";      // WiFi Network password.
const char* mqttServer   = "192.168.0.13";   // MQTT Server's IP Adress on WLAN (I'm running it on an fixed IP Raspberry Pi Model 3). 
const int   mqttPort     = 1883;            // Port used by the MQTT Server.

#define PUB_ARDUINO_STATUS "sh/garden/status"   // Topic on the MQTT Server used to receive the status message sent by the Arduino Nano.
#define SUB_ARDUINO_ACTION "sh/garden/action"   // Topic on the MQTT Server used to send messages to the Ardunino Nano.

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// This block is only necessary if you're using VSCode. If you're using the Arduino IDE, it does this for you.
void checkForMessage();
void initWifiStation();
void initMQTTClient();
void PubSubCallback(char* topic, byte* payload, unsigned int messageLength);

void setup() {
  
  Serial.begin(9600);
  initWifiStation();
  initMQTTClient();
  
}
  
void loop() {
  
  mqttClient.loop();
  checkForMessage();
  
}

void checkForMessage() {
  
  String incomingString="";

  while (Serial.available()){
	
	incomingString=Serial.readString();
	char stringConvertedToChar[256];  // Create a char array that will be used by mqttClient, since it don't accept a String.
	incomingString.toCharArray(stringConvertedToChar, 256); // Convert the String in the cahr array created above.
	mqttClient.publish(PUB_ARDUINO_STATUS, stringConvertedToChar);  // Publish the message.
	
  }
}

void initWifiStation() {

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);    

 Serial.print("\nConnecting to WiFi");  // Uncomment to debug to serial monitor
  
  while (WiFi.status() != WL_CONNECTED) {
	 delay(1000);        
    Serial.print(".");  // Uncomment to debug to serial monitor
  }
 Serial.println(String("\nConnected to the WiFi network (") + ssid + ")" );  // Uncomment to debug to serial monitor
}
 
void initMQTTClient() {

  // Connecting to MQTT server
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(PubSubCallback);
  
  while (!mqttClient.connected()) {
	
   Serial.println(String("Connecting to MQTT (") + mqttServer + ")..."); //Uncomment to debug to serial monitor
	if (mqttClient.connect("ESP8266Client")) {
     Serial.println("MQTT client connected");  // Uncomment to debug to serial monitor
	} else {
     Serial.print("\nFailed with state "); // Uncomment to debug to serial monitor
     Serial.println(mqttClient.state()); // Uncomment to debug to serial monitor
			
		if (WiFi.status() != WL_CONNECTED) {
		   initWifiStation();
		}
		delay(2000);
	}
  }

  // Declare Pub/Sub topics
  mqttClient.publish(PUB_ARDUINO_STATUS, "Initialized");
  mqttClient.subscribe(SUB_ARDUINO_ACTION);
}

void PubSubCallback(char* topic, byte* payload, unsigned int messageLength) {
  String strPayload = "";
  for (int i = 0; i < messageLength; i++) {
	  Serial.print((char)payload[i]);
	  strPayload += (char)payload[i];
  }
}

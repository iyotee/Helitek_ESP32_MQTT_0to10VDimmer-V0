///////////////////////////////////////////////////
//                                               //
//  CREATED BY  : Jérémy Noverraz                //
//  CREATED ON  : 06.02.2022                     //
//  VERSION     : 1.0.14                         //                                      
//  DESCRIPTION : 1CH 0-10V Dimmer over MQTT     //
//  LICENCE : GNU                                //
//                                               //
///////////////////////////////////////////////////

//Import libraries
#include <Arduino.h> //Arduino library
#include <WiFi.h> //WiFi library
#include <WiFiClient.h> //WiFiClient library
#include <PubSubClient.h> //PubSubClient library
#include <SPIFFS.h> //SPIFFS library
#include <analogwrite.h> //Analogwrite library
#include <ArduinoOTA.h> //ArduinoOTA library

//Define the pin for the dimmer
#define DIMMER_PIN 25

//Wifi network credentials and MQTT broker credentials
//Change the values for your network and broker
//You can find the network credentials and the broker credentials in the MQTT broker application

const char* ssid = "SwissLabsBox2"; //your network name
const char* password = "JP3YMhAdx4rbvyru3S"; //your network password

//MQTT configuration
#define MQTT_SERVER IPAddress(192,168,1,4) //IP address of the MQTT broker ex: 192,168,1,4
const int mqtt_port = 1883; //your MQTT port ex: 1883
const char* mqtt_user = "helitekmqttuser"; //your MQTT user (optional but may need to delete it in the code later) ex: "username"
const char* mqtt_password = "W3lc0m32h3l1t3k"; //your MQTT password (optional but may need to delete it in the code later) ex: "password"
const char* mqtt_client_id = "Helitek-LedDimmer-V0"; //your MQTT client id (must be unique) ex: "AC Dimmer"
const char* mqtt_commandtopic_channel1 = "helitek/dimmers/010v/channel1"; //your MQTT command topic for channel 1 ex: "helitek/dimmers/230/channel1"
const char* mqtt_statustopic_channel1 = "helitek/dimmers/010v/channel1/status"; //your MQTT status topic for channel 1 ex: "helitek/dimmers/230/channel1/status"
const char* mqtt_statetopic_channel1 = "helitek/dimmers/010v/channel1/state"; //your MQTT state topic for channel 1 ex: "helitek/dimmers/230/channel1/state"
const char* mqtt_commandbrightnesstopic_channel1 = "helitek/dimmers/010v/brightness"; //your MQTT command brightness topic for channel 1 ex: "helitek/dimmers/230/channel1/brightness" 
const char* mqtt_statusbrightnesstopic_channel1 = "helitek/dimmers/010v/brightness/status"; //your MQTT status brightness topic for channel 1 ex: "helitek/dimmers/230/channel1/brightness/status"
const char* mqtt_statebrightnesstopic_channel1 = "helitek/dimmers/010v/brightness/state"; //your MQTT state brightness topic for channel 1 ex: "helitek/dimmers/230/channel1/brightness/state"

//Wifi configuration
#define WIFI_HOSTNAME "Helitek-LedDimmer-V0" //your Wifi hostname ex: "AC Dimmer"

//OTA configuration
#define OTA_HOSTNAME "Helitek-LedDimmer-V0" //your OTA hostname ex: "AC Dimmer"

//MQTT variables
char msg[50]; //buffer for MQTT messages (must be big enough to hold the message) 

//Instanciate wifi client from WiFiClient class 
WiFiClient wclient; //WiFi client 

//Instanciate PubSubClient(client) from PubSubClient class from PubSubClient library with the wifi client as parameter 
PubSubClient client(wclient); // Setup MQTT client with wifi client

// OTA functions
void connectToOTA(){
  // Connect to OTA server
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPasswordHash("5f4dcc3b5aa765d61d8327deb882cf99");
  ArduinoOTA.setMdnsEnabled(true);
  ArduinoOTA.setPort(3232);
  ArduinoOTA.setTimeout(20000);
  ArduinoOTA.begin();
  Serial.println("OTA ready");
  // print a new line to the monitor
  Serial.println("\n");
}

//Connect to wifi network
void setup_wifi(){ //setup wifi function
  Serial.print("\nConnecting to "); //print to the serial monitor that we are connecting to wifi network 
  Serial.println(ssid); //print the network name to the serial monitor 

  //wifi mode
  WiFi.mode(WIFI_STA); //set the wifi mode to station mode (no access point) 

  //wifi begin
  WiFi.begin(ssid, password); //connect to the wifi network with the network name and password provided above
  WiFi.setHostname(WIFI_HOSTNAME); //set the hostname to the network name

  //wait for connection
  while (WiFi.status() != WL_CONNECTED) { //while the wifi connection is not established keep trying to connect
    delay(500); //delay for 500ms to allow the wifi to connect
    Serial.print("."); //print a dot to the serial monitor to show the connection is being established 
  }
  Serial.println(); //print a new line to the serial monitor to show the connection has been established 
  Serial.println("WiFi connected"); //print to the serial monitor that the wifi connection has been established
  Serial.println("Wifi IP address: "); //print to the serial monitor that the ip address is 
  Serial.print(WiFi.localIP()); //print the ip address to the serial monitor
  Serial.print("Wifi Hostname: "); //print to the serial monitor that the hostname is
  Serial.println(WiFi.getHostname()); //print the hostname to the serial monitor
  Serial.println("\n"); //print a new line to the serial monitor

}

// Reconnect to client
void reconnect(){ //reconnect function
  //Loop until we're reconnected
  while (!client.connected()){ //while the client is not connected to the MQTT broker, keep trying to connect 
    Serial.print("Attempting MQTT connection..."); //print to the serial monitor that the client is attempting to connect to the MQTT broker 
    //Attempt to connect
    if(client.connect(mqtt_client_id, mqtt_user, mqtt_password)){ //if the client is able to connect to the MQTT broker
      client.subscribe(mqtt_commandtopic_channel1); //subscribe to the command topic channel 1
      client.subscribe(mqtt_commandbrightnesstopic_channel1); //subscribe to the command brightness topic channel 1
      client.subscribe(mqtt_statustopic_channel1); //subscribe to the status topic channel 1
      client.subscribe(mqtt_statusbrightnesstopic_channel1); //subscribe to the status brightness topic channel 1
      client.subscribe(mqtt_statetopic_channel1); //subscribe to the state topic channel 1
      client.subscribe(mqtt_statebrightnesstopic_channel1); //subscribe to the state brightness topic channel 1
      Serial.println("connected"); //print to the serial monitor that the client has connected to the MQTT broker
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_commandtopic_channel1); //print the command topic to the serial monitor
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_commandbrightnesstopic_channel1); //print the command brightness topic to the serial monitor
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_statustopic_channel1); //print the status topic to the serial monitor
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_statusbrightnesstopic_channel1); //print the status brightness topic to the serial monitor
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_statetopic_channel1); //print the state topic to the serial monitor
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_statebrightnesstopic_channel1); //print the state brightness topic to the serial monitor
      Serial.print("\n"); //print a new line to the serial monitor
      Serial.println('\n'); //print a new line to the serial monitor 
    } else { //else if the client is not able to connect to the MQTT broker 
      Serial.print("failed, rc="); //print to the serial monitor that the client has failed to connect to the MQTT broker
      Serial.print(client.state()); //print the state of the client to the serial monitor
      Serial.println(" try again in 5 seconds"); //print to the serial monitor that the client will attempt to connect to the MQTT broker in 5 seconds 
      // Wait 5 seconds before retrying 
      delay(5000); //delay for 5 seconds 
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){ //callback function
  //Print the topic and payload
  Serial.print("Message arrived ["); //print to the serial monitor that a message has arrived
  Serial.print(topic); //print the topic to the serial monitor 
  Serial.print("] "); //print to the serial monitor
  //Store the value of the message in the msg variable
  for (int i = 0; i < length; i++) { //for each character in the payload
    msg[i] = (char)payload[i]; //store the character in the msg variable
  }
  // add the null 0 to the end of the string because strings can't be null terminated for some reason 
  msg[length] = '\0';
  //convert the msg in a String
  String message = String(msg); //convert the msg in a String
  // Print the message
  Serial.println(message); //print the message to the serial monitor
  // Check if the message is from the command topic channel1 or brightness topic channel1
  if(String(topic) == mqtt_commandtopic_channel1){ //if the message is from the command topic channel1
    // Check if the message is "OFF"
    if(message == "OFF"){ //else if the message is "OFF"
      analogWrite(DIMMER_PIN, 0, 255); //turn the dimmer off
      Serial.println("Turning off"); //print to the serial monitor that the dimmer is turning off
    }
  }
  // Check if the message is from the command brightness topic channel1 (do not forget to type it as a string)
  if(String(topic) == mqtt_commandbrightnesstopic_channel1){ //else if the message is from the command brightness topic channel1
    // Check if the message is a number between 0 and 255
    if(message.toInt() >= 0 && message.toInt() <= 255){ //if the message is a number between 0 and 255
      analogWrite(DIMMER_PIN, message.toInt(), 255); //set the brightness of the pin to the message
      Serial.print("Brightness set to: "); //print to the serial monitor that the brightness has been set to
      Serial.println(message); //print the message to the serial monitor
    }
  }
}

//Setup the dimmers and the MQTT client 
void setup() { //setup function
  Serial.begin(9600); //Start serial communication with baud rate of 9600 
  //delay 100ms to allow the serial monitor to start//
  delay(100); //delay for 100ms
  //setup wifi //
  setup_wifi(); //Connect to wifi network
  //setup MQTT //
  client.setServer(MQTT_SERVER, mqtt_port); //Set MQTT server and port
  client.setCallback(callback); //Set callback function for incomming messages from MQTT broker 
  //OTA
  connectToOTA(); //Connect to OTA
}

//Loop forever
void loop() { //loop function
  if (!client.connected()) { //Check if client is connected to MQTT broker, if not reconnect
    reconnect(); //then reconnect
  }
  client.loop(); //Check for incomming messages from MQTT broker
  //Handle OTA
  ArduinoOTA.handle(); //OTA handling
}
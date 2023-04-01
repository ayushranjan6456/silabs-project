#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* mqtt_server = "test.mosquitto.org";
// const int mqttPort = 8883;
// const char* mqttUser = "cedalo";
// const char* mqttPassword = "IekBTSRGLsPknTXS9AHH";


#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
char hr_str[MSG_BUFFER_SIZE];
char spo2_str[MSG_BUFFER_SIZE];
char temp_str[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ayushranjan6456/testing", "Connected");
      // ... and resubscribe
      client.subscribe("ayushranjan6456/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void onBeatDetected() {
    Serial.println("♥ Beat!");
}

int temppin= A0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  
  setup_wifi();

  delay(5000);
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();

  delay(5000);
  
  Serial.print("Initializing pulse oximeter..");
  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
  } else {
    Serial.println("SUCCESS");
  }

  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);

  //client.publish("ayushranjan6456/testing", "Hello from ESP8266");
  //client.subscribe("testing");  

}

void loop() {
  pox.update(); 
   
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello from Ayush's ESP #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);

    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();
    Serial.print("Heart rate:");
    Serial.print(hr);
    Serial.print("\nbpm / SpO2:");
    Serial.print(spo2);
    Serial.println("%");
    
    int analogValue = analogRead(temppin);
    float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
    float celsius = millivolts/10;
    float fahrenheit = ((celsius * 9)/5 + 32);
    Serial.print("\nTemp:   ");
    Serial.println(fahrenheit);
    

    snprintf (hr_str, MSG_BUFFER_SIZE, "%.2f", hr);
    snprintf (spo2_str, MSG_BUFFER_SIZE, "%f", spo2);
    snprintf (temp_str, MSG_BUFFER_SIZE, "%f", fahrenheit);
    client.publish("ayushranjan6456/testing", msg);
    client.publish("ayushranjan6456/heartrate", hr_str);
    client.publish("ayushranjan6456/spo2", spo2_str);
    client.publish("ayushranjan6456/temp", temp_str);
  }
}

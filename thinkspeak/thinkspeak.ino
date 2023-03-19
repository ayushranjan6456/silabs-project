//https://www.electroniclinic.com/
// Download Libraries: https://www.electroniclinic.com/arduino-libraries-download-and-projects-they-are-used-in-project-codes/

#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     20000

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
}


char ssid[] = "Galaxy F412740";   // your network SSID (name) 
char pass[] = "hcqe2971";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = 2052097;
const char * myWriteAPIKey = "FX2P6JW0QA91KHSM";

//int temp = random(0,50);
//int spo2 = random(80,100);
//int number3 = random(0,100);
//String myStatus = "";
 
void setup() {
  Serial.begin(115200);  // Initialize serial
  Serial.print("\nInitializing pulse oximeter..");

  // Initialize sensor
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } 
  else {
      Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
 
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}
 
void loop() {
 
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  pox.update();

    // Grab the updated heart rate and SpO2 levels
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        float hr = pox.getHeartRate();
        float spo2 = pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(hr);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");

        // set the fields with the values
        ThingSpeak.setField(1, hr);
        ThingSpeak.setField(2, spo2);
        
        // write to the ThingSpeak channel
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if(x == 200){
          Serial.println("Channel update successful.");
        }
        else{
          Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
        tsLastReport = millis();
    }
  // set the status
  //ThingSpeak.setStatus(myStatus);
}
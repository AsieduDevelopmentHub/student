#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"

#define SSID "Asare A05"
#define PASSWORD "Asare2016"
#define API_KEY "AIzaSyC0ScXmzo0-O2Vsxjpp2nTDmJkLETobEQg"
#define DB_URL "https://v0ai-real-default-rtdb.firebaseio.com/"

//Firebase Objects
FirebaseData data;
FirebaseAuth auth;
FirebaseConfig config; 
unsigned long SendDataPrevMillis = 0;
const char* ntpserver = "pool.ntp.org";
const long gmt = 0;
const int daylight = 0;
const int led = 8;
const float tariffRate = 1.5;

String getTimeString(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    return "NTP Server Failed";
  }
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

void setup(){
  Serial.begin(115200);
  delay(2000);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  //WiFi connection
  WiFi.setHostname("Refrigerator Monitoring System");
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi .");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nCoonected to WiFi.");
  
  //Initializing time 
  configTime(gmt, daylight, ntpserver);

  //Firebase Configuration Setup
  config.api_key = API_KEY;
  config.database_url = DB_URL;

  //Anonymous sign-Up
  if(Firebase.signUp(&config, &auth, "fridgemonitoring@asiedudevelopmenthub.com", "fridgemonitoring")){
    Serial.println("FIrebase sign up succesful");
  } else{
    Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  //Firebase begin
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  //Send data every 5 seconds
  if(Firebase.ready() && (millis() - SendDataPrevMillis > 5000 || SendDataPrevMillis)){
    SendDataPrevMillis = millis();

    //Random Sensor values
    float temperatureSensor = random(20, 160) / 10.0;
    float VoltageSensor = random(2204, 2312) / 10.0;
    float CurrentSensor = random(40, 198) / 10.0;
    float Power = VoltageSensor * CurrentSensor;
    float EnergyKwh = Power / 3600000; //Kilowatt per second
    TotalEnergy += EnergyKwh;
    float CurrentCost = TotalEnergy * tariffRate;
    String timestamp = getTimeString();

    //Printing details to Serial Monitor
    Serial.printf("Temperature: %.2f °C | Voltage: %.1f V | Current: %.2f A | Time: %s\n", temperatureSensor, VoltageSensor, CurrentSensor, timestamp.c_str());

    //Sending information to real-time db
    Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Temperature", temperatureSensor);
    Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Voltage", VoltageSensor);
    Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Current", CurrentSensor);
    Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Wattage", Power);
    Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Energy", TotalEnergy);
    Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Cost", CurrentCost);
    Firebase.RTDB.setString(&data, "Fridge/Time/LastUpdate", timestamp);
  }
  if(Firebase.RTDB.getInt(&data, "Fridge/Controls/Relay")){
      int ledState = data.intData();
      digitalWrite(led, ledState ? LOW : HIGH);
      Serial.println("LED Status: " + String(ledState));
    }
  
}
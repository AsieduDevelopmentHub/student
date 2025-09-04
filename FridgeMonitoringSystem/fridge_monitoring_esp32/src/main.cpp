#include <Arduino.h>
#include <ZMPT101B.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_SSD1306.h>
#include <ACS712.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ---------------------- USER DEFINITIONS ----------------------
#define SENSITIVITY 490.0f
#define tempsensorPin 2
#define sensitive 100              
#define SSID "Your WiFi name"
#define PASSWORD "Your WiFi password"
#define API_KEY "Firebase API Key"
#define DB_URL "Firebase Real Time database url"

// ---------------------- SENSOR OBJECTS ----------------------
ZMPT101B voltageSensor(1, 50.0);
OneWire pinout(tempsensorPin);
DallasTemperature temp(&pinout);
Adafruit_SSD1306 oled(128, 64, &Wire);
ACS712 sensor(0, 3.3, 4095, sensitive);

// ---------------------- FIREBASE OBJECTS ----------------------
FirebaseData data;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// ---------------------- GLOBALS ----------------------
unsigned long SendDataPrevMillis = 0;
unsigned long SendHistoryPrevMillis = 0;
unsigned long OledPrevMillis = 0;
unsigned long RelayPrevMillis = 0;

float tariffRate = 1.824; // Temporal value 
float TotalEnergy = 0.0;  
float TotalCost = 0.0;
int page = 0;
int ledstate = 0;
String status = "OFF";

const long intervalOled = 1000;
const char* ntpserver = "pool.ntp.org";
const long gmt = 0;
const int daylight = 0;

//relay and led status 
const int led = 4; 

bool wifiConnected = false; // NEW

// ---------------------- FUNCTIONS DECLARATION ----------------------
void initialization();
void onload();
void oledInfo(float v, float t, float a, float e, float c, float p, String status);
void firebaseSend(float v, float t, float a, float e, float c, float p, String timestamp);

String getTimeString(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "NTP Failed"; // Still works offline
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

// ---------------------- SETUP ----------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  temp.begin();
  voltageSensor.setSensitivity(SENSITIVITY);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(led, OUTPUT);

  oled.clearDisplay();
  oled.display();
  onload();
  initialization();
  Serial.println("___ATU Fridge Monitoring System___");
  Serial.println("   _____System Initiated_____");

  if (wifiConnected) { // NEW → Only fetch from Firebase if WiFi is ok
    if (Firebase.RTDB.getFloat(&data, "Fridge/Consumption/TariffRate")) {
      tariffRate = data.floatData();
    }
    if (Firebase.RTDB.getFloat(&data, "Fridge/Consumption/Energy")) {
      TotalEnergy = data.floatData();
    }
    if (Firebase.RTDB.getFloat(&data, "Fridge/Consumption/Cost")) {
      TotalCost = data.floatData();
    }
  }

  Serial.println("Calibrating ACS712, make sure no load is connected...");
  sensor.autoMidPoint();  // calibrate midpoint automatically
  Serial.print("MidPoint = ");
  Serial.println(sensor.getMidPoint());

  digitalWrite(led, HIGH);   // Start with Relay ON
}

// ---------------------- LOOP ----------------------
void loop() {
  temp.requestTemperatures();
  float tempCelcius = temp.getTempCByIndex(0);
  float voltage = voltageSensor.getRmsVoltage();

  int mA = sensor.mA_AC(200);      
  float CurrentSensor = mA  / 1000.0; 

  if (voltage <= 10) { voltage = 0; CurrentSensor = 0; } 
  if (CurrentSensor <= 0.1) CurrentSensor = 0;

  float Power = voltage * CurrentSensor; 
  float EnergyKwh = (Power / 1000.0) * (1.0 / 3600.0); 
  TotalEnergy += EnergyKwh;
  TotalCost += EnergyKwh * tariffRate;
  String timestamp = getTimeString();

  Serial.printf("Temperature: %.2f °C | Voltage: %.1f V | Current: %.2f A\n", tempCelcius, voltage, CurrentSensor);
  Serial.printf("Power: %.1f W | Energy: %.2f kWh | Cost: %.3f GHS\n", Power, TotalEnergy, TotalCost);
  Serial.printf("Time: %s\n", timestamp.c_str());

  // OLED always updates
  oledInfo(voltage, tempCelcius, CurrentSensor, TotalEnergy, TotalCost, Power, status);

  // Firebase only runs if WiFi is connected
  if (wifiConnected) {
    firebaseSend(voltage, tempCelcius, CurrentSensor, TotalEnergy, TotalCost, Power, timestamp);
  }
}

// ---------------------- INITIALIZATION ----------------------
void initialization(){
  WiFi.setHostname("Refrigerator Monitoring System");
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting to WiFi ");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 1000){
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nConnected to WiFi.");
    configTime(gmt, daylight, ntpserver);

    config.api_key = API_KEY;
    config.database_url = DB_URL;
    auth.user.email = "fridgemonitoring@asiedudevelopmenthub.com";
    auth.user.password = "fridgemonitoring";

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase ready.");
  } else {
    wifiConnected = false; // NEW
    Serial.println("\nWiFi not connected. Running in offline mode!");
  }
}


// ---------------------- FIREBASE ----------------------
void firebaseSend(float v, float t, float a, float e, float c, float p, String timestamp){
  if(Firebase.ready()){
    // Realtime updates every 2 sec
    if(millis() - SendDataPrevMillis > 2000){
      Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Temperature", t);
      Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Voltage", v);
      Firebase.RTDB.setFloat(&data, "Fridge/Sensors/Current", a);
      Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Wattage", p);
      Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Energy", e);
      Firebase.RTDB.setFloat(&data, "Fridge/Consumption/Cost", c);
      Firebase.RTDB.setString(&data, "Fridge/Time/LastUpdate", timestamp); 
      SendDataPrevMillis = millis();
      Serial.println("Data sent Successfully");
    }

    // History logging every 10 mins
    if(millis() - SendHistoryPrevMillis >= 600000){
      int spaceIndex = timestamp.indexOf(' ');
      String Date = timestamp.substring(0, spaceIndex);
      String Time = timestamp.substring(spaceIndex + 1);
      String path = String("Fridge/History/") + Date + "/" + Time;
      
      json.set("Temperature", t);
      json.set("Voltage", v);
      json.set("Current", a);
      json.set("Wattage", p);
      json.set("Energy", e);
      json.set("Cost", c);

      Firebase.RTDB.setJSON(&data, path.c_str(), &json);
      SendHistoryPrevMillis = millis();
      Serial.println("History logged Successfully");
    }

    // Relay control every 1s
    if (millis() - RelayPrevMillis > 1000) {
      RelayPrevMillis = millis();

      if (Firebase.RTDB.getInt(&data, "Fridge/Controls/Relay")) {
        int relayCommand = data.intData();

    // Default ON
        int ledState = HIGH;
        status = "OFF";

        if (relayCommand == 1) {
            // Only allow ON if voltage is safe
            if (v >= 180 && v <= 260) {
                digitalWrite(led, HIGH);
                ledState = HIGH;
                status = "ON";
            } else {
                // Voltage unsafe, keep it OFF
                digitalWrite(led, LOW);
                ledState = LOW;
                status = "OFF";
            }
        } else {
            // Firebase says OFF
            digitalWrite(led, LOW);
            ledState = LOW;
            status = "OFF";
        }

        // Report actual state back to Firebase
        int ledStatus = digitalRead(led);
        Firebase.RTDB.setInt(&data, "Fridge/Controls/Status", ledStatus);
      }
    } 
  }
}

// ---------------------- OLED ----------------------
void onload(){
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(5, 5);
  oled.println("ATU Fridge");
  oled.print("\nMonitoring");
  oled.display();
  delay(3000);
  oled.clearDisplay(); 
  for (int i = 0; i < min(128, 64) / 2; i += 2){ 
    oled.drawCircle(128/2, 64/2, i, SSD1306_WHITE);
    oled.display(); 
    delay(100); 
  } 
  oled.fillRect(5, 28, 30, 5, SSD1306_WHITE); 
  oled.fillRect(93, 28, 30, 5, SSD1306_WHITE); 
  oled.setTextSize(2); oled.setCursor(15, 5); 
  oled.println("V"); oled.setCursor(105, 5); 
  oled.println("A"); oled.setCursor(12, 49); 
  oled.print((char)247); oled.println("C"); 
  oled.setCursor(92, 49); oled.println("GHS"); 
  oled.setCursor(124/2 - 15, 64/2 - 8); 
  oled.setTextColor(SSD1306_BLACK); 
  oled.println("ATU"); 
  oled.display();
}

void oledInfo(float v, float t, float a, float e, float c, float p, String status){
  if (millis() - OledPrevMillis >= intervalOled) {
    OledPrevMillis = millis();
    page++;
    if (page > 2) page = 0;
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);

    if (page == 1){
      oled.drawLine(0, 33, 128, 33, SSD1306_WHITE);
      oled.drawLine(60, 0, 60, 32, SSD1306_WHITE);

      oled.setTextSize(1);
      oled.setCursor(5, 1); oled.println("Voltage: ");
      oled.setCursor(5, 16); oled.setTextSize(2); oled.print(v, 0); oled.println("V");

      oled.setTextSize(1);
      oled.setCursor(75, 1); oled.println("Current: ");
      oled.setCursor(68, 16); oled.setTextSize(2); oled.print(a, 2); oled.println("A");

      oled.setCursor(5, 40); oled.println("System " + status);

    } else if (page == 2){
      bool invalid = false;
      if (t <= -126) {
        invalid = true;
      }

      oled.drawLine(0, 64, 128, 0, SSD1306_WHITE);
      oled.setTextSize(1);
      oled.setCursor(5, 1); oled.println("Temperature: ");
      oled.setCursor(5, 16); oled.setTextSize(2);
      if (invalid) {
        oled.println("N/A");
      } else {
        oled.print(t, 1); 
        oled.print((char)247); 
        oled.println("C");
      }
      
      oled.setTextSize(1);
      oled.setCursor(80, 30); oled.println("Cost: ");
      oled.setCursor(45, 50); oled.setTextSize(2); oled.print(c, 2); oled.println("GHS");

    } else if (page == 0){
      oled.drawLine(0, 32, 128, 32, SSD1306_WHITE);
      oled.setTextSize(1);
      oled.setCursor(5, 10); oled.println("Power: ");
      oled.setCursor(50, 5); oled.setTextSize(2); oled.print(p, 0); oled.println("W");

      oled.setTextSize(1);
      oled.setCursor(40, 34); oled.println("Energy: ");
      oled.setCursor(15, 48); oled.setTextSize(2); oled.print(e, 2); oled.println("kWh");
    }

    oled.display();
  }
}
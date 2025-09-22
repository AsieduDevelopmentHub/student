#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <MQ135.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "Asare A05"
#define WIFI_PASSWORD "Asare2016"

// Firebase project details
#define API_KEY "AIzaSyC0ScXmzo0-O2Vsxjpp2nTDmJkLETobEQg"
#define DATABASE_URL "https://v0ai-real-default-rtdb.firebaseio.com/"

// Geolocation API
#define GEOLOCATION_API "http://ipapi.co/json/"

// Pin definitions - CORRECTED for ESP32-C3 (SDA =8 and SCL=9 on my board)
#define MQ135_PIN 1        // GPIO0 (Analog capable)
#define GREEN_LED 2        // GPIO4
#define YELLOW_LED 3      // GPIO5  
#define RED_LED 4          // GPIO6
#define DHTPIN 5           // GPIO7
#define DHTTYPE DHT22      // DHT 22

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Sensor objects
MQ135 mq135_sensor(MQ135_PIN);
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Global variables
float temperature = 0.0;
float humidity = 0.0;
float rzero;
int air_quality;
int vibration_level;
float tilt_angle;
String location = "Accra, Ghana";
String timestamp;
unsigned long sendDataPrevMillis = 0;
unsigned long lastDisplayChange = 0;
int displayMode = 0;
bool signupOK = false;
bool locationFetched = false;

// Error handling variables
int sensorErrorCount = 0;
const int MAX_ERRORS = 50; // Increased threshold since we're not restarting
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 30000;

// Alert thresholds
const int AIR_QUALITY_WARNING = 550;
const int AIR_QUALITY_CRITICAL = 650;
const int VIBRATION_WARNING = 500;
const int VIBRATION_CRITICAL = 800;
const float TILT_WARNING = 15.0;
const float TILT_CRITICAL = 30.0;
const float TEMP_WARNING = 35.0;
const float TEMP_CRITICAL = 40.0;
const float HUMIDITY_WARNING = 85.0;
const float HUMIDITY_CRITICAL = 90.0;

// Function prototypes
bool connectToWiFi();
String getTimeString();
bool initializeFirebase();
bool initializeMPU6050();
void readSensors();
void updateDisplay();
void updateLEDs();
void sendToFirebase();
void checkSystemHealth();
void attemptReconnect();
// void getLocationFromAPI();

void setup(){
  Serial.begin(115200);
  delay(1000); // Wait for serial to initialize
  
  Serial.println("Starting ESP32-C3 Machine Monitor");
  
  // Initialize pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  // Initialize LEDs to off
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Initialize I2C devices
  Wire.begin();
  // Initialize DHT sensor
  dht.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("__ATU Machine__");
  lcd.setCursor(0, 1);
  lcd.print("___Monitoring___");

  // Initialize MPU6050
  if (!initializeMPU6050()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MPU6050 Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");
    delay(3000);
  }

  // Connect to WiFi
  if (!connectToWiFi()) {
    lcd.clear();
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Retrying...");
    delay(2000);
  }

  // Initialize NTP client
  timeClient.begin();
  timeClient.setTimeOffset(0);
  
  // // Get location from API
  // if (WiFi.status() == WL_CONNECTED) {
  //   getLocationFromAPI();
  // }

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "fridgemonitoring@asiedudevelopmenthub.com";
  auth.user.password = "fridgemonitoring";
  
  // Initialize Firebase
  if (!initializeFirebase()) {
    lcd.clear();
    lcd.print("Firebase Error!");
    lcd.setCursor(0, 1);
    lcd.print("Continuing...");
    delay(2000);
  }
  // Calibrate MQ135
  lcd.clear();
  lcd.print("  Calibrating");
  lcd.setCursor(0, 1);
  lcd.print("    Sensors    ");
  rzero = mq135_sensor.getRZero();
  delay(3000);
  
  lcd.clear();
  lcd.print("  System Ready");
  Serial.println("System initialization complete");
  delay(2000);
}

// Gyroscope initialization
bool initializeMPU6050() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    return false;
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU6050 initialized successfully");
  return true;
}

//WiFi connection
bool connectToWiFi() {
  lcd.clear();
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  
  WiFi.setHostname("Machine & Environment Monitoring");
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Connected");
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    delay(2000);
    return true;
  } else {
    Serial.println("\nWiFi connection failed");
    return false;
  }
}

// Timestamp (Date + Time)
String getTimeString(){
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeInfo = localtime(&rawTime);
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
  return String(buffer);
}

bool initializeFirebase() {
  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println("Firebase initialized");
  signupOK = true;
  return signupOK;
}

void loop(){
  // Update NTP time
  timeClient.update();
  timestamp = getTimeString();
  Serial.print("Current Time: ");
  Serial.println(timestamp);

    // Read all sensors
  readSensors();
  
  // Update display
  updateDisplay();
  
  // Update LED indicators
  updateLEDs();
  
  // Check system health
  checkSystemHealth();
  
  // Reconnect to WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    attemptReconnect();
  }

  // Send data to Firebase (every 5 seconds)
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    sendToFirebase();
  }
  
  // Add a small delay
  delay(100);
}

// void getLocationFromAPI() {
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi not connected, cannot get location");
//     return;
//   }
  
//   lcd.clear();
//   lcd.print("Getting");
//   lcd.setCursor(0, 1);
//   lcd.print("location...");
  
//   HTTPClient http;
//   http.begin(GEOLOCATION_API);
//   int httpCode = http.GET();
  
//   if (httpCode == HTTP_CODE_OK) {
//     String payload = http.getString();
//     DynamicJsonDocument doc(1024);
//     deserializeJson(doc, payload);
    
//     String city = doc["city"].as<String>();
//     String country = doc["country_name"].as<String>();
    
//     if (city != "" && country != "") {
//       location = city + ", " + country;
//       locationFetched = true;
//       Serial.println("Location: " + location);
//       lcd.clear();
//       lcd.print("Location:");
//       lcd.setCursor(0, 1);
//       lcd.print(location);
//       delay(2000);
//     } else {
//       Serial.println("Failed to parse location from API response");
//       sensorErrorCount++;
//     }
//   } else {
//     Serial.printf("HTTP error code: %d\n", httpCode);
//     sensorErrorCount++;
//   }
  
//   http.end();
  
//   // If location fetch failed, try again later
//   if (!locationFetched) {
//     Serial.println("Location fetch failed, will try again later");
//   }
// }

void checkSystemHealth() {
  if (sensorErrorCount > MAX_ERRORS) {
    Serial.println("Too many sensor errors detected");
    lcd.clear();
    lcd.print("System Error");
    lcd.setCursor(0, 1);
    lcd.print("Check Sensors");
    // NO RESTART - Just display error message
    delay(3000);
    // Reset error count to allow recovery
    sensorErrorCount = 0;
  }
}

void attemptReconnect() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastReconnectAttempt > RECONNECT_INTERVAL) {
    lastReconnectAttempt = currentMillis;
    
    lcd.clear();
    lcd.print("Reconnecting");
    lcd.setCursor(0, 1);
    lcd.print("to WiFi...");
    
    if (connectToWiFi()) {
      lcd.clear();
      lcd.print("WiFi Reconnected");
      delay(2000);
      
      // Reinitialize Firebase after WiFi reconnect
      if (!initializeFirebase()) {
        Serial.println("Firebase reinitialization failed");
      }
    }
  }
}

void readSensors() {
  // Read MQ135
  air_quality = analogRead(MQ135_PIN) / 4;
  if (air_quality < 0 || air_quality > 4095) {
    Serial.println("MQ135 reading out of range");
    sensorErrorCount++;
  }
  
  // Read vibration sensor
  vibration_level = random(400, 530);
  
  // Read DHT22 (temperature and humidity)
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  // Check if any reads failed and keep old values if they did
  if (!isnan(newTemperature)) {
    temperature = newTemperature;
  } else {
    Serial.println("Failed to read temperature from DHT sensor!");
    sensorErrorCount++;
  }
  
  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  } else {
    Serial.println("Failed to read humidity from DHT sensor!");
    sensorErrorCount++;
  }
  
  // Read MPU6050
  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    // Calculate tilt angle (simplified)
    tilt_angle = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;
    if (tilt_angle < 0) tilt_angle = -tilt_angle;
  } else {
    Serial.println("Failed to read from MPU6050!");
    sensorErrorCount++;
  }
}

void updateDisplay() {
  // Change display mode every 3 seconds
  if (millis() - lastDisplayChange > 3000) {
    displayMode = (displayMode + 1) % 3;
    lastDisplayChange = millis();
  }
  
  lcd.clear();
  
  switch (displayMode) {
    case 0:
      // First line: Air quality and vibration
      lcd.setCursor(0, 0);
      lcd.print("AQ:");
      lcd.print(air_quality);
      lcd.print("   V:");
      lcd.print(vibration_level);
      
      // Second line: Status indicator
      lcd.setCursor(0, 1);
      if (air_quality > AIR_QUALITY_CRITICAL || vibration_level > VIBRATION_CRITICAL || 
          tilt_angle > TILT_CRITICAL || temperature > TEMP_CRITICAL || humidity > HUMIDITY_CRITICAL) {
        lcd.print("CRITICAL!");
      } else if (air_quality > AIR_QUALITY_WARNING || vibration_level > VIBRATION_WARNING || 
                 tilt_angle > TILT_WARNING || temperature > TEMP_WARNING || humidity > HUMIDITY_WARNING) {
        lcd.print("WARNING!");
      } else {
        lcd.print("NORMAL");
      }
      break;
      
    case 1:
      // First line: Tilt angle
      lcd.setCursor(0, 0);
      lcd.print("Tilt:");
      lcd.print(tilt_angle, 1);
      lcd.print("deg");
      
      // Second line: Location abbreviation
      lcd.setCursor(0, 1);
      if (location.length() > 12) {
        lcd.print(location.substring(0, 12));
      } else {
        lcd.print(location);
      }
      break;
      
    case 2:
      // First line: Temperature
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temperature, 1);
      lcd.print("C");
      
      // Second line: Humidity
      lcd.setCursor(0, 1);
      lcd.print("Hum:");
      lcd.print(humidity, 1);
      lcd.print("%");
      break;
  }
}

void updateLEDs() {
  // Turn off all LEDs first
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  
  // Check for critical conditions
  if (air_quality > AIR_QUALITY_CRITICAL || 
      vibration_level > VIBRATION_CRITICAL || 
      tilt_angle > TILT_CRITICAL ||
      temperature > TEMP_CRITICAL ||
      humidity > HUMIDITY_CRITICAL) {
    digitalWrite(RED_LED, HIGH);
    return;
  }
  
  // Check for warning conditions
  if (air_quality > AIR_QUALITY_WARNING || 
      vibration_level > VIBRATION_WARNING || 
      tilt_angle > TILT_WARNING ||
      temperature > TEMP_WARNING ||
      humidity > HUMIDITY_WARNING) {
    digitalWrite(YELLOW_LED, HIGH);
    return;
  }
  
  // If no warnings or critical issues, show green
  digitalWrite(GREEN_LED, HIGH);
}

void sendToFirebase() {
  // If location hasn't been fetched yet, try again
  // if (!locationFetched && WiFi.status() == WL_CONNECTED) {
  //   getLocationFromAPI();
  // }
  
  // Create a JSON object for the sensor data
  FirebaseJson json;
  
  json.set("air_quality", air_quality);
  json.set("vibration", vibration_level);
  json.set("tilt_angle", tilt_angle);
  json.set("temperature", temperature);
  json.set("humidity", humidity);
  json.set("location", location.c_str());
  
  // Send data to Firebase with a timestamp-based path
  int spaceIndex = timestamp.indexOf(' ');
  String Date = timestamp.substring(0, spaceIndex);
  String Time = timestamp.substring(spaceIndex + 1);
  String path = "Machinery/sensor_data/" + Date + "/" + Time;
  
  Serial.println("Sending data to Firebase...");
  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    Serial.println("Data sent successfully");
    
    // Also update the latest reading path
    Firebase.RTDB.setJSON(&fbdo, "Machinery/latest_reading", &json);
    
    // Reset error count on successful transmission
    sensorErrorCount = 0;
  } else {
    Serial.println("Failed to send data");
    Serial.println("Reason: " + fbdo.errorReason());
    sensorErrorCount++;
  }
}
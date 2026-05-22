#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"
#include <esp_task_wdt.h>

/*
====================================================
 Refrigerator Monitoring System
 Developed by: Asiedu Minta Kwaku

 Features:
 - WiFi Auto Reconnection
 - Firebase Realtime Database Integration
 - NTP Time Synchronization
 - Watchdog Auto Recovery
 - Simulated Sensor Monitoring
 - Real-Time Energy Consumption Tracking
====================================================
*/

// ---------------- WIFI CONFIG ----------------
#define WIFI_SSID       "Asare A05"
#define WIFI_PASSWORD   "Asare2016"

// ---------------- FIREBASE CONFIG ----------------
#define API_KEY         "AIzaSyC0ScXmzo0-O2Vsxjpp2nTDmJkLETobEQg"
#define DATABASE_URL    "https://v0ai-real-default-rtdb.firebaseio.com/"

// ---------------- HARDWARE ----------------
#define STATUS_LED      8

// ---------------- WATCHDOG ----------------
#define WDT_TIMEOUT     15   // seconds

// ---------------- POWER CONFIG ----------------
const float tariffRate = 1.5;

// ---------------- NTP ----------------
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

// ---------------- FIREBASE OBJECTS ----------------
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------------- TIMERS ----------------
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;

// ---------------- ENERGY VARIABLES ----------------
float totalEnergy = 0.0;

// ====================================================
// GET CURRENT TIMESTAMP
// ====================================================
String getTimeString()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        return "Time Sync Failed";
    }

    char buffer[30];

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

    return String(buffer);
}

// ====================================================
// CONNECT TO WIFI
// ====================================================
void connectWiFi()
{
    Serial.println("\nConnecting to WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.setHostname("Fridge-Monitor-System");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        esp_task_wdt_reset();

        Serial.print(".");

        delay(500);

        // Restart if WiFi takes too long
        if (millis() - startAttempt > 20000)
        {
            Serial.println("\nWiFi Connection Timeout. Restarting...");
            ESP.restart();
        }
    }

    Serial.println("\nWiFi Connected Successfully");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// ====================================================
// INITIALIZE FIREBASE
// ====================================================
void initFirebase()
{
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    Serial.println("\nInitializing Firebase...");

    if (Firebase.signUp(
            &config,
            &auth,
            "fridgemonitoring@asiedudevelopmenthub.com",
            "fridgemonitoring"))
    {
        Serial.println("Firebase Authentication Successful");
    }
    else
    {
        Serial.printf(
            "Firebase SignUp Failed: %s\n",
            config.signer.signupError.message.c_str());
    }

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    Serial.println("Firebase Initialized");
}

// ====================================================
// INITIALIZE TIME
// ====================================================
void initTime()
{
    Serial.println("\nSynchronizing Time...");

    configTime(
        gmtOffset_sec,
        daylightOffset_sec,
        ntpServer);

    String currentTime = getTimeString();

    Serial.println("Current Time: " + currentTime);
}

// ====================================================
// SEND SENSOR DATA
// ====================================================
void sendSensorData()
{
    // Simulated Sensor Values
    float temperature = random(20, 160) / 10.0;
    float voltage = random(2204, 2312) / 10.0;
    float current = random(40, 198) / 10.0;

    // Power Calculations
    float power = voltage * current;

    float energyKwh = power / 3600000.0;

    totalEnergy += energyKwh;

    float currentCost = totalEnergy * tariffRate;

    String timestamp = getTimeString();

    // Serial Monitor Output
    Serial.println("\n========== SENSOR REPORT ==========");

    Serial.printf("Temperature : %.2f °C\n", temperature);
    Serial.printf("Voltage     : %.1f V\n", voltage);
    Serial.printf("Current     : %.2f A\n", current);
    Serial.printf("Power       : %.2f W\n", power);
    Serial.printf("Energy      : %.6f kWh\n", totalEnergy);
    Serial.printf("Cost        : %.4f\n", currentCost);
    Serial.printf("Timestamp   : %s\n", timestamp.c_str());

    Serial.println("===================================");

    // Upload Data to Firebase
    bool success = true;

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Sensors/Temperature",
        temperature);

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Sensors/Voltage",
        voltage);

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Sensors/Current",
        current);

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Consumption/Wattage",
        power);

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Consumption/Energy",
        totalEnergy);

    success &= Firebase.RTDB.setFloat(
        &fbdo,
        "Fridge/Consumption/Cost",
        currentCost);

    success &= Firebase.RTDB.setString(
        &fbdo,
        "Fridge/Time/LastUpdate",
        timestamp);

    if (success)
    {
        Serial.println("Firebase Upload Successful");
    }
    else
    {
        Serial.println("Firebase Upload Failed");
        Serial.println(fbdo.errorReason());
    }
}

// ====================================================
// HANDLE REMOTE CONTROL
// ====================================================
void handleRelayControl()
{
    if (Firebase.RTDB.getInt(
            &fbdo,
            "Fridge/Controls/Relay"))
    {
        int relayState = fbdo.intData();

        digitalWrite(
            STATUS_LED,
            relayState ? HIGH : LOW);

        Serial.print("Relay State: ");
        Serial.println(relayState ? "ON" : "OFF");
    }
}

// ====================================================
// CHECK WIFI CONNECTION
// ====================================================
void checkWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nWiFi Disconnected");

        connectWiFi();
    }
}

// ====================================================
// SETUP
// ====================================================
void setup()
{
    Serial.begin(115200);

    delay(2000);

    pinMode(STATUS_LED, OUTPUT);

    digitalWrite(STATUS_LED, LOW);

    // Initialize Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true);

    esp_task_wdt_add(NULL);

    Serial.println("\n=================================");
    Serial.println(" Refrigerator Monitoring System");
    Serial.println(" Initializing...");
    Serial.println("=================================");

    connectWiFi();

    initTime();

    initFirebase();

    Serial.println("\nSystem Ready");
}

// ====================================================
// MAIN LOOP
// ====================================================
void loop()
{
    // Feed Watchdog
    esp_task_wdt_reset();

    // Check WiFi Status
    checkWiFi();

    // Send Data Periodically
    if (Firebase.ready() &&
        millis() - lastSendTime >= sendInterval)
    {
        lastSendTime = millis();

        sendSensorData();

        handleRelayControl();
    }

    delay(100);
}
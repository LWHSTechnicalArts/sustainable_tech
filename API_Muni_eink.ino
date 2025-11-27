/*
 * ESP32-S3 Feather Muni Tracker with 2.13" E-ink Display
 * Arduino IDE version for ESP32-S3 Feather + 2.13" E-ink FeatherWing
 * 

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Adafruit_ThinkInk.h"

// Include fonts
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

// WiFi credentials - CHANGE THESE!
const char* ssid = "StudentNet";
const char* password = "WIFI PASSWORD HERE";

// API URL for Muni data
const char* JSON_DATA_URL = "https://webservices.umoiq.com/api/pub/v1/agencies/sfmta-cis/stopcodes/13548/predictions?key=0be8ebd0284ce712a63f29dcaf7798c4";

// NTP server for time synchronization
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 3600;  // PST (adjust for your timezone)
const int daylightOffset_sec = 3600;   // DST offset

// E-ink display pins
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI

// Initialize the 2.13" monochrome display
ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

// Update interval (1 minute)
const unsigned long UPDATE_INTERVAL = 60000; // 1 minute in milliseconds
unsigned long lastUpdate = 0;
int errorCount = 0;

// Data storage
String muniRoute = "";
String muniStop = "";
String muniEta = "";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("SF Muni Tracker Starting...");
  
  // Initialize E-ink display
  Serial.println("Initializing E-ink display...");
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Show startup screen
  showStartupScreen();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized");
  
  Serial.println("Setup complete!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check if it's time to update
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL || lastUpdate == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      fetchMuniData();
    } else {
      Serial.println("WiFi disconnected, attempting reconnection...");
      connectToWiFi();
    }
    lastUpdate = currentMillis;
  }
  
  // Small delay to prevent excessive CPU usage
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi");
  }
}

void fetchMuniData() {
  Serial.println("Fetching Muni data...");
  
  HTTPClient http;
  http.begin(JSON_DATA_URL);
  http.addHeader("User-Agent", "ESP32-Muni-Tracker");
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
    
    // Parse JSON
    if (parseJsonData(payload)) {
      errorCount = 0; // Reset error count on success
      updateDisplay();
      Serial.println("Display updated successfully");
    } else {
      Serial.println("Failed to parse JSON data");
      handleError("JSON Parse Error");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
    handleError("HTTP Error " + String(httpResponseCode));
  }
  
  http.end();
}

bool parseJsonData(String jsonString) {
  // Allocate JSON document
  DynamicJsonDocument doc(2048);
  
  // Parse JSON
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Check if array has data
  if (doc.size() == 0) {
    Serial.println("No predictions available");
    muniRoute = "No Data";
    muniStop = "No predictions available";
    muniEta = "N/A";
    return true;
  }
  
  // Extract data from first prediction
  JsonObject firstPrediction = doc[0];
  
  muniRoute = firstPrediction["route"]["title"].as<String>();
  muniStop = firstPrediction["stop"]["name"].as<String>();
  muniEta = firstPrediction["values"][0]["minutes"].as<String>();
  
  // Print extracted data
  Serial.println("Extracted data:");
  Serial.println("Route: " + muniRoute);
  Serial.println("Stop: " + muniStop);
  Serial.println("ETA: " + muniEta + " minutes");
  
  return true;
}

void updateDisplay() {
  Serial.println("Updating E-ink display...");
  
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Get current time
  String currentTime = getFormattedTime();
  
  // Timestamp (small, upper left) - default font
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(currentTime);
  
  // Title - FreeSerif 18pt
  display.setFont(&FreeSerif18pt7b);
  display.setCursor(10, 30);
  display.print("MUNI");
  
  // Route info - FreeSerif 12pt
  display.setFont(&FreeSerif12pt7b);
  display.setCursor(10, 50);
  display.print("Route ");
  display.print(muniRoute);
  
  // ETA - FreeSerif 18pt (large for visibility)
  display.setFont(&FreeSerif18pt7b);
  if (muniEta == "0") {
    display.setCursor(10, 80);
    display.print("ARRIVING!");
  } else if (muniEta == "N/A") {
    display.setCursor(10, 80);
    display.print("No ETA");
  } else {
    display.setCursor(10, 80);
    display.print(muniEta);
    display.setFont(&FreeSerif12pt7b);
    display.print(" min");
  }
  
  // Stop info (bottom) - FreeSerif 9pt
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(10, 105);
  
  // Truncate long stop names to fit display
  String truncatedStop = muniStop;
  if (truncatedStop.length() > 30) {
    truncatedStop = truncatedStop.substring(0, 27) + "...";
  }
  display.print(truncatedStop);
  
  // Reset font and refresh display
  display.setFont();
  display.display();
  Serial.println("E-ink display updated at " + currentTime);
}

void showStartupScreen() {
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  display.setFont(&FreeSerif18pt7b);
  display.setCursor(10, 35);
  display.print("SF MUNI");
  
  display.setFont(&FreeSerif12pt7b);
  display.setCursor(10, 60);
  display.print("TRACKER");
  
  display.setFont();
  display.setTextSize(1);
  display.setCursor(10, 80);
  display.print("Starting up...");
  display.setCursor(10, 95);
  display.print("Connecting to WiFi...");
  
  display.display();
}

void showErrorScreen(String errorMsg) {
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Title - FreeSerif 18pt
  display.setFont(&FreeSerif18pt7b);
  display.setCursor(10, 30);
  display.print("MUNI");
  
  // Error message - default font
  display.setFont();
  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("ERROR:");
  display.setCursor(10, 62);
  display.print(errorMsg);
  
  display.setCursor(10, 80);
  display.print("Retrying in 1 min...");
  
  display.setCursor(10, 100);
  display.print("Time: " + getFormattedTime());
  
  display.display();
}

void handleError(String errorMsg) {
  errorCount++;
  Serial.println("Error #" + String(errorCount) + ": " + errorMsg);
  
  showErrorScreen(errorMsg);
  
  // If multiple errors, extend delay
  if (errorCount > 3) {
    Serial.println("Multiple errors detected, extending delay...");
    delay(300000); // Wait 5 minutes
    errorCount = 0;
  }
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // If NTP time isn't available, use millis() for relative time
    unsigned long minutes = millis() / 60000;
    return String(minutes) + "m";
  }
  
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%m/%d %I:%M%p", &timeinfo);
  return String(timeString);
}

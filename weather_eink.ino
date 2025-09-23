#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Adafruit_ThinkInk.h"

// WiFi credentials
const char* ssid = "StudentNet";
const char* password = "PASSWORD";  // ENTER THE WIFI PASSWORD!

// OpenWeatherMap API
const char* APPID = "YOUR APP ID"; // GET YOUR AP ID from: https://openweathermap.org/api
const char* baseURL = "http://api.openweathermap.org/data/2.5/weather?q=94112,us&APPID=";
const char* units = "&units=imperial";

// NTP server for time synchronization
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 3600;  // PST (adjust for your timezone)
const int daylightOffset_sec = 3600;   // DST offset

// E-ink display setup
#ifdef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK
#define EPD_DC PIN_EPD_DC
#define EPD_CS PIN_EPD_CS
#define EPD_BUSY PIN_EPD_BUSY
#define SRAM_CS -1
#define EPD_RESET PIN_EPD_RESET
#define EPD_SPI &SPI1
#else
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI
#endif

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  display.display();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("Connected to WiFi!");
  
  // Initialize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Error";
  }
  
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%m/%d %I:%M%p", &timeinfo);
  return String(timeString);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Build the complete URL
    String url = String(baseURL) + String(APPID) + String(units);
    http.begin(url);
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      
      // Parse JSON
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      
      // Extract weather data
      float temperature = doc["main"]["temp"];
      const char* location = doc["name"];
      const char* weatherType = doc["weather"][0]["description"];
      
      // Get current time
      String currentTime = getFormattedTime();
      
      // Clear display and show weather info
      display.clearBuffer();
      
      // Timestamp (small, upper left)
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(currentTime);
      
      // Location (top, moved down slightly)
      display.setTextSize(2);
      display.setCursor(10, 20);
      display.print(location);
      
      // Temperature (middle, large)
      display.setTextSize(4);
      display.setCursor(10, 50);
      display.print((int)temperature);
      display.print("F");
      
      // Weather type (bottom)
      display.setTextSize(3);
      display.setCursor(10, 90);
      display.print(weatherType);
      
      display.display();
      
      Serial.println("Weather updated on display at " + currentTime);
      
    } else {
      Serial.println("Error getting weather data");
    }
    
    http.end();
  }
  
  // Update every 10 minutes (600 seconds) to be more reasonable for e-ink
  delay(600000);
}

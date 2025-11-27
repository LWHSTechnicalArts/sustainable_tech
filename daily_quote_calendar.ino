#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Adafruit_ThinkInk.h"
#include "Adafruit_MAX1704X.h"

// WiFi credentials
const char* ssid = "StudentNet";
const char* password = "PASSWORD HERE";

// Time configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800;  // PST = UTC-8 hours
const int daylightOffset_sec = 3600; // DST offset

// Sleep duration
#define uS_TO_S_FACTOR 1000000ULL

// Quote API
const char* quoteAPI = "https://api.quotable.io/random";

// E-ink display pins
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);
Adafruit_MAX17048 maxlipo;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n=== Starting daily quote display ===");
  
  // Initialize battery monitor
  if (!maxlipo.begin()) {
    Serial.println(F("Could not find MAX17048 battery monitor"));
  } else {
    Serial.println(F("Found MAX17048 battery monitor"));
  }
  
  // Initialize display
  Serial.println("Initializing E-ink display...");
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Show startup message
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.print("Loading...");
  display.display();
  delay(2000);
  
  // Connect to WiFi
  connectWiFi();
  
  // Get current time
  Serial.println("Getting time from NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set
  Serial.print("Waiting for time");
  int timeAttempts = 0;
  while (timeAttempts < 40) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.println(" Time set!");
      break;
    }
    Serial.print(".");
    delay(500);
    timeAttempts++;
  }
  
  // Get and display quote
  displayDailyQuote();
  
  // Calculate seconds until next midnight
  uint64_t sleepTime = calculateSecondsUntilMidnight();
  
  Serial.print("Current time: ");
  Serial.println(getFormattedTime());
  Serial.print("Sleeping for ");
  Serial.print(sleepTime);
  Serial.print(" seconds (");
  Serial.print(sleepTime / 3600.0);
  Serial.println(" hours) until midnight");
  
  // Disconnect WiFi to save power
  Serial.println("Disconnecting WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Go to deep sleep until midnight
  Serial.println("Going to deep sleep...");
  Serial.flush();
  
  esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Never reached due to deep sleep
}

uint64_t calculateSecondsUntilMidnight() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time, defaulting to 24 hours");
    return 86400; // Default to 24 hours if we can't get time
  }
  
  // Current time in seconds since midnight
  int currentSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  
  // Seconds until midnight
  int secondsUntilMidnight = 86400 - currentSeconds;
  
  Serial.print("Current time: ");
  Serial.print(timeinfo.tm_hour);
  Serial.print(":");
  Serial.print(timeinfo.tm_min);
  Serial.print(":");
  Serial.println(timeinfo.tm_sec);
  Serial.print("Seconds until midnight: ");
  Serial.println(secondsUntilMidnight);
  
  return secondsUntilMidnight;
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
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

void displayDailyQuote() {
  Serial.println("=== Displaying daily quote ===");
  
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
// Battery percentage (upper right corner)
float batteryPercent = maxlipo.cellPercent();
display.setTextSize(1);
if (!isnan(batteryPercent)) {
  display.setCursor(display.width() - 80, 0);  // Moved left to fit "Battery: "
  display.print("Battery: ");
  display.print(batteryPercent, 0);
  display.print("%");
  Serial.print("Battery: ");
  Serial.print(batteryPercent, 1);
  Serial.println("%");
}
  
  // Get and display date
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char dayStr[15];
    char dateStr[30];
    
    strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);
    strftime(dateStr, sizeof(dateStr), "%B %d", &timeinfo);
    
    // Display day of week (large)
    display.setTextSize(3);
    display.setCursor(10, 15);
    display.print(dayStr);
    
    // Display date (medium)
    display.setTextSize(2);
    display.setCursor(10, 40);
    display.print(dateStr);
    
    Serial.print("Date: ");
    Serial.print(dayStr);
    Serial.print(", ");
    Serial.println(dateStr);
  } else {
    Serial.println("Could not get time");
    display.setTextSize(2);
    display.setCursor(10, 15);
    display.print("Time N/A");
  }
  
  display.drawLine(0, 65, display.width(), 65, EPD_BLACK);
  
  // Get and display quote
  String quote = "";
  String author = "";
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Fetching quote from API...");
    HTTPClient http;
    http.begin(quoteAPI);
    http.addHeader("User-Agent", "ESP32-Quote-Display");
    http.setTimeout(10000);
    
    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        quote = doc["content"].as<String>();
        author = doc["author"].as<String>();
        
        Serial.println("Quote: " + quote);
        Serial.println("Author: " + author);
      } else {
        quote = "Error parsing quote";
      }
    } else {
      quote = "Failed to fetch quote";
    }
    
    http.end();
  } else {
    quote = "No WiFi connection";
  }
  
  if (quote.length() == 0) {
    quote = "Make each day count.";
    author = "Unknown";
  }
  
  // Display quote with word wrapping
  display.setTextSize(1);
  int yPos = 73;
  int lineHeight = 10;
  int maxWidth = display.width() - 20;
  int maxLines = 3;
  int lineCount = 0;
  
  String words[150];
  int wordCount = 0;
  int startPos = 0;
  
  for (int i = 0; i <= quote.length(); i++) {
    if (i == quote.length() || quote[i] == ' ') {
      if (i > startPos) {
        words[wordCount++] = quote.substring(startPos, i);
      }
      startPos = i + 1;
    }
  }
  
  String currentLine = "";
  for (int i = 0; i < wordCount && lineCount < maxLines; i++) {
    String testLine = currentLine;
    if (currentLine.length() > 0) testLine += " ";
    testLine += words[i];
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(testLine.c_str(), 0, 0, &x1, &y1, &w, &h);
    
    if (w > maxWidth && currentLine.length() > 0) {
      display.setCursor(10, yPos);
      display.print(currentLine);
      yPos += lineHeight;
      lineCount++;
      currentLine = words[i];
    } else {
      currentLine = testLine;
    }
  }
  
  if (currentLine.length() > 0 && lineCount < maxLines) {
    display.setCursor(10, yPos);
    display.print(currentLine);
    yPos += lineHeight + 3;
  }
  
  if (author.length() > 0 && yPos < 110) {
    String truncatedAuthor = author;
    if (truncatedAuthor.length() > 25) {
      truncatedAuthor = truncatedAuthor.substring(0, 22) + "...";
    }
    
    display.setCursor(10, yPos);
    display.print("- ");
    display.print(truncatedAuthor);
  }
  
  Serial.println("Updating E-ink display...");
  display.display();
  Serial.println("Display update complete!");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    unsigned long minutes = millis() / 60000;
    return String(minutes) + "m";
  }
  
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%m/%d %I:%M%p", &timeinfo);
  return String(timeString);
}

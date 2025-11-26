//to program during deep sleep, hold boot when uploading

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Adafruit_ThinkInk.h"

// WiFi credentials
const char* ssid = "StudentNet";
const char* password = "";

// Time configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800;  // PST = UTC-8 hours
const int daylightOffset_sec = 3600; // DST offset

// Sleep duration (24 hours in microseconds)
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP  86400  // 24 hours in seconds

// Quote API
const char* quoteAPI = "https://api.quotable.io/random";

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

// Initialize the 2.13" monochrome display 
ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n=== Starting daily quote display ===");
  
  // Initialize display 
  Serial.println("Initializing E-ink display...");
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  Serial.println("Display initialized");
  
  // Show startup message
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.print("Loading...");
  Serial.println("Updating display...");
  display.display();
  Serial.println("Startup screen shown");
  
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
  
  // Disconnect WiFi to save power
  Serial.println("Disconnecting WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Go to deep sleep for 24 hours
  Serial.println("Going to deep sleep for 24 hours...");
  Serial.flush();
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Never reached due to deep sleep
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
  
  // Get current time (like your Muni tracker)
  String currentTime = getFormattedTime();
  
  // Timestamp (small, upper left - like your Muni tracker)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(currentTime);
  
  // Get and display date
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char dayStr[15];
    char dateStr[30];
    
    // Format: "Monday"
    strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);
    // Format: "November 26"
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
  
  // Draw separator line
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
      Serial.println("Response received");
      
      // Parse JSON
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        quote = doc["content"].as<String>();
        author = doc["author"].as<String>();
        
        Serial.println("Quote: " + quote);
        Serial.println("Author: " + author);
      } else {
        quote = "Error parsing quote";
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
      }
    } else {
      quote = "Failed to fetch quote";
      Serial.print("HTTP Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    quote = "No WiFi connection";
    Serial.println("No WiFi - cannot fetch quote");
  }
  
  // Fallback quote if needed
  if (quote.length() == 0) {
    quote = "Make each day count.";
    author = "Unknown";
  }
  
  // Display quote with word wrapping 
  display.setTextSize(1);
  int yPos = 73;
  int lineHeight = 10;
  int maxWidth = display.width() - 20;  // Leave margins
  int maxLines = 3;  // Limit to 3 lines for the quote
  int lineCount = 0;
  
  // Split quote into words
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
  
  // Display words with wrapping
  String currentLine = "";
  for (int i = 0; i < wordCount && lineCount < maxLines; i++) {
    String testLine = currentLine;
    if (currentLine.length() > 0) testLine += " ";
    testLine += words[i];
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(testLine.c_str(), 0, 0, &x1, &y1, &w, &h);
    
    if (w > maxWidth && currentLine.length() > 0) {
      // Print current line
      display.setCursor(10, yPos);
      display.print(currentLine);
      yPos += lineHeight;
      lineCount++;
      currentLine = words[i];
    } else {
      currentLine = testLine;
    }
  }
  
  // Print remaining line
  if (currentLine.length() > 0 && lineCount < maxLines) {
    display.setCursor(10, yPos);
    display.print(currentLine);
    yPos += lineHeight + 3;
  }
  
  // Display author 
  if (author.length() > 0 && yPos < 110) {
    // Truncate long author names
    String truncatedAuthor = author;
    if (truncatedAuthor.length() > 25) {
      truncatedAuthor = truncatedAuthor.substring(0, 22) + "...";
    }
    
    display.setCursor(10, yPos);
    display.print("- ");
    display.print(truncatedAuthor);
  }
  
  // Update display 
  Serial.println("Updating E-ink display...");
  display.display();
  Serial.println("Display update complete!");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // If NTP time isn't available, use millis()
    unsigned long minutes = millis() / 60000;
    return String(minutes) + "m";
  }
  
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%m/%d %I:%M%p", &timeinfo);
  return String(timeString);
}

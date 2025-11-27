/*
 * ESP32-S3 Feather Calendar Display with 2.13" E-ink Display
 * Arduino IDE version for ESP32-S3 Feather + 2.13" E-ink FeatherWing
 * Fetches and displays upcoming events from iCalendar (.ics) feeds
 * 
 * Required Libraries (install via Library Manager):
 * - Adafruit GFX Library
 * - Adafruit ThinkInk
 * - WiFi (built-in with ESP32)
 * - HTTPClient (built-in with ESP32)
 * 
 * Board: Select "Adafruit Feather ESP32-S3" in Arduino IDE
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "Adafruit_ThinkInk.h"

// WiFi credentials - CHANGE THESE!
const char* ssid = "StudentNet";
const char* password = "WIFI PASSWORD HERE";  // ENTER THE WIFI PASSWORD!

// iCalendar feed URL
// Test URLs you can try:
const char* ICS_URL = "https://www.lwhs.org/calendar/calendar_349.ics";

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

// Update interval (10 minutes for calendar data)
const unsigned long UPDATE_INTERVAL = 600000; // 10 minutes in milliseconds
unsigned long lastUpdate = 0;
int errorCount = 0;

// Event structure
struct CalendarEvent {
  String summary;
  String startDate;
  String startTime;
  String description;
  time_t startTimestamp;
  bool isValid;
};

// Array to store upcoming events
const int MAX_EVENTS = 5;
CalendarEvent upcomingEvents[MAX_EVENTS];
int eventCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Calendar Display Starting...");
  
  // Initialize E-ink display
  Serial.println("Initializing E-ink display...");
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Show startup screen
  showStartupScreen();
  
  // Scan for available networks first (for debugging)
  scanWiFiNetworks();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized");
  
  // Wait a moment for time to sync
  delay(2000);
  
  Serial.println("Setup complete!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check if it's time to update
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL || lastUpdate == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      fetchCalendarData();
    } else {
      Serial.println("WiFi disconnected, attempting reconnection...");
      connectToWiFi();
    }
    lastUpdate = currentMillis;
  }
  
  delay(43200000);   // 43,200,000 milliseconds = 12 hours
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  // Try to disconnect first in case of previous connection issues
  WiFi.disconnect();
  delay(100);
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) { // Increased to 60 attempts
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Print WiFi status every 10 attempts
    if (attempts % 10 == 0) {
      Serial.println();
      Serial.print("Attempt ");
      Serial.print(attempts);
      Serial.print(", WiFi Status: ");
      printWiFiStatus();
      
      // Try reconnecting every 20 attempts
      if (attempts % 20 == 0) {
        Serial.println("Retrying connection...");
        WiFi.disconnect();
        delay(500);
        WiFi.begin(ssid, password);
      }
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi after all attempts");
    Serial.print("Final WiFi Status: ");
    printWiFiStatus();
  }
}

void printWiFiStatus() {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      Serial.println("Connected");
      break;
    case WL_NO_SHIELD:
      Serial.println("No WiFi shield");
      break;
    case WL_IDLE_STATUS:
      Serial.println("Idle");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("No SSID available - Network not found");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("Scan completed");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Connection failed - Wrong password?");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("Connection lost");
      break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    default:
      Serial.print("Unknown status: ");
      Serial.println(WiFi.status());
      break;
  }
}

void fetchCalendarData() {
  Serial.println("Fetching calendar data...");
  
  HTTPClient http;
  http.begin(ICS_URL);
  http.addHeader("User-Agent", "ESP32-Calendar-Display");
  
  // Enable redirect following
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
    Serial.print("Payload size: ");
    Serial.println(payload.length());
    
    // Debug: print first 200 characters of payload
    if (payload.length() > 0) {
      Serial.println("Payload preview:");
      Serial.println(payload.substring(0, min(200, (int)payload.length())));
      Serial.println("---");
    }
    
    // Check if we got actual iCalendar data
    if (httpResponseCode == 200 && payload.length() > 0 && payload.indexOf("BEGIN:VCALENDAR") != -1) {
      // Parse iCalendar data
      if (parseICalendarData(payload)) {
        errorCount = 0; // Reset error count on success
        updateDisplay();
        Serial.println("Display updated successfully");
      } else {
        Serial.println("Failed to parse calendar data");
        handleError("Parse Error");
      }
    } else if (httpResponseCode == 200 && payload.length() > 0) {
      Serial.println("Received data but it doesn't appear to be iCalendar format");
      handleError("Not iCalendar format");
    } else {
      Serial.println("No data received or bad response");
      handleError("No data received");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
    handleError("HTTP Error " + String(httpResponseCode));
  }
  
  http.end();
}

bool parseICalendarData(String icsData) {
  eventCount = 0;
  
  // Get current time for filtering upcoming events
  time_t now;
  time(&now);
  
  // Split the data into lines
  int startPos = 0;
  int endPos = 0;
  
  String currentSummary = "";
  String currentStartDate = "";
  String currentDescription = "";
  bool inEvent = false;
  
  Serial.println("Parsing iCalendar data...");
  
  while (endPos != -1 && eventCount < MAX_EVENTS) {
    endPos = icsData.indexOf('\n', startPos);
    String line;
    
    if (endPos == -1) {
      line = icsData.substring(startPos);
    } else {
      line = icsData.substring(startPos, endPos);
    }
    
    // Remove carriage return if present
    line.trim();
    
    // Process the line
    if (line.startsWith("BEGIN:VEVENT")) {
      inEvent = true;
      currentSummary = "";
      currentStartDate = "";
      currentDescription = "";
    } else if (line.startsWith("END:VEVENT")) {
      if (inEvent && currentSummary != "" && currentStartDate != "") {
        // Parse the date/time
        time_t eventTime = parseICalDateTime(currentStartDate);
        
        // Only add future events
        if (eventTime > now && eventCount < MAX_EVENTS) {
          upcomingEvents[eventCount].summary = currentSummary;
          upcomingEvents[eventCount].startDate = formatDate(eventTime);
          upcomingEvents[eventCount].startTime = formatTime(eventTime);
          upcomingEvents[eventCount].description = currentDescription;
          upcomingEvents[eventCount].startTimestamp = eventTime;
          upcomingEvents[eventCount].isValid = true;
          
          Serial.println("Added event: " + currentSummary + " on " + upcomingEvents[eventCount].startDate);
          eventCount++;
        }
      }
      inEvent = false;
    } else if (inEvent) {
      if (line.startsWith("SUMMARY:")) {
        currentSummary = line.substring(8);
        currentSummary.trim();
      } else if (line.startsWith("DTSTART")) {
        // Handle both DTSTART and DTSTART;VALUE=DATE formats
        int colonPos = line.indexOf(':');
        if (colonPos != -1) {
          currentStartDate = line.substring(colonPos + 1);
          currentStartDate.trim();
        }
      } else if (line.startsWith("DESCRIPTION:")) {
        currentDescription = line.substring(12);
        currentDescription.trim();
        // Limit description length
        if (currentDescription.length() > 50) {
          currentDescription = currentDescription.substring(0, 47) + "...";
        }
      }
    }
    
    startPos = endPos + 1;
  }
  
  Serial.print("Found ");
  Serial.print(eventCount);
  Serial.println(" upcoming events");
  
  return eventCount > 0;
}

time_t parseICalDateTime(String dateTimeStr) {
  // Parse iCalendar date/time format (YYYYMMDDTHHMMSSZ or YYYYMMDD)
  struct tm tm = {0};
  
  if (dateTimeStr.length() >= 8) {
    // Extract year, month, day
    String yearStr = dateTimeStr.substring(0, 4);
    String monthStr = dateTimeStr.substring(4, 6);
    String dayStr = dateTimeStr.substring(6, 8);
    
    tm.tm_year = yearStr.toInt() - 1900;
    tm.tm_mon = monthStr.toInt() - 1;
    tm.tm_mday = dayStr.toInt();
    
    // If there's time information
    if (dateTimeStr.length() >= 15 && dateTimeStr.charAt(8) == 'T') {
      String hourStr = dateTimeStr.substring(9, 11);
      String minStr = dateTimeStr.substring(11, 13);
      String secStr = dateTimeStr.substring(13, 15);
      
      tm.tm_hour = hourStr.toInt();
      tm.tm_min = minStr.toInt();
      tm.tm_sec = secStr.toInt();
    } else {
      // All-day event, set to noon
      tm.tm_hour = 12;
      tm.tm_min = 0;
      tm.tm_sec = 0;
    }
  }
  
  return mktime(&tm);
}

String formatDate(time_t timestamp) {
  struct tm* timeinfo = localtime(&timestamp);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%m/%d", timeinfo);
  return String(buffer);
}

String formatTime(time_t timestamp) {
  struct tm* timeinfo = localtime(&timestamp);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%I:%M%p", timeinfo);
  return String(buffer);
}

void updateDisplay() {
  Serial.println("Updating E-ink display...");
  
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Get current time
  String currentTime = getFormattedTime();
  
  // Timestamp (small, upper left)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(currentTime);
  
  // Title
  display.setTextSize(2);
  display.setCursor(5, 15);
  display.print("UPCOMING MENU");
  
  // Display events
  int yPos = 40;
  int maxEventsToShow = min(eventCount, 3); // Show up to 3 events
  
  if (eventCount == 0) {
    display.setTextSize(1);
    display.setCursor(5, yPos);
    display.print("No upcoming events");
  } else {
    for (int i = 0; i < maxEventsToShow; i++) {
      if (upcomingEvents[i].isValid) {
        // Event date and time
        display.setTextSize(1);
        display.setCursor(5, yPos);
        display.print(upcomingEvents[i].startDate);
        
        // Event title (truncate if too long)
        String title = upcomingEvents[i].summary;
        if (title.length() > 25) {
          title = title.substring(0, 36) + "...";
        }
        
        yPos += 10;
        display.setCursor(5, yPos);
        display.print(title);
        
        yPos += 15; // Space between events
        
        // Don't exceed display bounds
        if (yPos > 100) break;
      }
    }
  }
  
  // Show total event count at bottom
  if (eventCount > maxEventsToShow) {
    display.setTextSize(1);
    display.setCursor(5, 110);
    display.print("+" + String(eventCount - maxEventsToShow) + " more events");
  }
  
  // Refresh display
  display.display();
  Serial.println("E-ink display updated at " + currentTime);
}

void showStartupScreen() {
  display.clearBuffer();
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.setCursor(10, 20);
  display.print("CALENDAR");
  display.setCursor(10, 40);
  display.print("DISPLAY");
  display.setTextSize(1);
  display.setCursor(10, 70);
  display.print("Starting up...");
  display.setCursor(10, 85);
  display.print("Connecting to WiFi...");
  display.display();
}

void showErrorScreen(String errorMsg) {
  display.clearBuffer();
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.setCursor(10, 15);
  display.print("CALENDAR");
  display.setCursor(10, 30);
  display.print("DISPLAY");
  
  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("ERROR: " + errorMsg);
  
  display.setCursor(10, 65);
  display.print("Retrying in 10 min...");
  
  display.setCursor(10, 85);
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
    delay(600000); // Wait 10 minutes
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

void scanWiFiNetworks() {
  Serial.println("Scanning for WiFi networks...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks:");
  
  bool foundTarget = false;
  for (int i = 0; i < n; ++i) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(")");
    Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " [Open]" : " [Secured]");
    
    if (WiFi.SSID(i) == String(ssid)) {
      foundTarget = true;
      Serial.println("   *** TARGET NETWORK FOUND ***");
    }
  }
  
  if (!foundTarget) {
    Serial.println("*** WARNING: Target network not found in scan! ***");
    Serial.print("Looking for: ");
    Serial.println(ssid);
  }
  
  Serial.println("Scan complete.\n");
}

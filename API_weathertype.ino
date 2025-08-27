#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "StudentNet";
const char* password = "WIFI_PASSWORD_HERE";

// OpenWeatherMap API
const char* APPID = "API_KEY_HERE";
const char* url = "http://api.openweathermap.org/data/2.5/weather?q=94112,us&APPID=0e974dbb38ad3719228ac1854b212827";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("Connected to WiFi!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      
      // Parse JSON
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      
      // Get weather description
      const char* weatherType = doc["weather"][0]["description"];
      
      Serial.print("Weather: ");
      Serial.println(weatherType);
      
    } else {
      Serial.println("Error getting weather data");
    }
    
    http.end();
  }
  
  delay(60000); // Check every minute
}

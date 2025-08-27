#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "StudentNet";
const char* password = "YOUR_WIFI_PASSWORD";

// OpenWeatherMap API
const char* APPID = "YOUR_API_KEY_HERE"; // Get from: https://openweathermap.org/api
const char* baseURL = "http://api.openweathermap.org/data/2.5/weather?q=94112,us&APPID=";
const char* units = "&units=imperial";

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
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
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
      
      // Parse JSON response
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      
      // Extract temperature and location
      float temperature = doc["main"]["temp"];
      const char* location = doc["name"];
      
      Serial.print("It's currently ");
      Serial.print(temperature);
      Serial.print(" degrees in ");
      Serial.print(location);
      Serial.println(".");
      
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
  
  delay(30000); // Wait 30 seconds before next request
}

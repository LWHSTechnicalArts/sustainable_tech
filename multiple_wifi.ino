#include <WiFi.h>

// WiFi credentials for multiple networks
char ssid1[] = "StudentNet";      // first network SSID
char pass1[] = "YOUR_FIRST_PASSWORD";  // first network password

char ssid2[] = "YOUR_SECOND_SSID";     // second network SSID
char pass2[] = "YOUR_SECOND_PASSWORD"; // second network password

int status = WL_IDLE_STATUS;

char server[] = "wifitest.adafruit.com";
char path[]   = "/testwifi/index.html";

WiFiClient client;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  // Try to connect to WiFi networks
  connectToWiFi();
  
  printWifiStatus();
  
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.print("GET "); client.print(path); client.println(" HTTP/1.1");
    client.print("Host: "); client.println(server);
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
    
    while (true) {
      delay(100);
    }
  }
}

void connectToWiFi() {
  // Try first network
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid1);
  WiFi.begin(ssid1, pass1);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to first WiFi network!");
    return;
  }
  
  // First network failed, try second network
  Serial.println("");
  Serial.println("First network failed. Trying second network...");
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid2);
  
  WiFi.begin(ssid2, pass2);
  
  attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to second WiFi network!");
    return;
  }
  
  // Both networks failed
  Serial.println("");
  Serial.println("Failed to connect to any WiFi network!");
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

#include "Adafruit_MAX1704X.h"
#include "Adafruit_ThinkInk.h"

Adafruit_MAX17048 maxlipo;

// E-ink display pins
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println(F("\nBattery Monitor with E-ink Display"));
  
  // Initialize e-ink display
  Serial.println("Initializing E-ink display...");
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Show startup screen
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.print("Battery");
  display.setCursor(10, 70);
  display.print("Monitor");
  display.display();
  
  // Initialize battery monitor
  while (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    
    // Show error on display
    display.clearBuffer();
    display.setTextSize(1);
    display.setCursor(10, 40);
    display.print("Battery sensor");
    display.setCursor(10, 55);
    display.print("not found!");
    display.setCursor(10, 70);
    display.print("Check connection");
    display.display();
    
    delay(2000);
  }
  
  Serial.print(F("Found MAX17048"));
  Serial.print(F(" with Chip ID: 0x")); 
  Serial.println(maxlipo.getChipID(), HEX);
  
  delay(1000);
}

void loop() {
  float cellVoltage = maxlipo.cellVoltage();
  float cellPercent = maxlipo.cellPercent();
  
  if (isnan(cellVoltage)) {
    Serial.println("Failed to read cell voltage, check battery is connected!");
    
    // Show error on display
    display.clearBuffer();
    display.setTextSize(2);
    display.setCursor(10, 50);
    display.print("No Battery");
    display.display();
    
    delay(2000);
    return;
  }
  
  // Print to serial
  Serial.print(F("Batt Voltage: ")); Serial.print(cellVoltage, 3); Serial.println(" V");
  Serial.print(F("Batt Percent: ")); Serial.print(cellPercent, 1); Serial.println(" %");
  Serial.println();
  
  // Update e-ink display
  updateDisplay(cellVoltage, cellPercent);
  
  delay(60000);  // Update every 60 seconds (e-ink friendly)
}

void updateDisplay(float voltage, float percent) {
  Serial.println("Updating display...");
  
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Title
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.print("BATTERY");
  
  // Draw separator line
  display.drawLine(0, 30, display.width(), 30, EPD_BLACK);
  
  // Battery percentage (large text)
  display.setTextSize(4);
  display.setCursor(15, 35);
  display.print(percent, 0);
  display.setTextSize(2);
  display.print("%");
  
  // Voltage (smaller text)
  display.setTextSize(1);
  display.setCursor(15, 70);
  display.print("Voltage: ");
  display.print(voltage, 2);
  display.print(" V");
  
  // Draw battery graph (horizontal bar)
  int graphX = 10;
  int graphY = 85;
  int graphWidth = 230;  // Total width
  int graphHeight = 20;
  
  // Draw outer rectangle (battery outline)
  display.drawRect(graphX, graphY, graphWidth, graphHeight, EPD_BLACK);
  
  // Draw battery terminal (little nub on the right)
  display.fillRect(graphX + graphWidth, graphY + 5, 5, graphHeight - 10, EPD_BLACK);
  
  // Calculate fill width based on percentage
  int fillWidth = (int)((graphWidth - 4) * (percent / 100.0));
  
  // Draw filled portion
  if (fillWidth > 0) {
    display.fillRect(graphX + 2, graphY + 2, fillWidth, graphHeight - 4, EPD_BLACK);
  }
  
  // Add status text below graph
  display.setTextSize(1);
  display.setCursor(10, 110);
  if (percent > 80) {
    display.print("Status: Excellent");
  } else if (percent > 50) {
    display.print("Status: Good");
  } else if (percent > 20) {
    display.print("Status: Low");
  } else {
    display.print("Status: CRITICAL!");
  }
  
  // Refresh display
  display.display();
  Serial.println("Display updated!");
}

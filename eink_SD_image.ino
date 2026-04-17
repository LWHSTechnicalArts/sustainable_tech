#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>
#include <SdFat.h>
#include <Adafruit_ImageReader_EPD.h>
#include "Adafruit_ThinkInk.h" 

// --- Pin Definitions for 2.13" Mono FeatherWing on ESP32-S3 Feather ---
#define EPD_DC    10
#define EPD_CS     9
#define SRAM_CS    6
#define EPD_RESET -1   // not connected on FeatherWing
#define EPD_BUSY  -1   // not connected on FeatherWing
#define SD_CS      5

// --- Display Object ---
// Use the correct initializer for your panel revision:
// Pre-Aug 2024:
ThinkInk_213_Mono_BN display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
// Post-Aug 2024 (uncomment if needed):
// ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

// --- SD and ImageReader ---
SdFat SD;
Adafruit_ImageReader_EPD reader(SD);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit eInk + SD Card Image Demo");

  // Initialize the display
  display.begin();

  // Initialize SD card — use 20 MHz on ESP32 for reliability
  if (!SD.begin(SD_CS, SD_SCK_MHZ(20))) {
    Serial.println("SD card init failed!");
    while (1);
  }
  Serial.println("SD card OK");

  // Clear the display buffer
  display.clearBuffer();

  // Draw BMP from SD card at top-left corner (0, 0)
  ImageReturnCode stat = reader.drawBMP("/image.bmp", display, 0, 0);
  reader.printStatus(stat);   // prints result to Serial

  // Push buffer to display
  display.display();

  Serial.println("Done!");
}

void loop() {
  // eInk images are static — nothing needed here
}

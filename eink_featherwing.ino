/***************************************************
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_ThinkInk.h"

// E-ink display pins
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI

// The GDEY0213B74 is like the B74 above but is not 'shifted down' by 8 pixels
ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit EPD full update test in mono");
  display.begin(THINKINK_MONO);
}

void loop() {
  Serial.println("Banner demo");
  display.clearBuffer();
  display.setTextSize(4);
  display.setCursor((display.width() - 180) / 2, (display.height() - 24) / 2);
  display.setTextColor(EPD_BLACK);
  display.print("Hello!");
  display.display();

  delay(2000);

  Serial.println("B/W rectangle demo");
  display.clearBuffer();
  display.fillRect(display.width() / 2, 0, display.width() / 2,
                   display.height(), EPD_BLACK);
  display.display();

  delay(2000);

  Serial.println("Text demo");
  // large block of text
  display.clearBuffer();
  display.setTextSize(2);
  testdrawtext(
      "Lick-Wilmerding is \n"
      "full of many \n"
      "wild teens \n"
      "living like \n"
      "there's no tomorrow! \n",
      EPD_BLACK);
  display.display();

  delay(2000);

  display.clearBuffer();
  
  // Draw a simple face
  int centerX = display.width() / 2;
  int centerY = display.height() / 2;
  int faceRadius = 40;
  
  // Face outline (circle)
  display.drawCircle(centerX, centerY, faceRadius, EPD_BLACK);
  
  // Eyes
  display.fillCircle(centerX - 15, centerY - 10, 8, EPD_BLACK);
  display.fillCircle(centerX + 15, centerY - 10, 8, EPD_BLACK);
  
  // Nose (small line)
  display.drawLine(centerX, centerY - 2, centerX, centerY + 5, EPD_BLACK);
  
  // Mouth (smile)
  for (int i = -15; i <= 15; i++) {
    int y = centerY + 15 + (i * i) / 25; // parabolic curve for smile
    display.drawPixel(centerX + i, y, EPD_BLACK);
  }
  
  display.display();

  delay(2000);
}

void testdrawtext(const char *text, uint16_t color) {
  display.setCursor(0, 0);
  display.setTextColor(color);
  display.setTextWrap(true);
  display.print(text);
}

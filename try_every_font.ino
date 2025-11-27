#include "Adafruit_ThinkInk.h"

// E-ink display pins
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY -1
#define SRAM_CS 6
#define EPD_RESET -1
#define EPD_SPI &SPI

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

// Include all the fonts
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoOblique12pt7b.h>
#include <Fonts/FreeMonoOblique18pt7b.h>
#include <Fonts/FreeMonoOblique24pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMonoBoldOblique18pt7b.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic18pt7b.h>
#include <Fonts/FreeSerifItalic24pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifBoldItalic24pt7b.h>

#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>

int currentFont = 0;
const int totalFonts = 51;  // Total number of fonts to display

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Font Display Demo Starting...");
  
  // Initialize display
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Show startup message
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.print("Font Demo");
  display.display();
  delay(2000);
}

void loop() {
  displayFont(currentFont);
  
  currentFont++;
  if (currentFont >= totalFonts) {
    currentFont = 0;  // Loop back to first font
  }
  
  delay(2000);  // Wait 2 seconds before next font
}

void displayFont(int fontIndex) {
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  
  // Title in default font
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Font #");
  display.print(fontIndex + 1);
  display.print(" of ");
  display.print(totalFonts);
  
  String fontName = "";
  int lineSpacing = 0;  // Will be set based on font size
  
  // Set the font based on index
  switch(fontIndex) {
    // FreeMono
    case 0: display.setFont(&FreeMono9pt7b); fontName = "FreeMono 9pt"; lineSpacing = 15; break;
    case 1: display.setFont(&FreeMono12pt7b); fontName = "FreeMono 12pt"; lineSpacing = 20; break;
    case 2: display.setFont(&FreeMono18pt7b); fontName = "FreeMono 18pt"; lineSpacing = 30; break;
    case 3: display.setFont(&FreeMono24pt7b); fontName = "FreeMono 24pt"; lineSpacing = 40; break;
    case 4: display.setFont(&FreeMonoBold9pt7b); fontName = "FreeMonoBold 9pt"; lineSpacing = 15; break;
    case 5: display.setFont(&FreeMonoBold12pt7b); fontName = "FreeMonoBold 12pt"; lineSpacing = 20; break;
    case 6: display.setFont(&FreeMonoBold18pt7b); fontName = "FreeMonoBold 18pt"; lineSpacing = 30; break;
    case 7: display.setFont(&FreeMonoBold24pt7b); fontName = "FreeMonoBold 24pt"; lineSpacing = 40; break;
    case 8: display.setFont(&FreeMonoOblique9pt7b); fontName = "FreeMonoOblique 9pt"; lineSpacing = 15; break;
    case 9: display.setFont(&FreeMonoOblique12pt7b); fontName = "FreeMonoOblique 12pt"; lineSpacing = 20; break;
    case 10: display.setFont(&FreeMonoOblique18pt7b); fontName = "FreeMonoOblique 18pt"; lineSpacing = 30; break;
    case 11: display.setFont(&FreeMonoOblique24pt7b); fontName = "FreeMonoOblique 24pt"; lineSpacing = 40; break;
    case 12: display.setFont(&FreeMonoBoldOblique9pt7b); fontName = "FreeMonoBoldOblique 9pt"; lineSpacing = 15; break;
    case 13: display.setFont(&FreeMonoBoldOblique12pt7b); fontName = "FreeMonoBoldOblique 12pt"; lineSpacing = 20; break;
    case 14: display.setFont(&FreeMonoBoldOblique18pt7b); fontName = "FreeMonoBoldOblique 18pt"; lineSpacing = 30; break;
    case 15: display.setFont(&FreeMonoBoldOblique24pt7b); fontName = "FreeMonoBoldOblique 24pt"; lineSpacing = 40; break;
    
    // FreeSans
    case 16: display.setFont(&FreeSans9pt7b); fontName = "FreeSans 9pt"; lineSpacing = 15; break;
    case 17: display.setFont(&FreeSans12pt7b); fontName = "FreeSans 12pt"; lineSpacing = 20; break;
    case 18: display.setFont(&FreeSans18pt7b); fontName = "FreeSans 18pt"; lineSpacing = 30; break;
    case 19: display.setFont(&FreeSans24pt7b); fontName = "FreeSans 24pt"; lineSpacing = 40; break;
    case 20: display.setFont(&FreeSansBold9pt7b); fontName = "FreeSansBold 9pt"; lineSpacing = 15; break;
    case 21: display.setFont(&FreeSansBold12pt7b); fontName = "FreeSansBold 12pt"; lineSpacing = 20; break;
    case 22: display.setFont(&FreeSansBold18pt7b); fontName = "FreeSansBold 18pt"; lineSpacing = 30; break;
    case 23: display.setFont(&FreeSansBold24pt7b); fontName = "FreeSansBold 24pt"; lineSpacing = 40; break;
    case 24: display.setFont(&FreeSansOblique9pt7b); fontName = "FreeSansOblique 9pt"; lineSpacing = 15; break;
    case 25: display.setFont(&FreeSansOblique12pt7b); fontName = "FreeSansOblique 12pt"; lineSpacing = 20; break;
    case 26: display.setFont(&FreeSansOblique18pt7b); fontName = "FreeSansOblique 18pt"; lineSpacing = 30; break;
    case 27: display.setFont(&FreeSansOblique24pt7b); fontName = "FreeSansOblique 24pt"; lineSpacing = 40; break;
    case 28: display.setFont(&FreeSansBoldOblique9pt7b); fontName = "FreeSansBoldOblique 9pt"; lineSpacing = 15; break;
    case 29: display.setFont(&FreeSansBoldOblique12pt7b); fontName = "FreeSansBoldOblique 12pt"; lineSpacing = 20; break;
    case 30: display.setFont(&FreeSansBoldOblique18pt7b); fontName = "FreeSansBoldOblique 18pt"; lineSpacing = 30; break;
    case 31: display.setFont(&FreeSansBoldOblique24pt7b); fontName = "FreeSansBoldOblique 24pt"; lineSpacing = 40; break;
    
    // FreeSerif
    case 32: display.setFont(&FreeSerif9pt7b); fontName = "FreeSerif 9pt"; lineSpacing = 15; break;
    case 33: display.setFont(&FreeSerif12pt7b); fontName = "FreeSerif 12pt"; lineSpacing = 20; break;
    case 34: display.setFont(&FreeSerif18pt7b); fontName = "FreeSerif 18pt"; lineSpacing = 30; break;
    case 35: display.setFont(&FreeSerif24pt7b); fontName = "FreeSerif 24pt"; lineSpacing = 40; break;
    case 36: display.setFont(&FreeSerifBold9pt7b); fontName = "FreeSerifBold 9pt"; lineSpacing = 15; break;
    case 37: display.setFont(&FreeSerifBold12pt7b); fontName = "FreeSerifBold 12pt"; lineSpacing = 20; break;
    case 38: display.setFont(&FreeSerifBold18pt7b); fontName = "FreeSerifBold 18pt"; lineSpacing = 30; break;
    case 39: display.setFont(&FreeSerifBold24pt7b); fontName = "FreeSerifBold 24pt"; lineSpacing = 40; break;
    case 40: display.setFont(&FreeSerifItalic9pt7b); fontName = "FreeSerifItalic 9pt"; lineSpacing = 15; break;
    case 41: display.setFont(&FreeSerifItalic12pt7b); fontName = "FreeSerifItalic 12pt"; lineSpacing = 20; break;
    case 42: display.setFont(&FreeSerifItalic18pt7b); fontName = "FreeSerifItalic 18pt"; lineSpacing = 30; break;
    case 43: display.setFont(&FreeSerifItalic24pt7b); fontName = "FreeSerifItalic 24pt"; lineSpacing = 40; break;
    case 44: display.setFont(&FreeSerifBoldItalic9pt7b); fontName = "FreeSerifBoldItalic 9pt"; lineSpacing = 15; break;
    case 45: display.setFont(&FreeSerifBoldItalic12pt7b); fontName = "FreeSerifBoldItalic 12pt"; lineSpacing = 20; break;
    case 46: display.setFont(&FreeSerifBoldItalic18pt7b); fontName = "FreeSerifBoldItalic 18pt"; lineSpacing = 30; break;
    case 47: display.setFont(&FreeSerifBoldItalic24pt7b); fontName = "FreeSerifBoldItalic 24pt"; lineSpacing = 40; break;
    
    // Tiny fonts
    case 48: display.setFont(&Org_01); fontName = "Org_01"; lineSpacing = 10; break;
    case 49: display.setFont(&Picopixel); fontName = "Picopixel"; lineSpacing = 8; break;
    case 50: display.setFont(&TomThumb); fontName = "TomThumb"; lineSpacing = 10; break;
  }
  
  // Display font name
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print(fontName);
  
  // Calculate starting Y position based on font size
  int startY;
  if (fontIndex >= 48) {
    startY = 30;  // Tiny fonts
  } else if (fontIndex % 4 == 0) {
    startY = 40;  // 9pt fonts
  } else if (fontIndex % 4 == 1) {
    startY = 45;  // 12pt fonts
  } else if (fontIndex % 4 == 2) {
    startY = 55;  // 18pt fonts
  } else {
    startY = 70;  // 24pt fonts
  }
  
  // Set the font again for sample text
  switch(fontIndex) {
    case 0: display.setFont(&FreeMono9pt7b); break;
    case 1: display.setFont(&FreeMono12pt7b); break;
    case 2: display.setFont(&FreeMono18pt7b); break;
    case 3: display.setFont(&FreeMono24pt7b); break;
    case 4: display.setFont(&FreeMonoBold9pt7b); break;
    case 5: display.setFont(&FreeMonoBold12pt7b); break;
    case 6: display.setFont(&FreeMonoBold18pt7b); break;
    case 7: display.setFont(&FreeMonoBold24pt7b); break;
    case 8: display.setFont(&FreeMonoOblique9pt7b); break;
    case 9: display.setFont(&FreeMonoOblique12pt7b); break;
    case 10: display.setFont(&FreeMonoOblique18pt7b); break;
    case 11: display.setFont(&FreeMonoOblique24pt7b); break;
    case 12: display.setFont(&FreeMonoBoldOblique9pt7b); break;
    case 13: display.setFont(&FreeMonoBoldOblique12pt7b); break;
    case 14: display.setFont(&FreeMonoBoldOblique18pt7b); break;
    case 15: display.setFont(&FreeMonoBoldOblique24pt7b); break;
    case 16: display.setFont(&FreeSans9pt7b); break;
    case 17: display.setFont(&FreeSans12pt7b); break;
    case 18: display.setFont(&FreeSans18pt7b); break;
    case 19: display.setFont(&FreeSans24pt7b); break;
    case 20: display.setFont(&FreeSansBold9pt7b); break;
    case 21: display.setFont(&FreeSansBold12pt7b); break;
    case 22: display.setFont(&FreeSansBold18pt7b); break;
    case 23: display.setFont(&FreeSansBold24pt7b); break;
    case 24: display.setFont(&FreeSansOblique9pt7b); break;
    case 25: display.setFont(&FreeSansOblique12pt7b); break;
    case 26: display.setFont(&FreeSansOblique18pt7b); break;
    case 27: display.setFont(&FreeSansOblique24pt7b); break;
    case 28: display.setFont(&FreeSansBoldOblique9pt7b); break;
    case 29: display.setFont(&FreeSansBoldOblique12pt7b); break;
    case 30: display.setFont(&FreeSansBoldOblique18pt7b); break;
    case 31: display.setFont(&FreeSansBoldOblique24pt7b); break;
    case 32: display.setFont(&FreeSerif9pt7b); break;
    case 33: display.setFont(&FreeSerif12pt7b); break;
    case 34: display.setFont(&FreeSerif18pt7b); break;
    case 35: display.setFont(&FreeSerif24pt7b); break;
    case 36: display.setFont(&FreeSerifBold9pt7b); break;
    case 37: display.setFont(&FreeSerifBold12pt7b); break;
    case 38: display.setFont(&FreeSerifBold18pt7b); break;
    case 39: display.setFont(&FreeSerifBold24pt7b); break;
    case 40: display.setFont(&FreeSerifItalic9pt7b); break;
    case 41: display.setFont(&FreeSerifItalic12pt7b); break;
    case 42: display.setFont(&FreeSerifItalic18pt7b); break;
    case 43: display.setFont(&FreeSerifItalic24pt7b); break;
    case 44: display.setFont(&FreeSerifBoldItalic9pt7b); break;
    case 45: display.setFont(&FreeSerifBoldItalic12pt7b); break;
    case 46: display.setFont(&FreeSerifBoldItalic18pt7b); break;
    case 47: display.setFont(&FreeSerifBoldItalic24pt7b); break;
    case 48: display.setFont(&Org_01); break;
    case 49: display.setFont(&Picopixel); break;
    case 50: display.setFont(&TomThumb); break;
  }
  
  // Display "Brown" and "Bear" with proper spacing
  display.setCursor(5, startY);
  display.print("Brown");
  display.setCursor(5, startY + lineSpacing);
  display.print("Bear");
  
  // Update display
  Serial.print("Displaying: ");
  Serial.println(fontName);
  display.display();
}

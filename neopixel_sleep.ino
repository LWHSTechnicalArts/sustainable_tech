/*
  test_sleep_esp32s3.ino
  Feather ESP32-S3 — red for 5 seconds, deep sleep for 5 seconds, repeat
*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN         5
#define NUM_PIXELS      16
#define PIXEL_GATE_PIN  6   // P-FET gate: LOW = ring on, HIGH = ring off

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Gate on
  pinMode(PIXEL_GATE_PIN, OUTPUT);
  digitalWrite(PIXEL_GATE_PIN, LOW);
  delay(5);

  // Turn off onboard status NeoPixel power
  #if defined(NEOPIXEL_POWER)
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
  #endif

  // Red for 5 seconds
  pixels.begin();
  pixels.setBrightness(80);
  pixels.fill(pixels.Color(255, 0, 0));
  pixels.show();
  delay(5000);

  // Gate off
  pixels.clear();
  pixels.show();
  delay(2);
  digitalWrite(PIXEL_GATE_PIN, HIGH);

  // Deep sleep 5 seconds
  esp_sleep_enable_timer_wakeup(5000000ULL); // 5 seconds in microseconds
  esp_deep_sleep_start();
}

void loop() {}

/*
 * ESP32-S3 Feather + PIR Motion Sensor Deep Sleep Example
 * 
 * This sketch demonstrates:
 * - Entering deep sleep to conserve battery
 * - Waking up when motion is detected on a PIR sensor
 * - Running code briefly when awake, then sleeping again
 * 
 * Wiring:
 * PIR sensor OUT pin -> GPIO 4 (RTC GPIO)
 * PIR sensor VCC -> 3.3V
 * PIR sensor GND -> GND
 * 
 * Note: GPIO 4 is an RTC GPIO pin that can wake the ESP32
 */

#define PIR_PIN 4  // RTC GPIO pin that triggers the wake

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give serial time to connect
  
  Serial.println("\n\nESP32-S3 PIR Wake-up Example");
  
  // Configure the PIR pin as input
  pinMode(PIR_PIN, INPUT);
  
  // Print wake-up reason
  printWakeupReason();
  
  // Do something briefly when awake
  Serial.println("Device is awake!");
  Serial.println("Motion detected or just powered on");
  
  // Simulate some work (blink LED, take sensor reading, etc.)
  delay(1000);
  
  Serial.println("Going back to sleep...\n");
  delay(100);  // Brief delay to ensure serial message is sent
  
  // Enter deep sleep
  goToDeepSleep();
}

void loop() {
  // Never reaches here because we go to sleep in setup()
}

void goToDeepSleep() {
  /*
   * Configure external wake-up:
   * - gpio_num: which pin triggers the wake
   * - level: LOW (0) wakes on low signal, HIGH (1) wakes on high signal
   * 
   * For PIR: HIGH means motion detected, so we wake on HIGH
   * (though some PIR sensors may be different—check your datasheet)
   */
  
  esp_sleep_enable_ext0_wakeup(PIR_PIN, HIGH);
  
  // Alternative: if you need multiple pins, use ext1:
  // esp_sleep_enable_ext1_wakeup((1ULL << PIR_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);
  
  // Optional: configure what happens in sleep mode
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  
  // Enter deep sleep (CPU and most peripherals shut down)
  Serial.println("Entering deep sleep...");
  esp_deep_sleep_start();
  
  // Code after this never executes until wake-up resets the chip
}

void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal on RTC_GPIO");
      Serial.println("-> Motion detected on PIR sensor!");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
  }
}

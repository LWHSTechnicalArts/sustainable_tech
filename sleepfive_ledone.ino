//lights led for one second, then goes to deep sleep for five. After five seconds the timer restarts the micro


#define uS_TO_S_FACTOR 1000000ULL  // Microseconds to seconds conversion
#define SLEEP_TIME 5                // Sleep for 5 seconds
#define LED_TIME 1000              // LED on for 1000ms (1 seconds)

// For Adafruit ESP32-S3 Feather - built-in led on PIN 13
#define LED_PIN 13

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("ESP32-S3 Sleep/LED Cycle Starting...");
  Serial.println("Sleep 5s -> LED 1s -> Repeat");
  
  // Turn on LED first
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON for 1 second");
  delay(LED_TIME);
  
  // Turn off LED before sleep
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF - Going to sleep for 5 seconds");
  Serial.flush();
  
  // Configure timer wakeup
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);
  
  // Go to deep sleep
  esp_deep_sleep_start();
}

void loop() {
  // This will never run - deep sleep restarts the program
}

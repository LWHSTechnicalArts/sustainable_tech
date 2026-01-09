//Adafruit Feather ESP32-S3 Deep Sleep with Button Wake 
#define BUTTON_PIN 5 

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==Deep Sleep Button Wake==");
  // Check what caused the wake-up
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Woke up by button press!");
      break;
    default:
      Serial.println("Power-on or reset");
      break;
  }
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(A0, OUTPUT);  // Set A0 as output
  
  // Read and display current button state
  int pinState = digitalRead(BUTTON_PIN);
  Serial.print("Button state: ");
  Serial.println(pinState ? "HIGH (not pressed)" : "LOW (pressed)");
  
  // Stay awake for a bit to see serial output
  Serial.println("\nStaying awake for 5 seconds...");
  digitalWrite(A0,HIGH);
  delay(5000);
  digitalWrite(A0,LOW);
  
  // Configure wake-up sources
  Serial.println("Configuring deep sleep...");
  
  // CRITICAL: Hold the pullup state during deep sleep
  gpio_hold_en(GPIO_NUM_5);
  gpio_deep_sleep_hold_en();
  
  // Wake when pin goes LOW (button pressed)
  esp_sleep_enable_ext1_wakeup(1ULL << BUTTON_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
  
  Serial.println("Going to deep sleep now...");
  Serial.println("Press button to wake up!\n");
  delay(100);
  
  esp_deep_sleep_start();
}

void loop() {
  // Never executes
}

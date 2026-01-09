//Adafruit Feather ESP32-S3 Deep Sleep with PIR Wake 
#define PIR_PIN 6  // Changed from 5 to 6

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==Deep Sleep PIR Wake==");
  
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Woke up by PIR motion detected!");
      break;
    default:
      Serial.println("Power-on or reset");
      Serial.println("Waiting 10 seconds for PIR to initialize...");
      delay(10000);
      break;
  }
  
  pinMode(PIR_PIN, INPUT);
  pinMode(A0, OUTPUT);
  
  int pirState = digitalRead(PIR_PIN);
  Serial.print("PIR state: ");
  Serial.println(pirState ? "HIGH (motion detected)" : "LOW (no motion)");
  
  Serial.println("\nTurning on A0 for 5 seconds...");
  digitalWrite(A0, HIGH);
  delay(5000);
  digitalWrite(A0, LOW);
  Serial.println("A0 turned off");
  
  Serial.println("Waiting for PIR to clear...");
  unsigned long timeout = millis() + 10000;
  while(digitalRead(PIR_PIN) == HIGH && millis() < timeout) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  
  delay(2000);
  
  Serial.println("Configuring deep sleep...");
  
  // Wake when pin goes HIGH (motion detected)
  esp_sleep_enable_ext1_wakeup(1ULL << PIR_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
  
  Serial.println("Going to deep sleep now...");
  Serial.println("Wave hand in front of PIR to wake up!\n");
  delay(100);
  
  esp_deep_sleep_start();
}

void loop() {
  // Never executes
}

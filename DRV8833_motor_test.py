// ESP32-S3 Feather + Adafruit DRV8833 Motor Driver
 
#include <Arduino.h>

// Motor A pins
#define MOTOR_A_IN1 9
#define MOTOR_A_IN2 10

// Motor B pins
#define MOTOR_B_IN1 11
#define MOTOR_B_IN2 12

// Sleep pin (optional, for power control)
#define MOTOR_SLEEP_PIN 6

// PWM parameters
#define PWM_FREQ 5000      // 5 kHz PWM frequency
#define PWM_RESOLUTION 8   // 8-bit (0-255)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32-S3 + DRV8833 Motor Test");
  
  // Configure motor control pins
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  pinMode(MOTOR_SLEEP_PIN, OUTPUT);
  
  // Wake up the motor driver (set NSLP HIGH)
  digitalWrite(MOTOR_SLEEP_PIN, HIGH);
  
  // Configure PWM on pins (new ESP32-S3 API)
  ledcAttach(MOTOR_A_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_A_IN2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_B_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_B_IN2, PWM_FREQ, PWM_RESOLUTION);
  
  // Stop all motors initially
  stopAllMotors();
  
  Serial.println("Motor driver initialized");
}

void loop() {
  // Test sequence
  Serial.println("\nMotor A forward at 75% speed");
  motorA_forward(192);  // 192/255 ≈ 75%
  delay(2000);
  
  Serial.println("Motor A reverse at 50% speed");
  motorA_reverse(128);  // 128/255 ≈ 50%
  delay(2000);
  
  Serial.println("Motor A stop");
  motorA_stop();
  delay(1000);
  
  Serial.println("Motor B forward at full speed");
  motorB_forward(255);
  delay(2000);
  
  Serial.println("Motor B stop");
  motorB_stop();
  delay(2000);
  
  Serial.println("Both motors forward");
  motorA_forward(200);
  motorB_forward(200);
  delay(2000);
  
  stopAllMotors();
  delay(1000);
  
  // Put motor driver to sleep for 5 seconds
  sleepMotorDriver();
  delay(5000);
  
  // Wake motor driver back up
  wakeMotorDriver();
  delay(2000);
}

// ===== MOTOR A FUNCTIONS =====

void motorA_forward(uint8_t speed) {
  /*
   * For forward: AIN1 = HIGH (with PWM), AIN2 = LOW
   * speed: 0-255 (0 = stop, 255 = full speed)
   */
  ledcWrite(MOTOR_A_IN1, speed);
  ledcWrite(MOTOR_A_IN2, 0);
}

void motorA_reverse(uint8_t speed) {
  /*
   * For reverse: AIN1 = LOW, AIN2 = HIGH (with PWM)
   */
  ledcWrite(MOTOR_A_IN1, 0);
  ledcWrite(MOTOR_A_IN2, speed);
}

void motorA_stop() {
  /*
   * To stop: both pins LOW
   */
  ledcWrite(MOTOR_A_IN1, 0);
  ledcWrite(MOTOR_A_IN2, 0);
}

// ===== MOTOR B FUNCTIONS =====

void motorB_forward(uint8_t speed) {
  ledcWrite(MOTOR_B_IN1, speed);
  ledcWrite(MOTOR_B_IN2, 0);
}

void motorB_reverse(uint8_t speed) {
  ledcWrite(MOTOR_B_IN1, 0);
  ledcWrite(MOTOR_B_IN2, speed);
}

void motorB_stop() {
  ledcWrite(MOTOR_B_IN1, 0);
  ledcWrite(MOTOR_B_IN2, 0);
}

// ===== UTILITY FUNCTIONS =====

void stopAllMotors() {
  motorA_stop();
  motorB_stop();
}

void sleepMotorDriver() {
  /*
   * Disable the motor driver before deep sleep
   * This cuts power draw to microamps
   */
  Serial.println("Putting motor driver to sleep...");
  digitalWrite(MOTOR_SLEEP_PIN, LOW);  // NSLP pin LOW = sleep mode
}

void wakeMotorDriver() {
  /*
   * Wake up the motor driver after sleep
   */
  Serial.println("Waking motor driver...");
  digitalWrite(MOTOR_SLEEP_PIN, HIGH);  // NSLP pin HIGH = active
  delay(100);  // Brief stabilization time
}

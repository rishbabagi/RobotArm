#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Servo.h>
#include <EEPROM.h>

// Joystick pins
#define JOY1_X_PIN A6
#define JOY1_Y_PIN A7
#define JOY1_BUTTON_PIN 7
#define JOY2_X_PIN A1
#define JOY2_BUTTON_PIN 8
#define JOY3_X_PIN A2
#define JOY3_BUTTON_PIN 9
#define JOY4_X_PIN A0
#define JOY4_BUTTON_PIN 10

// Ultrasonic sensor
#define ECHO_PIN 12
#define TRIG_PIN 13

// EEPROM
#define EE_SIZE_ADDRESS 0
#define EE_START_ADDRESS 2

// Joystick neutral correction
#define ANALOG_X_CORRECTION 128
#define ANALOG_Y_CORRECTION 128

// Servos
Servo arm_servos[5];
int pos[5] =        {100, 90, 90, 90, 90};
int min_pos[5] =    {92, 0, 0, 80, 0};
int max_pos[5] =    {140, 180, 180, 200, 160};
int servo_pins[5] = {2, 3, 4, 5, 6};

int ee_size = 1;
int ee_address = EE_START_ADDRESS;

long duration;
int distance = 0;

// LCD
hd44780_I2Cexp lcd;

// Structs
struct button {
  byte pressed = 0;
};

struct analog {
  int x, y;
  button btn;
};

analog joy1, joy2, joy3, joy4;

// -----------------------------
// Utility Functions
// -----------------------------

byte readAnalogAxisLevel(int pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

bool isPressed(int pin) {
  return digitalRead(pin) == 0;
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0) return -1;

  int dist = (duration * 0.034 / 2);
  return dist;
}

// -----------------------------
// Setup
// -----------------------------

void setup() {
  pinMode(JOY1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY2_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY3_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY4_BUTTON_PIN, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.backlight();

  // Attach servos
  for (int i = 0; i < 5; i++) {
    arm_servos[i].attach(servo_pins[i]);
    arm_servos[i].write(pos[i]);
    delay(300);
  }

  // Load saved step count
  EEPROM.get(EE_SIZE_ADDRESS, ee_size);
  if (ee_size < 1 || ee_size > 200) {
    ee_size = 1;
  }

  ee_address = EE_START_ADDRESS;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manual Mode");
}

// -----------------------------
// Main Loop
// -----------------------------

void loop() {
  distance = getDistance();

  // Read joysticks
  joy1.x = readAnalogAxisLevel(JOY1_X_PIN) - ANALOG_X_CORRECTION;
  joy1.y = readAnalogAxisLevel(JOY1_Y_PIN) - ANALOG_Y_CORRECTION;
  joy1.btn.pressed = isPressed(JOY1_BUTTON_PIN);

  joy2.x = readAnalogAxisLevel(JOY2_X_PIN) - ANALOG_X_CORRECTION;
  joy2.btn.pressed = isPressed(JOY2_BUTTON_PIN);

  joy3.x = readAnalogAxisLevel(JOY3_X_PIN) - ANALOG_X_CORRECTION;
  joy3.btn.pressed = isPressed(JOY3_BUTTON_PIN);

  joy4.x = readAnalogAxisLevel(JOY4_X_PIN) - ANALOG_X_CORRECTION;
  joy4.btn.pressed = isPressed(JOY4_BUTTON_PIN);

  // -----------------------------
  // Manual Servo Control
  // -----------------------------

  if (abs(joy1.x) > 10) {
    pos[0] += joy1.x / 20;
    pos[0] = constrain(pos[0], min_pos[0], max_pos[0]);
    arm_servos[0].write(pos[0]);
  }

  if (abs(joy1.y) > 10) {
    pos[4] += joy1.y / 20;
    pos[4] = constrain(pos[4], min_pos[4], max_pos[4]);
    arm_servos[4].write(pos[4]);
  }

  if (abs(joy2.x) > 10) {
    pos[3] -= joy2.x / 20;
    pos[3] = constrain(pos[3], min_pos[3], max_pos[3]);
    arm_servos[3].write(pos[3]);
  }

  if (abs(joy3.x) > 10) {
    pos[2] -= joy3.x / 20;
    pos[2] = constrain(pos[2], min_pos[2], max_pos[2]);
    arm_servos[2].write(pos[2]);
  }

  if (abs(joy4.x) > 10) {
    pos[1] -= joy4.x / 20;
    pos[1] = constrain(pos[1], min_pos[1], max_pos[1]);
    arm_servos[1].write(pos[1]);
  }

  // -----------------------------
  // RESET ALL STEPS (Hold Replay 3 Seconds)
  // -----------------------------
  if (joy4.btn.pressed) {
    unsigned long holdStart = millis();

    while (isPressed(JOY4_BUTTON_PIN)) {
      if (millis() - holdStart >= 3000) {

        ee_size = 1;
        EEPROM.put(EE_SIZE_ADDRESS, ee_size);

        for (int addr = EE_START_ADDRESS; addr < 1024; addr++) {
          EEPROM.write(addr, 0);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Memory Cleared");
        lcd.setCursor(0, 1);
        lcd.print("Start New Steps");

        while (isPressed(JOY4_BUTTON_PIN)) delay(20);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Manual Mode");
        return;
      }
      delay(20);
    }
  }

  // -----------------------------
  // Save Step (with longer display)
  // -----------------------------
  if (joy1.btn.pressed) {

    // Save step data
    EEPROM.put(ee_address, pos);

    // Increment step count
    ee_size++;

    // Save updated step count
    EEPROM.put(EE_SIZE_ADDRESS, ee_size);

    // Move to next slot
    ee_address += 10;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Saved Step #");
    lcd.print(ee_size - 1);

    delay(800);  // show "Saved Step" longer

    while (isPressed(JOY1_BUTTON_PIN)) delay(20);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Manual Mode");
  }

  // -----------------------------
  // Replay Steps (Servo Relax Mode)
  // -----------------------------
  if (joy4.btn.pressed) {

    int new_pos[5];
    EEPROM.get(EE_SIZE_ADDRESS, ee_size);
    ee_address = EE_START_ADDRESS;

    for (int i = 0; i < ee_size; i++) {
      EEPROM.get(ee_address, new_pos);

      for (int s = 0; s < 5; s++) {

        // Move servo smoothly
        while (pos[s] != new_pos[s]) {
          pos[s] += (pos[s] < new_pos[s]) ? 1 : -1;
          arm_servos[s].write(pos[s]);
          delay(35);
        }

        // RELAX SERVO IMMEDIATELY
        arm_servos[s].detach();
        delay(80);  // let power recover
        arm_servos[s].attach(servo_pins[s]);
        arm_servos[s].write(pos[s]);

        delay(120);  // pause before next servo
      }

      ee_address += 10;
      delay(200);
    }

    // -----------------------------
    // FINAL DETACH AFTER REPLAY
    // -----------------------------
    for (int s = 0; s < 5; s++) {
      arm_servos[s].detach();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Replay Done");
    lcd.setCursor(0, 1);
    lcd.print("Servos Off");
    delay(1500);

    // Reattach safely
    for (int s = 0; s < 5; s++) {
      arm_servos[s].attach(servo_pins[s]);
      arm_servos[s].write(pos[s]);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Manual Mode");

    while (isPressed(JOY4_BUTTON_PIN)) delay(20);
  }

  delay(30);
}

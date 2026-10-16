# Robotic Arm (Arduino)

A 5-servo robotic arm controlled by four joysticks, with the ability to record a
sequence of positions and play them back automatically. Saved sequences are stored
in EEPROM, so they stay in memory even after the arm is unplugged.

## Features
- **Joystick control**: four analog joysticks drive five servo joints, with a
  dead zone to ignore stick drift and min/max limits on every joint to protect
  the hardware
- **Record steps**: press Joystick 1's button to save the arm's current pose as a step
- **Replay**: press Joystick 4's button to play back every saved step in order;
  servos move one degree at a time for smooth motion
- **Power management**: each servo is briefly detached after it reaches its target
  to reduce current draw and let the power supply recover
- **Persistent memory**: saved steps live in EEPROM and survive power cycles
- **Clear memory**: hold Joystick 4's button for 3 seconds to erase all saved steps
- **LCD status display**: a 16×2 I2C LCD shows the current mode, saved step
  numbers, and replay status

## Hardware
- Arduino (Nano or another board with A6/A7 analog pins)
- 5 × servo motors (pins 2–6)
- 4 × analog joysticks with push buttons
- 16×2 LCD with I2C backpack
- HC-SR04 ultrasonic sensor (pins 12/13)
- External power supply for the servos

## Libraries
- `Servo`, `EEPROM`, `Wire` (built into the Arduino IDE)
- [`hd44780`](https://github.com/duinoWitchery/hd44780) by Bill Perry
  (install it through the Library Manager)

## Getting Started
1. Clone the repo: `git clone https://github.com/rishbabagi/RobotArm.git`
2. Open `Junior_Robotics_Final_Project.ino` in the Arduino IDE
3. Install the `hd44780` library
4. Select your board and port, then upload

## Possible Improvements
- Use the ultrasonic sensor for obstacle detection or auto-stop
- Add speed control for replay
- Show live joint angles on the LCD

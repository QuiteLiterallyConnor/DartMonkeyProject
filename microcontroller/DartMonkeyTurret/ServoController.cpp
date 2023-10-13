#include "ServoController.h"

void ServoController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  servoPin = config["pin"];
  speed = config["speed"];
  angle_limit = config["angle_limit"];
  startingAngle = config["starting_angle"];
  servo.attach(servoPin);
  servo.setEasingType(EASE_CUBIC_IN_OUT);
  setAngle(startingAngle);
  delay(500);
  Serial.print("%%%_INFO:");
  Serial.print(name.c_str());
  Serial.print(": Ready to transmit Servo PWM signals at pin " );
  Serial.println(servoPin);
}

void ServoController::handleGcodeCommand(std::string cmd) {
  char actionType = cmd[1];
  int value = std::stoi(cmd.substr(2));
  if (actionType == 'S') {
    setAngle(value);
  } else if (actionType == 'O') {
    changeAngle(value);
  }
}

void ServoController::changeAngle(int delta) {
  setAngle(currentAngle + delta);
}

void ServoController::setAngle(int angle) {
  if (angle >= 0 && angle <= angle_limit) {
    currentAngle = angle;
    print();
    servo.easeTo(angle, speed);
    // servo.write(angle);
  }
}

int ServoController::getCurrentAngle() {
  return currentAngle;
}

std::string ServoController::getName() {
  return name;
}

void ServoController::print() {
    Serial.print("%%%_");
    Serial.print(name.c_str());
    Serial.print("_POS:");
    Serial.println(currentAngle);
}
#include "ServoController.h"

void ServoController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  servoPin = config["pin"];
  speed = config["speed"];
  angle_limit = config["angle_limit"];
  servo.attach(servoPin, config["starting_angle"]);

  servo.setEasingType(EASE_CUBIC_IN_OUT);

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
  int new_angle = currentAngle += delta;
  if (new_angle <= angle_limit && new_angle >= 0) {
      setAngle(new_angle);
  }
}

void ServoController::setAngle(int angle) {
  if (angle <= angle_limit && angle >= 0) {
    servo.easeTo(angle, speed);
    // servo.write(angle);
    currentAngle = angle;

  }
  print();
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
#include "ServoController.h"

void ServoController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  servoPin = config["pin"];
  servo.attach(servoPin);
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
  if (new_angle <= upper_angle_limit && new_angle >= lower_angle_limit) {
      setAngle(new_angle);
  }
  print();
}

void ServoController::setAngle(int angle) {
  if (angle <= upper_angle_limit && angle >= lower_angle_limit) {
    currentAngle = angle;
    servo.write(currentAngle);
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
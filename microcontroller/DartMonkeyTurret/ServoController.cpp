#include "ServoController.h"

void ServoController::initialize(std::string n, StaticJsonDocument<1024> config) {
  name = n;
  servoPin = config["pin"];
  speed = config["speed"];
  angle_limit = config["angle_limit"];
  startingAngle = config["starting_angle"];
  interpolation = config["interpolation"].as<std::string>();
  servo.attach(servoPin);

  if (interpolation == "linear") {
    servo.setEasingType(EASE_LINEAR);
  } else if (interpolation == "ease") {
    servo.setEasingType(EASE_CUBIC_IN_OUT);
  } else {
    servo.setEasingType(EASE_LINEAR);
  }

  setAngle(startingAngle);
  delay(500);
  std::string tmp;
  tmp += "%%%_INFO:";
  tmp += name;
  tmp += ": Ready to transmit Servo PWM signals at pin ";
  tmp += std::to_string(servoPin);
  
  Serial.println(tmp.c_str());
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
    servo.startEaseTo(angle, speed);
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
    std::string tmp;
    tmp += "%%%_";
    tmp += name;
    tmp += "_POS:";
    tmp += std::to_string(currentAngle);
    
    Serial.println(tmp.c_str());
}

#include "ServoController.h"

void ServoController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  servoPin = config["pin"];
  servo.attach(servoPin);    
  Serial.print("Ready to transmit Servo PWM signals at pin " );
  Serial.println(servoPin);
}

void ServoController::changeAngle(int delta) {
    Serial.println("changing angle");
    if (currentAngle < upper_angle_limit && currentAngle > lower_angle_limit) {
        currentAngle += delta;
        setAngle(currentAngle);
    }
  print();
}

void ServoController::setAngle(int angle) {
    currentAngle = angle;
    servo.write(angle);
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
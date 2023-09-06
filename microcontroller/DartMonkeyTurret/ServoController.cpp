#include "ServoController.h"

void ServoController::initialize(int pin) {
  servoPin = pin;
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
  Serial.println(currentAngle);
}

void ServoController::setAngle(int angle) {
    servo.write(angle);
}

int ServoController::getCurrentAngle() {
  return currentAngle;
}

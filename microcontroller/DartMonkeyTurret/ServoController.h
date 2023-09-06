#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#include <Arduino.h>
#include "Servo.h"

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

class ServoController {
public:
    ServoController() {}
    void initialize(int pin);
    void changeAngle(int delta);
    void setAngle(int angle);
    int getCurrentAngle();
private:
  Servo servo;
  int servoPin;
  int currentAngle = 0;
  int upper_angle_limit = 90;
  int lower_angle_limit = -90;
};

#endif
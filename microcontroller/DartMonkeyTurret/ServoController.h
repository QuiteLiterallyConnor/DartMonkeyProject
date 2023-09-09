#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#include <Arduino.h>
#include <string>
#include "Servo.h"
#include <ArduinoJson.h>

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

class ServoController {
public:
    ServoController() {}
    void initialize(std::string n, StaticJsonDocument<500> config);
    void changeAngle(int delta);
    void setAngle(int angle);
    int getCurrentAngle();
    std::string getName();
private:
  Servo servo;
  std::string name;
  int servoPin;
  int currentAngle = 0;
  int upper_angle_limit = 90;
  int lower_angle_limit = -90;
  void print();
};

#endif
#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>

#define SUPPRESS_HPP_WARNING
#include "EasingLib.h"

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

class ServoController {
public:
    ServoController() {}
    void initialize(std::string n, StaticJsonDocument<500> config);
    void handleGcodeCommand(std::string cmd);
    void changeAngle(int delta);
    void setAngle(int angle);
    int getCurrentAngle();
    std::string getName();
private:
  ServoEasing servo;
  // Servo servo;
  std::string name;
  int servoPin;
  int speed = 100;
  int currentAngle = 0;
  int angle_limit = 90;
  int startingAngle = 0;
  void print();
};

#endif
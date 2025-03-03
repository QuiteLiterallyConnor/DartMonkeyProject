#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>

#define SUPPRESS_HPP_WARNING
#include "EasingLib.h"

class ServoController {
public:
    ServoController() {}
    void Init(StaticJsonDocument<1024> config);
    void handleGcodeCommand(std::string cmd);
    void changeAngle(int delta);
    void setAngle(int angle);
    int getCurrentAngle();
    std::string getName();
private:
  ServoEasing servo;
  std::string name;
  std::string interpolation;
  int servoPin;
  int speed = 100;
  int currentAngle = 0;
  int angle_min = 0;
  int angle_max = 0;
  int startingAngle = 0;
  void print();
};

#endif
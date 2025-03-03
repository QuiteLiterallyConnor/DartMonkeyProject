#ifndef ESCCONTROLLER_H
#define ESCCONTROLLER_H

#include <Arduino.h>
#include <string>
#include <iostream>
#include <ArduinoJson.h>

#define SUPPRESS_HPP_WARNING
#include "EasingLib.h"

class ESCController {
public:
    ESCController() {}
    void Init(StaticJsonDocument<1024> config);
    void sync();
    void handleGcodeCommand(std::string cmd);
    int getCurrentSpeed() const;
    void togglePower();
    void setSpeed(int speed);
    void offsetSpeed(int delta);

private:
    std::string name;
    ServoEasing controller;
    // Servo controller;
    int controllerPin;
    int currentSpeed;
    int prevSpeed;

    const int STOP_PWM = 1490;
    const int IDLE_PWM = 1500;
    const int LOWER_PWM_LIMIT = 1530;
    const int UPPER_PWM_LIMIT = 1900;

    int speedToPulseWidth(int speed) const;
    void print();
};

#endif
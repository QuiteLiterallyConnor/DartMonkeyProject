#ifndef ESCCONTROLLER_H
#define ESCCONTROLLER_H

#include <Arduino.h>
#include <iostream>
#include "Servo.h"

class ESCController {
public:
    ESCController() {}
    void initialize(int pin);
    void sync();
    int getCurrentSpeed() const;
    void stop();
    void togglePower();
    void setSpeed(int speed);
    void changeSpeed(int delta);

private:
    Servo controller;
    int controllerPin;
    int currentSpeed;
    int prevSpeed;

    const int STOP_PWM = 1490;
    const int IDLE_PWM = 1500;
    const int LOWER_PWM_LIMIT = 1530;
    const int UPPER_PWM_LIMIT = 1900;

    int speedToPulseWidth(int speed) const;
};

#endif
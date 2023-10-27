#ifndef RELAYCONTROLLER_H
#define RELAYCONTROLLER_H

#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>

class RelayController {
public:
    RelayController() {}
    void initialize(std::string n, StaticJsonDocument<500> config);
    void handleGcodeCommand(std::string cmd);
    void setOn();
    void setOff();
    void toggle();
    bool state();
    std::string getName();

private:
    std::string name;
    int relayPin;
    bool currentState = false; // By default, relay is off
    void printState();
};

#endif

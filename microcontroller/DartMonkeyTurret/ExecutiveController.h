#ifndef EXECUTIVECONTROLLER_H
#define EXECUTIVECONTROLLER_H

#include <algorithm>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <ESP32Time.h>  
#include <vector>
#include "esp_system.h"

#include "SerialController.h"
#include "ServoController.h"
#include "ESCController.h"

void adam();
void init_controllers();
std::map<std::string, SerialController::Command> initSerialCmdMap();

class ExecutiveController {
public:
    int initialize();
    void Reset();
    void ExecuteSerialCommands();
private:
    int loadConfig();
    const char* getConfigJsonString();
    bool isValidCommand(const std::string& cmd);
    void handleCommand(const std::string& cmd);
    StaticJsonDocument<1024> doc;
};

#endif

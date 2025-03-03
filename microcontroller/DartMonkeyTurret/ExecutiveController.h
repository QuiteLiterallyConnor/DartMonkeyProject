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
#include <vector>
#include "esp_system.h"

#include "SerialController.h"
#include "ServoController.h"
#include "ESCController.h"

class ExecutiveController {
public:
    int Init();
    void GetCommands();
    void ExecuteCommands();
    
private:
    void loadDevices();
    const char* getConfigJsonString();
    bool isValidCommand(const std::string& cmd);
    void handleCommand(const std::string& cmd);
    StaticJsonDocument<1024> doc;
    std::map<std::string, ServoController> servoControllers;
    std::map<std::string, ESCController> escControllers;
};

#endif

#ifndef SERIALCONTROLLER_H
#define SERIALCONTROLLER_H

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
#include <TimeLib.h>
#include <vector>

class SerialController {
public:

    struct Command {
        std::string description;
        std::function<bool(std::string cmd)> action;
    };

    SerialController() {}
    void Init();
    void ReadSerial();
    bool GetCommandBuffer(std::vector<std::string> &buf);
    std::vector<std::string> commandBuffer;
private:
    std::string inputBuffer;
    unsigned long lastCheckTime;
    const unsigned long checkInterval = 50;
    void processSerialInput();
};

#endif

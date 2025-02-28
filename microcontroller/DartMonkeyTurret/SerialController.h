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
    void initialize(std::map<std::string, SerialController::Command> cmd);
    void handleSerial();
    bool GetCommandBuffer(std::vector<std::string> &buf);
    std::map<std::string, Command> GetCommandMap();
    std::vector<std::string> commandBuffer;
    std::map<std::string, Command> commandMap;
private:
    std::string inputBuffer;
    unsigned long lastCheckTime;
    const unsigned long checkInterval = 50;
    void setupCommands();
    void processSerialInput();
    void handleCommand(const std::string& cmd);
    void handleMetaCommand(const std::string& cmd);
    void wait(std::string cmd);
    void initializeCommandMap();
};

#endif

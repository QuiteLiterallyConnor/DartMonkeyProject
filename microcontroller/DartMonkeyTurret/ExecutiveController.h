#ifndef EXECUTIVECONTROLLER_H
#define EXECUTIVECONTROLLER_H

#include <algorithm>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <TeensyThreads.h>
#include <TimeLib.h>
#include <vector>

#include "ServoController.h"
#include "ESCController.h"
#include "RelayController.h"

#define IR_INPUT_PIN              1
#define ESC_OUTPUT_PIN            22
#define X_ROTATION_PIN            5
#define Y_ROTATION_PIN            6
#define ROTOR_CONTROL_MOTOR_1_PIN 7
#define ROTOR_CONTROL_MOTOR_2_PIN 8

void adam();
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aButton, bool isRepeat);
void init_controllers();

struct DateTime {
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
    int Second;
};

class SerialController {
public:

    struct Command {
        std::string description;
        std::function<bool(std::string cmd)> action;
    };

    SerialController() {}
    void initialize(std::map<std::string, Command> cmdMap);
    void handleSerial();
    std::map<std::string, SerialController::Command> initializeCommandMap();
private:
    std::string inputBuffer;
    unsigned long lastCheckTime;
    const unsigned long checkInterval = 50;
    std::map<std::string, Command> commandMap;
    void setupCommands();
    void processSerialInput();
    bool isValidCommand(const std::string& cmd);
    void handleCommand(const std::string& cmd);
    void handleMetaCommand(const std::string& cmd);
    void wait(std::string cmd);
};


class ExecutiveController {
public:
    int initialize();
    void Reset();
    std::string GetSessionKey();
    void PrintSessionKey();
    void SetSystemTime(std::string coded_string);
    void RequestServerTime();
    void HandleGcodeCommand(std::string cmd);
    void PrintHeartbeat();
private:
    void storeTransmittedMessage(const char * msg);
    void setSessionKey(std::string key);
    DateTime decodeUnixTimestampString(const std::string& unixTimestampStr);
    std::string generateSessionKey();
    char randomChar();
    std::string convertToHex(const std::string &input);
    int loadConfig();
    const char* getConfigJsonString();
    StaticJsonDocument<512> doc;
    std::string sessionKey;
    std::vector<std::string> transmittedMessages;
};

#endif

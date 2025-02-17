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
#include "RelayController.h"

#define IR_INPUT_PIN              1
#define ESC_OUTPUT_PIN            22
#define X_ROTATION_PIN            5
#define Y_ROTATION_PIN            6
#define ROTOR_CONTROL_MOTOR_1_PIN 7
#define ROTOR_CONTROL_MOTOR_2_PIN 8

// Remove AVR-specific include for ESP32
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#endif

void adam();
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aButton, bool isRepeat);
void init_controllers();
std::map<std::string, SerialController::Command> initSerialCmdMap();

struct DateTime {
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
    int Second;
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
    void ExecuteSerialCommands();
private:
    void storeTransmittedMessage(const char * msg);
    void setSessionKey(std::string key);
    DateTime decodeUnixTimestampString(const std::string& unixTimestampStr);
    std::string generateSessionKey();
    char randomChar();
    std::string convertToHex(const std::string &input);
    int loadConfig();
    const char* getConfigJsonString();
    bool isValidCommand(const std::string& cmd);
    void handleCommand(const std::string& cmd);
    StaticJsonDocument<1024> doc;
    std::string sessionKey;
    std::vector<std::string> transmittedMessages;
};

#endif

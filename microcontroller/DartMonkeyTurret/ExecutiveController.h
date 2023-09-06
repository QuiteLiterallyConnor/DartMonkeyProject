#ifndef EXECUTIVECONTROLLER_H
#define EXECUTIVECONTROLLER_H

#include <algorithm>
#include <Arduino.h>
#include <iostream>
#include <map>
#include <string>
#include <functional>

#include "ServoController.h"
#include "ESCController.h"

#define IR_INPUT_PIN              1
#define ESC_OUTPUT_PIN            22
#define X_ROTATION_PIN            5
#define Y_ROTATION_PIN            6
#define ROTOR_CONTROL_MOTOR_1_PIN 7
#define ROTOR_CONTROL_MOTOR_2_PIN 8

void adam();
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aButton, bool isRepeat);
void init_controllers();

class IRController {
public:
    IRController() {};
    struct Button {
        std::string buttonName;
        std::function<void()> action;
        bool allowsRepeat;
    };
    int controller_pin;
    std::map<int, Button> buttonMap;
    std::map<int, IRController::Button> initializeIRButtonMap();
    void initialize(int pin, std::map<int, Button> btnMap);
    void handleCommand(int btn, bool isRepeat);

private:
};


class SerialController {
public:
    struct Command {
        std::string description;
        std::function<bool(int)> action;  // Note the int argument for value
        bool allowsRepeat;
        bool canBeNegative;
    };
    SerialController() {}
    void initialize(std::map<std::string, Command> cmdMap);
    void handleSerial();
    std::map<std::string, SerialController::Command> initializeCommandMap();
private:
    std::string inputBuffer;
    std::string lastCommand;
    unsigned long lastCheckTime;
    const unsigned long checkInterval = 50;
    std::map<std::string, Command> commandMap;
    void setupCommands();
    void processSerialInput();
    bool isValidCommand(const std::string& cmd);
    void handleCommand(const std::string& cmd);
    void handleMetaCommand(const std::string& cmd);
};

#endif

#include "ExecutiveController.h"
#include "TinyIRReceiver.hpp"

ESCController motorController;
ServoController xRotationController;
ServoController yRotationController;
ServoController motorAServoController;
ServoController motorBServoController;
IRController irController;
SerialController serialController;

void print_status() {
  Serial.println("%%%_DEVICE_STATUS");
  Serial.print("X_SERVO_POS:");
  Serial.println(xRotationController.getCurrentAngle());
  Serial.print("Y_SERVO_POS:");
  Serial.println(yRotationController.getCurrentAngle());
  Serial.print("MOTOR_A_SERVO_POS:");
  Serial.println(motorAServoController.getCurrentAngle());
  Serial.print("MOTOR_B_SERVO_POS:");
  Serial.println(motorBServoController.getCurrentAngle());
  Serial.print("MOTOR_SPEED:");
  Serial.println(motorController.getCurrentSpeed());
  Serial.println("%%%");
}

void adam() {
  serialController.handleSerial();
}

std::map<int, IRController::Button> IRController::initializeIRButtonMap() {
    return {
      {69,  {"Power button",  [&]() { motorController.togglePower();      return false; }}},
      {70,  {"Vol +",         [&]() { xRotationController.setAngle(5);      return true; }}},
      {71,  {"Func/Stop",     [&]() { [](){};                                   return false; }}},
      {68,  {"Rewind",        [&]() { yRotationController.setAngle(-5);     return true; }}},
      {64,  {"Pause/Play",    [&]() { [](){};                                   return false; }}},
      {67,  {"Fast Forward",  [&]() { yRotationController.setAngle(5);      return true; }}},
      {7,   {"Down Arrow",    [&]() { motorController.changeSpeed(-10);       return true; }}},
      {9,   {"Up Arrow",      [&]() { motorController.changeSpeed(10);        return true; }}},
      {21,  {"Vol -",         [&]() { xRotationController.setAngle(-5);    return false; }}},
      {22,  {"0",             [&]() { motorController.setSpeed(0);          return false; }}},
      {25,  {"EQ",            [&]() { [](){};                             return false; }}},
      {13,  {"ST/REPT",       [&]() { motorController.sync();             return false; }}},
      {12,  {"1",             [&]() { motorController.setSpeed(10);       return false; }}},
      {24,  {"2",             [&]() { motorController.setSpeed(20);       return false; }}},
      {94,  {"3",             [&]() { motorController.setSpeed(30);       return false; }}},
      {8,   {"4",             [&]() { motorController.setSpeed(40);       return false; }}},
      {28,  {"5",             [&]() { motorController.setSpeed(50);       return false; }}},
      {90,  {"6",             [&]() { motorController.setSpeed(60);       return false; }}},
      {66,  {"7",             [&]() { motorController.setSpeed(70);       return false; }}},
      {82,  {"8",             [&]() { motorController.setSpeed(80);       return false; }}},
      {74,  {"9",             [&]() { motorController.setSpeed(100);       return false; }}}
    }; 
}

std::map<std::string, SerialController::Command> SerialController::initializeCommandMap() {
    return {
        {"X",  {"X axis",     [&](int value) { xRotationController.setAngle(value);            return true;  }, true, true  }},
        {"Y",  {"Y axis",     [&](int value) { yRotationController.setAngle(value);            return true;  }, true, true  }},
        {"A",  {"A servo",    [&](int value) { motorAServoController.setAngle(value);          return false; }, false, true }},  // You might need to define aServoController and motorAServoController
        {"B",  {"B servo",    [&](int value) { motorBServoController.setAngle(value);          return false; }, false, true }},
        {"S",  {"Speed",      [&](int value) { motorController.setSpeed(value);                return true;  }, true, false }},
        {"D",  {"Delay",      [&](int value) { delay(value);                                   return true;  }, true, false }},
        {"P",  {"Ping",       [&](int value) { Serial.println("pong");                         return true;  }, true, false }},
        {"H",  {"Heartbeat",  [&](int value) { print_status();                                 return true;  }, true, false }}
    };
}

void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aButton, bool isRepeat) {
    int btn = static_cast<int>(aButton);
    irController.handleCommand(btn, isRepeat);
}

void init_controllers() {
    xRotationController.initialize(X_ROTATION_PIN);
    yRotationController.initialize(Y_ROTATION_PIN);
    motorAServoController.initialize(ROTOR_CONTROL_MOTOR_1_PIN);
    motorBServoController.initialize(ROTOR_CONTROL_MOTOR_2_PIN);
    motorController.initialize(ESC_OUTPUT_PIN);
    irController.initialize(IR_INPUT_PIN, irController.initializeIRButtonMap());
    serialController.initialize(serialController.initializeCommandMap());
}

void IRController::initialize(int pin, std::map<int, Button> btnMap) {
    initPCIInterruptForTinyReceiver();
    controller_pin = pin;
    buttonMap = btnMap;
    Serial.print("Ready to receive NEC IR signals at pin ");
    Serial.println(IR_INPUT_PIN);
}

void IRController::handleCommand(int btn, bool isRepeat) {
    if (buttonMap.find(btn) != buttonMap.end()) {
        if (buttonMap.at(btn).allowsRepeat || !isRepeat) {
            buttonMap.at(btn).action();
        }
    }
}

void SerialController::initialize(std::map<std::string, Command> cmdMap) {
  commandMap = cmdMap;
  Serial.println("Ready to receive serial commands");
}

void SerialController::handleSerial() {
    if (millis() - lastCheckTime >= checkInterval) {
        processSerialInput();
        lastCheckTime = millis();
    }
}

void SerialController::processSerialInput() {
    while (Serial.available()) {
        char incomingChar = Serial.read();
        inputBuffer += incomingChar;

        if (incomingChar == '\n' || incomingChar == '\r') {
            inputBuffer.erase(std::remove(inputBuffer.begin(), inputBuffer.end(), '\n'), inputBuffer.end());
            inputBuffer.erase(std::remove(inputBuffer.begin(), inputBuffer.end(), '\r'), inputBuffer.end());
            Serial.print("Received command: ");
            Serial.println(inputBuffer.c_str());
            if (isValidCommand(inputBuffer)) {
                handleCommand(inputBuffer);
            } else {
                Serial.println("Error: Invalid command received?!");
            }
            inputBuffer = "";
        }
    }
}

bool SerialController::isValidCommand(const std::string& command) {
    if (command.empty()) return false;
    if (command == "H") return true;
    char commandType = command[0];
    std::string valueString = command.substr(1);
    auto cmdMap = initializeCommandMap();
    if (cmdMap.find(std::string(1, commandType)) == cmdMap.end()) {
        return false;
    }
    int value = std::stoi(valueString); // Will throw if not a valid integer
    if (valueString.find('.') != std::string::npos) {
        return false;
    }
    bool canBeNegative = cmdMap[std::string(1, commandType)].canBeNegative;
    if (value < 0 && !canBeNegative) {
        return false;
    }
    return true;
}

void SerialController::handleCommand(const std::string& cmd) {
    std::string label = cmd.substr(0, 1);
    int value = atoi(cmd.substr(1).c_str());
    Command& command = commandMap[label];
    if (lastCommand != label || (lastCommand == label && command.allowsRepeat)) {
        command.action(value);
        lastCommand = label;
    }
}

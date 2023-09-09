#include "ExecutiveController.h"
#include "TinyIRReceiver.hpp"

ExecutiveController executiveController;
ESCController motorController;
ServoController xRotationController;
ServoController yRotationController;
ServoController motorAServoController;
ServoController motorBServoController;
IRController irController;
SerialController serialController;

void print_status() {
  Serial.print("%%%_X_SERVO_POS:");
  Serial.println(xRotationController.getCurrentAngle());
  Serial.print("%%%_Y_SERVO_POS:");
  Serial.println(yRotationController.getCurrentAngle());
  Serial.print("%%%_MOTOR_A_SERVO_POS:");
  Serial.println(motorAServoController.getCurrentAngle());
  Serial.print("%%%_MOTOR_B_SERVO_POS:");
  Serial.println(motorBServoController.getCurrentAngle());
  Serial.print("%%%_MOTOR_SPEED:");
  Serial.println(motorController.getCurrentSpeed());
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
        {"H",  {"Heartbeat",  [&](int value) { print_status();                                 return true;  }, true, false }}
    };
}

void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aButton, bool isRepeat) {
    int btn = static_cast<int>(aButton);
    irController.handleCommand(btn, isRepeat);
}

void init_controllers() {
  executiveController.initialize();
}

void IRController::initialize(int pin, std::map<int, Button> btnMap) {
    initPCIInterruptForTinyReceiver();
    controller_pin = pin;
    buttonMap = btnMap;
    Serial.print("Ready to receive NEC IR signals at pin ");
    Serial.println(controller_pin);
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

bool SerialController::isValidCommand(const std::string& cmd) {
    if (cmd.empty()) return false;
    if (cmd == "H") return true;
    char commandType = cmd[0];
    
    if (!isalpha(commandType) || commandType < 'A' || commandType > 'Z') {
        return false;
    }

    if (cmd.length() == 1) return false;
    std::string valueString = cmd.substr(1);
    auto cmdMap = initializeCommandMap();
    if (cmdMap.find(std::string(1, commandType)) == cmdMap.end()) {
        return false;
    }

    for (size_t i = 0; i < valueString.length(); ++i) {
        char c = valueString[i];
        if (!isdigit(c) && (c != '-' || i != 0)) return false;
    }

    int value = std::stoi(valueString);
    if (value == 0 && valueString != "0") return false;

    bool canBeNegative = cmdMap[std::string(1, commandType)].canBeNegative;
    if (value < 0 && !canBeNegative) return false;

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

void ExecutiveController::initialize() {
    if (loadConfig()) {
      Serial.println("Failed to read in config json.");
    }

    Serial.println("Finished loading config json.");
}

int ExecutiveController::loadConfig() {
    const char* jsonString = getConfigJsonString();
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
      return 1;
    }

    xRotationController.initialize("X_SERVO", doc["X_SERVO"]);
    yRotationController.initialize("Y_SERVO", doc["Y_SERVO"]);
    motorAServoController.initialize("MOTOR_A_SERVO", doc["MOTOR_A_SERVO"]);
    motorBServoController.initialize("MOTOR_B_SERVO", doc["MOTOR_B_SERVO"]);
    motorController.initialize("MOTORS", doc["MOTORS_ESC"]);
    irController.initialize(doc["IR_SENSOR"], irController.initializeIRButtonMap());
    serialController.initialize(serialController.initializeCommandMap());

    return 0;
}

const char* ExecutiveController::getConfigJsonString() {
    return R"json(
    {
      "X_SERVO": { "pin": 5 },
      "Y_SERVO": { "pin": 2 },
      "MOTOR_A_SERVO": { "pin": 3 },
      "MOTOR_B_SERVO": { "pin": 4 },
      "MOTORS_ESC": { "pin": 66 },
      "IR_SENSOR": { "pin": 6 }
    }

    )json";

}

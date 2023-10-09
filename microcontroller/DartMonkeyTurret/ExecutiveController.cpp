#include "ExecutiveController.h"

ExecutiveController executiveController;
ESCController motorAController;
ESCController motorBController;
ServoController xRotationController;
ServoController yRotationController;
ServoController motorAServoController;
ServoController motorBServoController;
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
  Serial.print("%%%_MOTOR_A_SPEED:");
  Serial.println(motorAController.getCurrentSpeed());
  Serial.print("%%%_MOTOR_B_SPEED:");
  Serial.println(motorBController.getCurrentSpeed());
}

void adam() {

  serialController.handleSerial();



}

std::map<std::string, SerialController::Command> SerialController::initializeCommandMap() {
    return {
        {"HR",  {"Hard Restart",  [&](std::string cmd) { executiveController.Reset();                            return true;  }}},
        {"X",  {"X axis",         [&](std::string cmd) { xRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"Y",  {"Y axis",         [&](std::string cmd) { yRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"AS",  {"A servo",       [&](std::string cmd) { motorAServoController.handleGcodeCommand(cmd);          return false; }}},
        {"BS",  {"B servo",       [&](std::string cmd) { motorBServoController.handleGcodeCommand(cmd);          return false; }}},
        {"AM",  {"A motor",       [&](std::string cmd) { motorBController.handleGcodeCommand(cmd);               return true;  }}},                                                    
        {"BM",  {"B motor",       [&](std::string cmd) { motorBController.handleGcodeCommand(cmd);               return true;  }}},
        {"H",  {"Heartbeat",      [&](std::string cmd) { Serial.println("%%%_HEARTBEAT"); print_status();        return true;  }}}
    };
}

volatile bool blink = false;
void blink_thread() {
  while (1) {
    if (blink) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      blink = false;
    }
  }
}

void blinkLED() {
  blink = true;
}

void init_controllers() {
  if (executiveController.initialize()) {
    Serial.println("%%%_ERR:EXEC_CONTROLLER:INIT_FAILED");
  }
}

void SerialController::initialize(std::map<std::string, Command> cmdMap) {
  commandMap = cmdMap;
  threads.addThread(blink_thread, 0);
  Serial.println("%%%_INFO:EXECUTIVE_CONTROLLER: Finished init");
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
            Serial.print("%%%_ACK:");
            Serial.println(inputBuffer.c_str());
            blinkLED();

            if (isValidCommand(inputBuffer)) {
                handleCommand(inputBuffer);
            } else {
                Serial.print("%%%_ERR:INVALID_CMD:");
                Serial.println(inputBuffer.c_str());
            }

            inputBuffer = "";
        }
    }
}

bool SerialController::isValidCommand(const std::string& cmd) {
    if (cmd.empty()) return false;
    if (cmd == "H" || cmd == "R") return true;
    char commandType = cmd[0];
    if (!isalpha(commandType) || commandType < 'A' || commandType > 'Z') {
        return false;
    }
    if (cmd.length() == 1) return false;
    char commandAction = cmd[1];
    if (!isalpha(commandAction) || commandAction < 'A' || commandAction > 'Z') {
        return false;
    }

    std::string valueString = cmd.substr(2);
    auto cmdMap = initializeCommandMap();
    if (cmdMap.find(std::string(1, commandType)) == cmdMap.end()) {
        return false;
    }

    for (size_t i = 0; i < valueString.length(); ++i) {
        char c = valueString[i];
        if (!isdigit(c) && (c != '-' || i != 0)) {
          return false;
        }
    }

    int value = std::stoi(valueString);
    if (value == 0 && valueString != "0") {
      return false;
    }

    return true;
}

void SerialController::handleCommand(const std::string& cmd) {
    std::string label = cmd.substr(0, 1);
    std::string stdcmd = cmd;
    Command& command = commandMap[label];
    command.action(stdcmd);
}

int ExecutiveController::initialize() {
    if (loadConfig()) {
      Serial.println("%%%_ERR:INVALID_CONFIG");
      return 1;
    }
    return 0;
}

void ExecutiveController::Reset() {
  SCB_AIRCR = 0x05FA0004; // Write a value to Application Interrupt and Reset Control Register
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
    motorAController.initialize("MOTOR_B", doc["MOTOR_B"]);
    motorBController.initialize("MOTOR_A", doc["MOTOR_A"]);
    serialController.initialize(serialController.initializeCommandMap());

    return 0;
}

const char* ExecutiveController::getConfigJsonString() {
    return R"json(
    {
      "X_SERVO": { "pin": 1, "speed": 30, "starting_angle": 45, "angle_limit": 120 },
      "Y_SERVO": { "pin": 2, "speed": 50, "starting_angle": 45, "angle_limit": 90 },
      "MOTOR_A_SERVO": { "pin": 3, "speed": 50, "starting_angle": 45, "angle_limit": 20 },
      "MOTOR_B_SERVO": { "pin": 4, "speed": 50, "starting_angle": 45, "angle_limit": 20 },
      "MOTOR_A": { "pin": 5 },
      "MOTOR_B": { "pin": 6 }
    }
    )json";
}

#include "ExecutiveController.h"

ExecutiveController executiveController;
SerialController serialController;
ESCController motorAController;
ESCController motorBController;
ServoController xRotationController;
ServoController yRotationController;
ServoController motorAServoController;
ServoController motorBServoController;

void adam() {

  serialController.handleSerial();
  executiveController.ExecuteSerialCommands();

}

void init_controllers() {
  if (executiveController.initialize()) {
    std::string tmp = "%%%_ERR:EXEC_CONTROLLER:INIT_FAILED";
    Serial.println(tmp.c_str());
  }

  serialController.initialize(initSerialCmdMap());

  
}

std::map<std::string, SerialController::Command> initSerialCmdMap() {
      return {
        {"X",  {"X axis",                         [&](std::string cmd) { xRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"Y",  {"Y axis",                         [&](std::string cmd) { yRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"A",  {"A motor servo",                  [&](std::string cmd) { motorAServoController.handleGcodeCommand(cmd);          return false; }}},
        {"B",  {"B motor servo",                  [&](std::string cmd) { motorBServoController.handleGcodeCommand(cmd);          return false; }}},
        {"C",  {"A motor",                        [&](std::string cmd) { motorAController.handleGcodeCommand(cmd);               return true;  }}},                                                    
        {"D",  {"B motor",                        [&](std::string cmd) { motorBController.handleGcodeCommand(cmd);               return true;  }}},
    };
}


int ExecutiveController::initialize() {
    std::string tmp = "%%%_INFO: EXECUTIVE CONTROLLER: STARTING SETUP";
    Serial.println(tmp.c_str());

    if (loadConfig()) {
      Serial.println("%%%_ERR:INVALID_CONFIG");
      return 1;
    }

    tmp = "%%%_INFO: EXECUTIVE CONTROLLER: FINISHED SETUP";
    Serial.println(tmp.c_str());
    return 0;
}


void ExecutiveController::ExecuteSerialCommands() {
    std::vector<std::string> cmds;

    if (serialController.commandBuffer.empty()) {
      return;
    } else {
        cmds = serialController.commandBuffer;
        serialController.commandBuffer.clear();

        for (const std::string& cmd : cmds) {
            if (!isValidCommand(cmd)) {
                std::string error_msg = "%%%_ERR:INVALID_CMD:" + cmd;
                Serial.println(error_msg.c_str());
            } else {
                handleCommand(cmd);
            }
        }


    }

}


bool ExecutiveController::isValidCommand(const std::string& cmd) {
    if (cmd.empty()) return false;
    char commandType = cmd[0];
    if (!isalpha(commandType) || commandType < 'A' || commandType > 'Z') {
        Serial.print("%%%_ERR:INVALID_COMMAND:Invalid first character");
        return false;
    }
    if (cmd.length() == 1) return false;
    char commandAction = cmd[1];
    if (!isalpha(commandAction) || commandAction < 'A' || commandAction > 'Z') {
        Serial.print("%%%_ERR:INVALID_COMMAND:Invalid second character");
        return false;
    }

    std::map<std::string, SerialController::Command> cmdMap = serialController.commandMap;

    if (cmdMap.find(std::string(1, commandType)) == cmdMap.end()) {
        Serial.print("%%%_ERR:INVALID_COMMAND:Command not found");
        return false;
    }

    std::string valueString = cmd.substr(2);
    for (size_t i = 0; i < valueString.length(); ++i) {
        char c = valueString[i];
        if (!isdigit(c) && (c != '-' || i != 0)) {
          Serial.print("%%%_ERR:INVALID_COMMAND:Invalid character in value");
          return false;
        }
    }

    // int value = std::stoi(valueString);

    return true;
}

void ExecutiveController::handleCommand(const std::string& cmd) {
    std::string label = cmd.substr(0, 1);
    std::string stdcmd = cmd;
    std::map<std::string, SerialController::Command> commandMap = serialController.commandMap;
    SerialController::Command& command = commandMap[label];
    command.action(stdcmd);
}

void ExecutiveController::Reset() {
  esp_restart();  // Use ESP32's restart function to reset the system
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
    motorAController.initialize("MOTOR_A", doc["MOTOR_A"]);
    motorBController.initialize("MOTOR_B", doc["MOTOR_B"]);

    return 0;
}


const char* ExecutiveController::getConfigJsonString() {
    return R"json(
    {
      "X_SERVO": { "pin": 32, "speed": 30, "starting_angle": 60, "angle_limit": 120, "interpolation": "ease" },
      "Y_SERVO": { "pin": 4, "speed": 50, "starting_angle": 45, "angle_limit": 100, "interpolation": "ease" },
      "MOTOR_A_SERVO": { "pin": 6, "speed": 50, "starting_angle": 0, "angle_limit": 25, "interpolation": "linear" },
      "MOTOR_B_SERVO": { "pin": 8, "speed": 50, "starting_angle": 20, "angle_limit": 25, "interpolation": "linear" },
      "MOTOR_A": { "pin": 10, "reversed": "true" },
      "MOTOR_B": { "pin": 15, "reversed": "false" },
      "MOTOR_A_RELAY": { "pin": 17 },
      "MOTOR_B_RELAY": { "pin": 19 }
    }
    )json";
}    


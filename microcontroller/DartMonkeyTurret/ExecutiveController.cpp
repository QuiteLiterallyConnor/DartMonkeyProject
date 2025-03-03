#include "ExecutiveController.h"

SerialController serialController;

int ExecutiveController::Init() {
  serialController.Init();
  std::string tmp = "%%%_INFO: EXECUTIVE CONTROLLER: STARTING SETUP";
  Serial.println(tmp.c_str());
  loadDevices();
  tmp = "%%%_INFO: EXECUTIVE CONTROLLER: FINISHED SETUP";
  Serial.println(tmp.c_str());
  return 0;
}

void ExecutiveController::GetCommands() {
  serialController.ReadSerial();
}

void ExecutiveController::ExecuteCommands() {
    std::vector<std::string> cmds;
    if (!serialController.GetCommandBuffer(cmds)) {
        return;
    }

    for (const std::string& cmd : cmds) {
        if (!isValidCommand(cmd)) {
            std::string error_msg = "%%%_ERR:INVALID_CMD:" + cmd;
            Serial.println(error_msg.c_str());
            continue;
        }
        handleCommand(cmd);
    }
}

void ExecutiveController::loadDevices() {
  const char* jsonString = getConfigJsonString();
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
      Serial.println("%%%_ERR:INVALID_CONFIG");
      return;
  }

  for (JsonObject servo : doc["servos"].as<JsonArray>()) {
      std::string enumKey = servo["enum"].as<std::string>();
      servoControllers[enumKey].Init(servo);
  }

  for (JsonObject motor : doc["motors"].as<JsonArray>()) {
      std::string enumKey = motor["enum"].as<std::string>();
      escControllers[enumKey].Init(motor);
  }
}

void ExecutiveController::handleCommand(const std::string& cmd) {
  std::string label = cmd.substr(0, 1);
  std::string stdcmd = cmd;

  if (servoControllers.find(label) != servoControllers.end()) {
      servoControllers[label].handleGcodeCommand(stdcmd);
  } else if (escControllers.find(label) != escControllers.end()) {
      escControllers[label].handleGcodeCommand(stdcmd);
  } else {
      Serial.print("%%%_ERR:INVALID_COMMAND:Command not found");
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

    if (servoControllers.find(std::string(1, commandType)) == servoControllers.end() &&
        escControllers.find(std::string(1, commandType)) == escControllers.end()) {
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

    return true;
}

const char* ExecutiveController::getConfigJsonString() {
    return R"json(
      {
        "servos": [
            {
                "name": "X_AXIS_SERVO",
                "enum": "X",
                "pin": 1,
                "speed": 50,
                "min": 100,
                "max": 180,
                "start": 140,
                "interpolation": "ease"
            },
            {
                "name": "Y_AXIS_SERVO",
                "enum": "Y",
                "pin": 2,
                "speed": 50,
                "min": 0,
                "max": 180,
                "start": 90,
                "interpolation": "ease"
            },
            {
                "name": "A_MOTOR_SERVO",
                "enum": "A",
                "pin": 3,
                "speed": 50,
                "min": 0,
                "max": 180,
                "start": 90,
                "interpolation": "linear"
            },
            {
                "name": "B_MOTOR_SERVO",
                "enum": "B",
                "pin": 4,
                "speed": 50,
                "min": 0,
                "max": 180,
                "start": 90,
                "interpolation": "linear"
            }
        ],
        "motors": [
            {
                "name": "MOTOR_A_B",
                "enum": "C",
                "pin": 3
            }
        ]
    }
    )json";
}


#include "ExecutiveController.h"

ExecutiveController executiveController;
SerialController serialController;
ESCController motorAController;
ESCController motorBController;
ServoController xRotationController;
ServoController yRotationController;
ServoController motorAServoController;
ServoController motorBServoController;
RelayController motorARelayController;
RelayController motorBRelayController;

void print_status() {
  std::string tmp;
  tmp = "%%%_HEARTBEAT_%%%_X_SERVO_POS:" + std::to_string(xRotationController.getCurrentAngle());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_Y_SERVO_POS:" + std::to_string(yRotationController.getCurrentAngle());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_A_SERVO_POS:" + std::to_string(motorAServoController.getCurrentAngle());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_B_SERVO_POS:" + std::to_string(motorBServoController.getCurrentAngle());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_A_SPEED:" + std::to_string(motorAController.getCurrentSpeed());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_B_SPEED:" + std::to_string(motorBController.getCurrentSpeed());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_A_RELAY_STATE:" + std::to_string(motorARelayController.state());
  Serial.println(tmp.c_str());
  tmp = "%%%_HEARTBEAT_%%%_MOTOR_B_RELAY_STATE:" + std::to_string(motorBRelayController.state());
  Serial.println(tmp.c_str());
}

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
        {"E",  {"Executive Controller",           [&](std::string cmd) { executiveController.HandleGcodeCommand(cmd);           return true;  }}},
        {"X",  {"X axis",                         [&](std::string cmd) { xRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"Y",  {"Y axis",                         [&](std::string cmd) { yRotationController.handleGcodeCommand(cmd);            return true;  }}},
        {"A",  {"A motor servo",                  [&](std::string cmd) { motorAServoController.handleGcodeCommand(cmd);          return false; }}},
        {"B",  {"B motor servo",                  [&](std::string cmd) { motorBServoController.handleGcodeCommand(cmd);          return false; }}},
        {"C",  {"A motor",                        [&](std::string cmd) { motorAController.handleGcodeCommand(cmd);               return true;  }}},                                                    
        {"D",  {"B motor",                        [&](std::string cmd) { motorBController.handleGcodeCommand(cmd);               return true;  }}},
        {"F",  {"A motor relay",                  [&](std::string cmd) { motorARelayController.handleGcodeCommand(cmd);          return true;  }}},
        {"G",  {"B motor relay",                  [&](std::string cmd) { motorBRelayController.handleGcodeCommand(cmd);          return true;  }}},
    };
}

void ExecutiveController::storeTransmittedMessage(const char * msg) {
    std::string msgStr = msg;
    if (msgStr.find("HEARTBEAT") == std::string::npos) { // heartbeat not found in string
      transmittedMessages.push_back(msg);
    }
}

int ExecutiveController::initialize() {
    sessionKey = "NOT_SET";
    std::string tmp = "%%%_INFO: EXECUTIVE CONTROLLER: STARTING SETUP";
    Serial.println(tmp.c_str());

    if (loadConfig()) {
      Serial.println("%%%_ERR:INVALID_CONFIG");
      return 1;
    }
    // Serial.setWriteHandler(storeTransmittedMessage);
    // std::string key = generateSessionKey();
    // setSessionKey(key);
    tmp = "%%%_INFO: EXECUTIVE CONTROLLER: FINISHED SETUP";
    Serial.println(tmp.c_str());
    // PrintSessionKey();
    return 0;
}

void ExecutiveController::PrintHeartbeat() {
  std::string tmp = "%%%_HEARTBEAT_TAG";
  Serial.println(tmp.c_str());

  tmp = "%%%_HEARTBEAT_START";
  Serial.println(tmp.c_str());

  print_status();

  tmp = "%%%_HEARTBEAT_END";
  Serial.println(tmp.c_str());
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
    motorARelayController.initialize("MOTOR_A_RELAY", doc["MOTOR_A_RELAY"]);
    motorBRelayController.initialize("MOTOR_B_RELAY", doc["MOTOR_B_RELAY"]);

    return 0;
}

// const char* ExecutiveController::getConfigJsonString() {
//     return R"json(
//     {
//       "X_SERVO": { "pin": 2, "speed": 30, "starting_angle": 60, "angle_limit": 120, "interpolation": "ease" },
//       "Y_SERVO": { "pin": 4, "speed": 50, "starting_angle": 45, "angle_limit": 100, "interpolation": "ease" },
//       "MOTOR_A_SERVO": { "pin": 6, "speed": 50, "starting_angle": 0, "angle_limit": 20, "interpolation": "linear" },
//       "MOTOR_B_SERVO": { "pin": 8, "speed": 50, "starting_angle": 20, "angle_limit": 20, "interpolation": "linear" },
//       "MOTOR_A": { "pin": 10 },
//       "MOTOR_B": { "pin": 15 }
//     }
//     )json";
// }

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


void ExecutiveController::HandleGcodeCommand(std::string cmd) {
  char actionType = cmd[1];
  if (actionType == 'T') {
    int unix_len = 10;
    if (cmd.length() > static_cast<std::string::size_type>(2 + unix_len)) {
      SetSystemTime(cmd.substr(2));
    }
  } else if (actionType == 'H') {
    PrintHeartbeat();
  } else if (actionType == 'R') {
    Reset();
  } else if (actionType == 'K') {
    PrintSessionKey();
  }
}

void ExecutiveController::RequestServerTime() {
  Serial.println("%%%_SERVER:REQ_TIME");
}

void ExecutiveController::SetSystemTime(std::string coded_string) {
  DateTime t = decodeUnixTimestampString(coded_string);
  setTime(t.Hour, t.Minute, t.Second, t.Day, t.Month, t.Year);
}

DateTime ExecutiveController::decodeUnixTimestampString(const std::string& unixTimestampStr) {
    time_t rawtime = std::stoll(unixTimestampStr);
    struct tm * ptm = gmtime(&rawtime);

    DateTime t = {
        .Year = ptm->tm_year + 1900,
        .Month = ptm->tm_mon + 1,
        .Day = ptm->tm_mday,
        .Hour = ptm->tm_hour,
        .Minute = ptm->tm_min,
        .Second = ptm->tm_sec
    };

    return t;
}

std::string ExecutiveController::generateSessionKey() {
  unsigned long currentTime = now(); 
  unsigned long randomNumber = random(100000, 999999);
  std::string randomString = "";
  for (int i = 0; i < 5; i++) {
    randomString += randomChar();
  }
  std::string combinedString = std::to_string(currentTime) + std::to_string(randomNumber) + randomString;
  std::string hexString = convertToHex(combinedString);
  return hexString;
}

void ExecutiveController::setSessionKey(std::string key) {
  sessionKey = key;
}

void ExecutiveController::PrintSessionKey() {
  std::string tmp = "%%%_SERVER:SESSION_KEY:" + GetSessionKey();
  Serial.println(tmp.c_str());
}

std::string ExecutiveController::GetSessionKey() {
  return sessionKey;
}

char ExecutiveController::randomChar() {
  const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  int randomValue = random(0, sizeof(chars) - 2);
  return chars[randomValue];
}

std::string ExecutiveController::convertToHex(const std::string &input) {
  std::stringstream hexStream;
  for (unsigned int i = 0; i < input.size(); i++) {
    char c = input[i];
    hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
  }
  return hexStream.str();
}

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
  Serial.print("%%%_HEARTBEAT_%%%_X_SERVO_POS:");
  Serial.println(xRotationController.getCurrentAngle());
  Serial.print("%%%_HEARTBEAT_%%%_Y_SERVO_POS:");
  Serial.println(yRotationController.getCurrentAngle());
  Serial.print("%%%_HEARTBEAT_%%%_MOTOR_A_SERVO_POS:");
  Serial.println(motorAServoController.getCurrentAngle());
  Serial.print("%%%_HEARTBEAT_%%%_MOTOR_B_SERVO_POS:");
  Serial.println(motorBServoController.getCurrentAngle());
  Serial.print("%%%_HEARTBEAT_%%%_MOTOR_A_SPEED:");
  Serial.println(motorAController.getCurrentSpeed());
  Serial.print("%%%_HEARTBEAT_%%%_MOTOR_B_SPEED:");
  Serial.println(motorBController.getCurrentSpeed());
}

void adam() {

  serialController.handleSerial();



}

std::map<std::string, SerialController::Command> SerialController::initializeCommandMap() {
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
  Serial.println("%%%_INFO:EXECUTIVE_CONTROLLER:Finished init");
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
            blinkLED();
            Serial.flush();

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

    auto cmdMap = initializeCommandMap();
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

void SerialController::handleCommand(const std::string& cmd) {
    std::string label = cmd.substr(0, 1);
    std::string stdcmd = cmd;
    Command& command = commandMap[label];
    command.action(stdcmd);
}

void ExecutiveController::storeTransmittedMessage(const char * msg) {
    std::string msgStr = msg;
    if (msgStr.find("HEARTBEAT") == std::string::npos) { // heartbeat not found in string
      transmittedMessages.push_back(msg);
    }
}

int ExecutiveController::initialize() {

    sessionKey = "NOT_SET";

    Serial.println("%%%_INFO: EXECUTIVE CONTROLLER: STARTING SETUP");
    if (loadConfig()) {
      Serial.println("%%%_ERR:INVALID_CONFIG");
      return 1;
    }
    // Serial.setWriteHandler(storeTransmittedMessage);
    // std::string key = generateSessionKey();
    // setSessionKey(key);
    Serial.println("%%%_INFO: EXECUTIVE CONTROLLER: FINISHED SETUP");
    // PrintSessionKey();
    return 0;
}

void ExecutiveController::PrintHeartbeat() {
  Serial.println("%%%_HEARTBEAT_TAG");
  Serial.println("%%%_HEARTBEAT_START"); 
  print_status(); 
  Serial.println("%%%_HEARTBEAT_END");
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
    motorAController.initialize("MOTOR_A", doc["MOTOR_A"]);
    motorBController.initialize("MOTOR_B", doc["MOTOR_B"]);
    motorARelayController.initialize("MOTOR_A_RELAY", doc["MOTOR_A_RELAY"]);
    motorBRelayController.initialize("MOTOR_B_RELAY", doc["MOTOR_B_RELAY"]);
    serialController.initialize(serialController.initializeCommandMap());

    return 0;
}

const char* ExecutiveController::getConfigJsonString() {
    return R"json(
    {
      "X_SERVO": { "pin": 1, "speed": 30, "starting_angle": 60, "angle_limit": 120 },
      "Y_SERVO": { "pin": 2, "speed": 50, "starting_angle": 45, "angle_limit": 90 },
      "MOTOR_A_SERVO": { "pin": 3, "speed": 50, "starting_angle": 0, "angle_limit": 20 },
      "MOTOR_B_SERVO": { "pin": 4, "speed": 50, "starting_angle": 20, "angle_limit": 20 },
      "MOTOR_A": { "pin": 5 },
      "MOTOR_B": { "pin": 6 }
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
  Serial.print("%%%_SERVER:SESSION_KEY:");
  Serial.println(GetSessionKey().c_str());
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

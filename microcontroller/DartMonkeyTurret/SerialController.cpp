#include "SerialController.h"


void SerialController::initialize(std::map<std::string, SerialController::Command> cmdMap) {
  commandMap = cmdMap;
  
  std::string tmp = "%%%_INFO:EXECUTIVE_CONTROLLER:Finished init";
  Serial.println(tmp.c_str());
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

        if (incomingChar != '\n' && incomingChar != '\r') {
            continue;
        }

        inputBuffer.erase(std::remove(inputBuffer.begin(), inputBuffer.end(), '\n'), inputBuffer.end());
        inputBuffer.erase(std::remove(inputBuffer.begin(), inputBuffer.end(), '\r'), inputBuffer.end());
        Serial.flush();

        char *token = strtok(&inputBuffer[0], ";");
        while (token != NULL) {
            std::string command = token;
            command.erase(std::remove_if(command.begin(), command.end(), ::isspace), command.end());
            commandBuffer.push_back(command);
            token = strtok(NULL, ";");
        }

        inputBuffer = "";
    }
}

bool SerialController::GetCommandBuffer(std::vector<std::string> &buf) {
    if (!commandBuffer.empty()) {
        buf = commandBuffer;
        commandBuffer.clear();
        return true;
    } else {
        return false;
    }
}

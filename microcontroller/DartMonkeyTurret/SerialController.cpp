#include "SerialController.h"

void SerialController::Init() {
    std::string tmp = "%%%_INFO: SERIAL CONTROLLER: STARTING SETUP";
    Serial.println(tmp.c_str());

    tmp = "%%%_INFO: SERIAL CONTROLLER: FINISHED SETUP";
    Serial.println(tmp.c_str());
}

void SerialController::ReadSerial() {
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

        inputBuffer.clear();
    }
}

bool SerialController::GetCommandBuffer(std::vector<std::string> &buf) {
    if (!commandBuffer.empty()) {
        buf = std::move(commandBuffer);
        commandBuffer.clear();
        return true;
    }
    return false;
}

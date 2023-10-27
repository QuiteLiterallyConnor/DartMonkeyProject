#include "RelayController.h"

void RelayController::initialize(std::string n, StaticJsonDocument<1024> config) {
    name = n;
    relayPin = config["pin"];
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW); // Initialize in an OFF state
    std::string tmp;
    tmp += "%%%_INFO:";
    tmp += name;
    tmp += ": Relay initialized and set to OFF.";
    Serial.println(tmp.c_str());
}

void RelayController::handleGcodeCommand(std::string cmd) {
  char actionType = cmd[1];
  if (actionType == 'T') {
    toggle();
  } else if (actionType == 'O') {
    setOn();
  } else if (actionType == 'F') {
    setOff();
  }
}

void RelayController::setOn() {
  digitalWrite(relayPin, HIGH);
  currentState = true;
  printState();
}

void RelayController::setOff() {
  digitalWrite(relayPin, LOW);
  currentState = false;
  printState();
}

void RelayController::toggle() {
  currentState = !currentState;
  digitalWrite(relayPin, currentState ? HIGH : LOW);
  printState();
}

bool RelayController::state() {
  return currentState;
}

std::string RelayController::getName() {
  return name;
}

void RelayController::printState() {
    std::string tmp;
    tmp += "%%%_";
    tmp += name;
    tmp += "_STATE:";
    tmp += (currentState ? "ON" : "OFF");
    
    Serial.println(tmp.c_str());
}

#include "ESCController.h"

void ESCController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  controllerPin = config["pin"];
  controller.attach(controllerPin);    
  sync();
  Serial.print("%%%_INFO:");
  Serial.print(name.c_str());
  Serial.print(": Ready to transmit ESC PWM signals at pin " );
  Serial.println(controllerPin);
}

void ESCController::sync() {
  controller.writeMicroseconds(IDLE_PWM);
  delay(3000);
  controller.writeMicroseconds(STOP_PWM);
}

void ESCController::handleGcodeCommand(std::string cmd) {
  char actionType = cmd[1];
  int value = std::stoi(cmd.substr(2));

  if (actionType == 'S') {
    setSpeed(value);
  } else if (actionType == 'O') {
    offsetSpeed(value);
  } else if (actionType == 'T') {
    togglePower();
  }
}

int ESCController::getCurrentSpeed() const {
    return currentSpeed;
}

void ESCController::togglePower() {
    if (currentSpeed == 0) {
        setSpeed(prevSpeed);
    } else {
        prevSpeed = currentSpeed;
        setSpeed(0);
    }
    print();
}

void ESCController::setSpeed(int speed) {
    if (speed >= 0 && speed <= 100) {
      (speed > 0) ? controller.writeMicroseconds(speedToPulseWidth(speed)) : controller.writeMicroseconds(IDLE_PWM);
      currentSpeed = speed;
    }
    print();
}

void ESCController::offsetSpeed(int delta) {
  setSpeed(currentSpeed + delta);
}

int ESCController::speedToPulseWidth(int speed) const {
    return map(speed * 10, 0, 1000, LOWER_PWM_LIMIT, UPPER_PWM_LIMIT);
}

void ESCController::print() {
    Serial.print("%%%_");
    Serial.print(name.c_str());
    Serial.print("_SPEED:");
    Serial.println(currentSpeed);
}

#include "ESCController.h"

void ESCController::Init(StaticJsonDocument<1024> config) {
    name = config["name"].as<std::string>();
    controllerPin = config["pin"];
    controller.attach(controllerPin);    
    sync();
    std::string tmp = "%%%_INFO:" + name + ": Ready to transmit ESC PWM signals at pin " + std::to_string(controllerPin);
    Serial.println(tmp.c_str());
    controller.setEaseTo(EASE_LINEAR);
}

void ESCController::sync() {
    controller.writeMicroseconds(IDLE_PWM);
    delay(500);
    controller.writeMicroseconds(STOP_PWM);
}

void ESCController::handleGcodeCommand(std::string cmd) {
    char actionType = cmd[1];
    int value = std::stoi(cmd.substr(2));

    switch (actionType) {
        case 'S':
            setSpeed(value);
            break;
        case 'O':
            offsetSpeed(value);
            break;
        case 'T':
            togglePower();
            break;
        default:
            break;
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
        currentSpeed = speed;
        print();
        int pulseWidth = (speed > 0) ? speedToPulseWidth(speed) : IDLE_PWM;
        controller.startEaseTo(pulseWidth, 10);
    } else {
        print();
    }
}

void ESCController::offsetSpeed(int delta) {
    setSpeed(currentSpeed + delta);
}

int ESCController::speedToPulseWidth(int speed) const {
    return map(speed * 10, 0, 1000, LOWER_PWM_LIMIT, UPPER_PWM_LIMIT);
}

void ESCController::print() {
    std::string tmp = "%%%_" + name + "_SPEED:" + std::to_string(currentSpeed);
    Serial.println(tmp.c_str());
}

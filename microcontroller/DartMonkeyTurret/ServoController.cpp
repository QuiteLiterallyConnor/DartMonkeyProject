#include "ServoController.h"

void ServoController::Init(StaticJsonDocument<1024> config) {
    name = config["name"].as<std::string>();
    servoPin = config["pin"];
    speed = config["speed"];
    angle_min = config["min"];
    angle_max = config["max"];
    startingAngle = config["start"];
    interpolation = config["interpolation"].as<std::string>();
    servo.attach(servoPin);

    if (startingAngle < angle_min || startingAngle > angle_max) {
        std::string error_msg = "%%%_ERROR: " + name + " starting angle out of range";
        Serial.println(error_msg.c_str());
        return;
    }

    if (interpolation == "linear") {
        servo.setEasingType(EASE_LINEAR);
    } else if (interpolation == "ease") {
        servo.setEasingType(EASE_CUBIC_IN_OUT);
    } else {
        servo.setEasingType(EASE_LINEAR);
    }

    setAngle(startingAngle);
    std::string tmp = "%%%_INFO:" + name + ": Ready to transmit Servo PWM signals at pin " + std::to_string(servoPin);
    Serial.println(tmp.c_str());
}

void ServoController::handleGcodeCommand(std::string cmd) {
    char actionType = cmd[1];
    int value = std::stoi(cmd.substr(2));
    switch (actionType) {
        case 'S':
            setAngle(value);
            break;
        case 'O':
            changeAngle(value);
            break;
        default:
            break;
    }
}

void ServoController::changeAngle(int delta) {
    setAngle(currentAngle + delta);
}

void ServoController::setAngle(int angle) {
    if (angle >= angle_min && angle <= angle_max) {
        currentAngle = angle;
        print();
        servo.startEaseTo(angle, speed);
    }
}

int ServoController::getCurrentAngle() {
    return currentAngle;
}

std::string ServoController::getName() {
    return name;
}

void ServoController::print() {
    std::string tmp = "%%%_" + name + "_POS:" + std::to_string(currentAngle);
    Serial.println(tmp.c_str());
}

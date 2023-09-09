#include "ESCController.h"

void ESCController::initialize(std::string n, StaticJsonDocument<500> config) {
  name = n;
  controllerPin = config["pin"];
  controller.attach(controllerPin);    
  sync();
  Serial.print("Ready to transmit ESC PWM signals at pin " );
  Serial.println(controllerPin);
}

void ESCController::sync() {
  controller.writeMicroseconds(IDLE_PWM);
  delay(3000);
  controller.writeMicroseconds(STOP_PWM);
}

int ESCController::getCurrentSpeed() const {
    return currentSpeed;
}

void ESCController::togglePower() {
    if (currentSpeed == 0) {
        currentSpeed = prevSpeed;
        setSpeed(prevSpeed);
    } else {
        prevSpeed = currentSpeed;
        setSpeed(0);
    }
    print();
}

void ESCController::setSpeed(int speed) {
    (speed == 0) ? controller.writeMicroseconds(IDLE_PWM) : controller.writeMicroseconds(speedToPulseWidth(speed));
    currentSpeed = speed;
    print();
}

void ESCController::changeSpeed(int delta) {
    if (currentSpeed + delta >= 0 && currentSpeed + delta <= 100) {
        currentSpeed += delta;
        setSpeed(currentSpeed);
    }
    print();
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

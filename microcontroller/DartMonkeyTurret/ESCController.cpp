#include "ESCController.h"

void ESCController::initialize(int pin) {
  controllerPin = pin;
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

void ESCController::stop() {
    controller.writeMicroseconds(STOP_PWM);
}

void ESCController::togglePower() {
    if (currentSpeed == 0) {
        currentSpeed = prevSpeed;
        setSpeed(prevSpeed);
    } else {
        prevSpeed = currentSpeed;
        setSpeed(0);
    }
    Serial.print("Motor A & B: ");
    Serial.println(currentSpeed);

}

void ESCController::setSpeed(int speed) {
    if (speed == 0) {
        controller.writeMicroseconds(IDLE_PWM);
    } else {
        controller.writeMicroseconds(speedToPulseWidth(speed));
    }

    currentSpeed = speed;
    Serial.print("Motor A & B: ");
    Serial.println(currentSpeed);

}

void ESCController::changeSpeed(int delta) {
  Serial.println("changing speed");
    if (currentSpeed + delta >= 0 && currentSpeed + delta <= 100) {
        currentSpeed += delta;
        setSpeed(currentSpeed);
    }
    Serial.print("Motor A & B: ");
    Serial.println(currentSpeed);
}

int ESCController::speedToPulseWidth(int speed) const {
    return map(speed * 10, 0, 1000, LOWER_PWM_LIMIT, UPPER_PWM_LIMIT);
}





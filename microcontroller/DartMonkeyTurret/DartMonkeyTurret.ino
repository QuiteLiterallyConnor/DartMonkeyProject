#include "ExecutiveController.h"

void setup() {
    Serial.begin(115200);
    Serial.println("%%%_INFO:starting setup");
    init_controllers();
    Serial.println("%%%_INFO:finished setup");
}

void loop() {
    adam();
}
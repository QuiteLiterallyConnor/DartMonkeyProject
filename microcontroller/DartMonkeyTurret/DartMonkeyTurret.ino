#include "ExecutiveController.h"
void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("starting setup...");
    init_controllers();
    Serial.println("finished setup.");
}

void loop() {
    adam();
}
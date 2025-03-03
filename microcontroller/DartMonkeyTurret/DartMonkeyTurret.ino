#include "ExecutiveController.h"

ExecutiveController ec;

void setup() {
    Serial.begin(115200);
    if (ec.Init()) {
      std::string tmp = "%%%_ERR:EXEC_CONTROLLER:INIT_FAILED";
      Serial.println(tmp.c_str());
    }
    Serial.println("%%%_INFO:finished setup");
}

void loop() {
    ec.GetCommands();
    ec.ExecuteCommands();
}
// photoplethysmography heart rate monitor
// using libraries available and own implementation if needed
#include "DFRobot_Heartrate.h"
#include "monitor.h"
#define heartPin A0


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Monitor_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  int heartValue = analogRead(heartPin);
  Serial.println(heartValue);
  delay(20);
  Monitor_update();
}

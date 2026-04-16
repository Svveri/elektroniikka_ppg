#include <PulseSensorPlayground.h>
#include "monitor.h"

PulseSensorPlayground pulse;


const int SIGNAL_PIN = A0;
const int THRESHOLD = 550;

void setup() {
  Serial.begin(115200);
  Monitor_init();
  pulse.analogInput(SIGNAL_PIN);
  pulse.setThreshold(THRESHOLD);

  if (pulse.begin()) {
    Serial.println("PulseSensor ready");
  }
}

void loop() {
  int bpm = pulse.getBeatsPerMinute();

  if (pulse.sawStartOfBeat()) {
    Serial.print("Beat detected | BPM: ");
    Serial.println(bpm);
  }
  Monitor_update(bpm, SIGNAL_PIN);
  delay(20);
}

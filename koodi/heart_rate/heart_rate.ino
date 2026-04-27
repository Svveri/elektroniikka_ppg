#include <Arduino.h>
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
  float signal = analogRead(SIGNAL_PIN);
  int bpm = pulse.getBeatsPerMinute();

  if (pulse.sawStartOfBeat()) {
    Serial.print("Beat detected | BPM: ");
    Serial.println(bpm);
  }
  Monitor_update(bpm, signal);
  delay(2); // 500hz ???
}

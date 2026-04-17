#include <Arduino.h>
#include <PulseSensorPlayground.h>
#include "monitor.h"

// LED PINNIT

const int PIN_GREEN = 9;
const int PIN_RED =   10;
const int PIN_IR =   5;

// PPG SIGNAALIN PINNI

const int SIGNAL_PIN =   A0;

// LED AJOITUKSET
const int LED_ON =    5;
const int SETTLE_MS = 1;

// PULSESENSOR ASETUKSET
const int THRESHOLD = 550;

// pulseplaygroundista asetetaan pulse
PulseSensorPlayground pulse;

// LED TILA
enum LEDPhase { PHASE_GREEN, PHASE_RED, PHASE_IR};
LEDPhase currentPhase = PHASE_GREEN;

// TILA 
float Signal        =  0.0;
int   BPM           =  0;
float baseline      =  512.0;

// Ajoitus 
long lastSwitchTime =  0;
bool settling       =  false;
long settleStart    =  0;

// Setup
void setup() {
  Serial.begin(115200);

  // Alustetaan pinnit ja asetetaan tilat
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_IR, OUTPUT);
  // sammutetaan niin alkaa pois päältä pinnit
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_IR, LOW);

  // 3.3 V referenssi ADC:lle
  analogReference(EXTERNAL);

  // Pulssisensorin konfigurointi
  pulse.analogInput(SIGNAL_PIN);
  pulse.setThreshold(THRESHOLD);

  // tarkistetaan käynnistyikö
  if(!pulse.begin()) {
    Serial.println("PULSESENSOR DID NOT START LOL");
  } else {
    Serial.println("PULSESENSOR OK");
  }

  // Näyttö päälle
  Monitor_init();
  
  // Käynnistä ensimmäinen LEDi, jotta päästään alkuun
  digitalWrite(PIN_GREEN, HIGH);
  lastSwitchTime = millis();
  settling       = true;
  settleStart    = millis();

  Serial.println("PPG VALMIS ;)");
}

void loop() {
  long now       = millis();

  if(settling) {
    if(now-settleStart >= SETTLE_MS){
      settling = false;

      // Luetaan ADC vain vihreä LED:n aikana (PARAS PPG SIGNAALI)
      if (currentPhase == PHASE_GREEN){
        int raw   = analogRead(SIGNAL_PIN);

        baseline = baseline * 0.99 + (float)raw*0.01;
        float ac = (float)raw - baseline;

        Signal   = (ac + 80.0) / 160.0;
        Signal   = constrain(Signal, 0.0, 1.0);
      }
    }
    Monitor_update(BPM, Signal);
    return;
  }

  // LED vaihto
  if (now- lastSwitchTime >= LED_ON){
    lastSwitchTime = now;

    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_RED, LOW);

    switch (currentPhase){
      case PHASE_GREEN: currentPhase = PHASE_RED; break;
      case PHASE_RED:   currentPhase = PHASE_IR; break;
      case PHASE_IR:    currentPhase = PHASE_GREEN; break;
    }

    switch (currentPhase){
      case PHASE_GREEN: digitalWrite(PIN_GREEN, HIGH); break;
      case PHASE_RED:   digitalWrite(PIN_RED, HIGH); break;
      case PHASE_IR:    digitalWrite(PIN_IR, HIGH); break;
    }
    
    settling    = true;
    settleStart = now;
  }

  // PULSSI
  if (pulse.sawNewSample()) {
    if (pulse.sawStartOfBeat()){
      BPM = (float)pulse.getBeatsPerMinute();
      Serial.print("Beat detected | BPM: ");
      Serial.println((int)BPM);
    }
  }
  Monitor_update(BPM, Signal);
}

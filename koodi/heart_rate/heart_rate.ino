// photoplethysmography heart rate monitor
// using libraries available and own implementation if needed
#include "DFRobot_Heartrate.h"
#include "monitor.h"
#include "source.h"
#define heartPin A0

DFRobot_Heartrate heartrate(ANALOG_MODE); // alustetaan libraryn metodit käsittelemään analogista signaalia

const int sensorPin = A0; // asetetaan A0 portti sensorPin muuttujaan
int lastBPM = 0; // Beats Per Minute arvon säilyttävä muuttuja 
int signalValue = 0;
float signal = 0;

// Muuttujat RR-intervalli funktiota varten
/*unsigned long rrBuffer[RR_WINDOW]; // tämä on se itse bufferi [array], joka säilyttää millisekunteina RR piikkien intervallin; esim rrBuffer = [995, 1000, 992, 1003, 1005]
int rrIndex = 0; // tämä indexi määrää mihin kohtaan rrBufferissa kirjoitetaan tällä hetkellä
int rrCount = 0; // kertoo kuinka monta validia tulosta on säilytyksessä, estää virhetilanteet kuten keskiarvon laskun nollalla
unsigned long lastBeatMillis = 0; // ajastusta varten muuttuja, tällä nähdään RR piikkien välillä oleva aika
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Monitor_init(); // näytön alustus
}

void loop() {
  // put your main code here, to run repeatedly:
  signalValue = analogRead(heartPin); // luetaan A0 portista signaali
  signal = signalValue / 1023.0;
  int bpm = heartrate.getValue(signalValue); // käytetään kirjaston metodia, 

  computeRR(bpm); // kutsutaan tämän funktion sisällä myös computeBPM();
  /**
  Serial.print("Signal: ");
  Serial.print(signalValue);

  Serial.print(" | BPM: ");
  if (lastBPM > 0) {
    Serial.print(lastBPM);
  } else {
    Serial.print("--");
  }

  Serial.println();
*/
  //delay(2);  // High sampling rate (~500 Hz)

  Monitor_update(lastBPM, signalValue);
}

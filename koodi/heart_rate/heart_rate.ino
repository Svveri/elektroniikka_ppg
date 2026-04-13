// photoplethysmography heart rate monitor
// using libraries available and own implementation if needed
#include "DFRobot_Heartrate.h"
#include "monitor.h"
#define heartPin A0

DFRobot_Heartrate heartrate(ANALOG_MODE); // alustetaan libraryn metodit käsittelemään analogista signaalia

const int sensorPin = A0; // asetetaan A0 portti sensorPin muuttujaan
int lastBPM = 0; // Beats Per Minute arvon säilyttävä muuttuja 

// Muuttujat RR-intervalli funktiota varten
#define RR_WINDOW 5 // RR intervallin keskiarvon laskemiseen tarkoitettu ikkunan koko
unsigned long rrBuffer[RR_WINDOW]; // tämä on se itse bufferi, joka säilyttää millisekunteina RR piikkien intervallin; esim rrBuffer = [995, 1000, 992, 1003, 1005]
int rrIndex = 0; // tämä indexi määrää mihin kohtaan rrBufferissa kirjoitetaan tällä hetkellä
int rrCount = 0; // kertoo kuinka monta validia tulosta on säilytyksessä, estää virhetilanteet kuten keskiarvon laskun nollalla
unsigned long lastBeatMillis = 0; // ajastusta varten muuttuja, tällä nähdään RR piikkien välillä oleva aika
// RR-intervalli integrointi funktio, tasoittaa pulssin muodostuksen, ei pompi holtittomasti edestakaisin
int computeBPM() {
  unsigned long sum = 0; // Tähän summataan RR tulokset, joista keskiarvo lasketaan
  for (int i = 0; i < rrCount; i++) {
    sum += rrBuffer[i]; // summataan bufferissa olevat luvut
  }
  unsigned long avgRR = sum / rrCount; // jaetaan summaus lukujen määrällä = saadaan keskiarvo
  return 60000 / avgRR; // Muunnetaan RR -> Beats Per Minute
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Monitor_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  int signalValue = analogRead(heartPin); // luetaan A0 portista signaali

  int bpm = heartrate.getValue(signalValue); // käytetään kirjaston metodia, 

  if (bpm > 0) {   // Jos bpm enemmän kuin 1 säilytetään validi lukema
  unsigned long now = millis(); // aloitetaan ajanotto
  unsigned long rr = now - lastBeatMillis; // laskee ajan joka on kulunut viime sykähdyksestä
  lastBeatMillis = now; // ---
  if (rr > 300 && rr < 2000) {   // jos intervalli on oikeansuhteinen, tämä suodattaa jotkin signaalin häröilyt pois, eli jos RR alle 300, kyseessä virhe. Jos RR yli 2000, tulos ei validi.
    rrBuffer[rrIndex] = rr; // lisätään bufferiin
    rrIndex = (rrIndex + 1) % RR_WINDOW; // varmistetaan että pysytään RR ikkunan sisäpuolella.
    if (rrCount < RR_WINDOW) rrCount++; // tarkastetaan onko RR näytteitä sen verran mitä ikkuna sallii
    lastBPM = computeBPM(); // integroi RR intervaalit ja muuttaa Beats Per Minutes- muotoon.
  }
}
  Serial.print("Signal: ");
  Serial.print(signalValue);

  Serial.print(" | BPM: ");
  if (lastBPM > 0) {
    Serial.print(lastBPM);
  } else {
    Serial.print("--");
  }

  Serial.println();

  delay(2);  // High sampling rate (~500 Hz)

 Monitor_update(lastBPM);
}

#include "source.h"
#include <Arduino.h>

extern int signalValue; // extern int koska alustettu ino tiedostossa, mutta halutaan käyttää monitor.cpp:ssä
extern unsigned long lastBeatMillis; // extern int koska alustettu ino tiedostossa
/*extern unsigned long rrBuffer[RR_WINDOW]; // extern koska tämäkin on alustettu ino tiedostossa
extern int rrIndex; // tämäkin alustetaan ino tiedostossa
extern int rrCount; // sama kuin ylempänä*/
extern int lastBPM; // sama kuin ylempänä


static unsigned long rrBuffer[RR_WINDOW];
int rrIndex, rrCount;
unsigned long lastBeatMillis;
unsigned long rr; // säilyttää RR piikkien välisen ajan ja tallentaa
unsigned long now;


// tällä funktiolla saadaan analoginen signaali asetettua muuttujaan
int get_signal() {
  return signalValue;
}

// RR-intervalli integrointi funktio, tasoittaa pulssin muodostuksen, ei pompi holtittomasti edestakaisin
int computeBPM() {
  unsigned long sum = 0; // Tähän summataan RR tulokset, joista keskiarvo lasketaan
  for (int i = 0; i < rrCount; i++) {
    sum += rrBuffer[i]; // summataan bufferissa olevat luvut
  }
  unsigned long avgRR = sum / rrCount; // jaetaan summaus lukujen määrällä = saadaan keskiarvo
  return 60000 / avgRR; // Muunnetaan RR -> Beats Per Minute
}

void computeRR(int bpm) {
  static unsigned long lastAccepted = 0;
  unsigned long now = millis();

  if (bpm <= 5) return;

  // estä useat triggerit saman lyönnin aikana
  if (now - lastAccepted < 250) return;

  unsigned long rr = now - lastBeatMillis;
  lastBeatMillis = now;
  lastAccepted = now;

  if (rr < 300 || rr > 2000) return;

  rrBuffer[rrIndex] = rr;
  rrIndex = (rrIndex + 1) % RR_WINDOW;
  if (rrCount < RR_WINDOW) rrCount++;

  lastBPM = computeBPM();
}
/*
void computeRR(int bpm) {
  if (bpm <= 5) {
    return;
  }    // Jos bpm enemmän kuin 0 säilytetään validi lukema
  now = millis(); // aloitetaan ajanotto
  rr = now - lastBeatMillis; // laskee ajan joka on kulunut viime sykähdyksestä
  lastBeatMillis = now; // ---
  if (rr > 300 && rr < 2000) {   // jos intervalli on oikeansuhteinen, tämä suodattaa jotkin signaalin häröilyt pois, eli jos RR alle 300, kyseessä virhe. Jos RR yli 2000, tulos ei validi
    rrBuffer[rrIndex] = rr; // lisätään bufferiin
    rrIndex = (rrIndex + 1) % RR_WINDOW; // varmistetaan että pysytään RR ikkunan sisäpuolella
    if (rrCount < RR_WINDOW) rrCount++; // tarkastetaan onko RR näytteitä sen verran mitä ikkuna sallii
    lastBPM = computeBPM(); // integroi RR intervaalit ja muuttaa Beats Per Minutes- muotoon
  }
}*/
#ifndef SOURCE_H
#define SOURCE_H
#define RR_WINDOW 5 // RR intervallin keskiarvon laskemiseen tarkoitettu ikkunan koko

extern int computeBPM();

int get_signal();

int computeRR(int bpm);

#endif
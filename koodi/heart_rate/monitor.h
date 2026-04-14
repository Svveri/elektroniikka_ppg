#ifndef MONITOR_H
#define MONITOR_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Initialize the monitor
void Monitor_init();

// Update everything (call inside loop)
void Monitor_update(float RR, float signal);

#endif
#include <Arduino.h>
#include "monitor.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <math.h>

// ===== FUNCTION PROTOTYPES (REQUIRED IN .CPP) =====
int ppgX(float externalsig);
int ppgX3(float bpm);
void drawGraph();
void drawUI();
void updateNumbers();
void drawHeart();

// PINS
// MONITORS 1 -> PIN VCC 3.3
// MONITORS 2 -> PIN GND
#define TFT_CS 8 // CS MONITOR PIN -> DIGITAL PIN 8 (RESISTORS)
#define TFT_RST 9 // RESET PIN MONITOR -> DIGITAL PIN 9 (RESISTORS)
#define TFT_DC 10 // A0 MONITOR PIN -> DIGITAL PIN 10 (RESISTORS)
// MONITOR SDA PIN -> D11 (RESISTORS)
// MONITOR SCK PIN -> D13 PIN (RESISTORS)
// MONITOR LED -> 3.3V VIA 150 ohm


// CREATE TFT OBJECT
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// SCREEN
#define SCREEN_W 128
#define SCREEN_H 160

// COLOURS
#define C_BLACK 0x0000 
#define C_WHITE 0xFFFF
#define C_GREEN 0x07E0
#define C_LIME 0x3FE0
#define C_RED 0xF800
#define C_CYAN 0x07FF
#define C_DARKGREY 0x4208
#define C_LIGHTGREY 0xC618

// SYNTEETTINEN PPG-AALTO
// Yksi sydänlyönti (arvot 0–100 % leveydestä)
const uint8_t PPG_WAVE[] = {
  50, 52, 55, 60, 68, 80, 95, 85, 70, 60,
  55, 53, 52, 54, 60, 58, 55, 52, 50
};

// Taulukon pituus
const int PPG_LEN = sizeof(PPG_WAVE) / sizeof(PPG_WAVE[0]);

// GRAPH
#define GRAPH_X 1 // X START ON GRAPH
#define GRAPH_Y 1 // Y START ON GRAPH
#define GRAPH_W 38 // WIDTH OF GRAPH
#define GRAPH_H 126 // HEIGTH OF GRAPH
#define GRAPH_RIGHT (GRAPH_X + GRAPH_W) // RIGHT EDGES X CORDINATE 

// SIMULATION (SIM)
#define SCROLL_MS 200 // WAVEFORM UPDATE SPEED (ms)

// STATE
float simTime = 0.0; // SIMULATION TIME FOR WAVEFORM
float simBPM = 0; // this had the SIM_BPM value hard coded (= SIM_BPM;)

int waveBuf[GRAPH_H]; // BUFFER FOR HORIZONTAL PIXELS
int preWaveBuf[GRAPH_H]; // PREVIOUS FRAME

long lastUpdate = 0; // TIMESTAMP OF THE LAST WAVEFORM UPDATE
long lastUIUpdate = 0; // TIMESTAMP OF LAST SLOW UI UPDATE
#define UI_UPDATE_MS 1000 // SLOW UI REFRESHAL INTERVAL (ms)

// <3 + LIVE

// TURNED OFF LIVE DOT BECAUSE IT WAS ADDING LAG A BIT
// bool liveDot = false;
bool heartOn = false; // TURNS TRUE WHEN HEART IS RED
int beatFrames = 0; // FRAME COUNTER TO CONTROL HEARTBEAT TIMING 

// initialize
void Monitor_init() {

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3); // DISPLAY ROTATION
  tft.fillScreen(C_BLACK); //CLEAR SCREEN

  for (int i = 0; i < GRAPH_H; i++) {
    float t = (float)i / 50.0; // init time step
    int signal_value = 0;
    waveBuf[i] = ppgX3(signal_value); // fill buffer with PPG values, kutsutaan itse tekemää uutta funktiota
    preWaveBuf[i] = waveBuf[i];
  }

  simTime = (float)GRAPH_H / 50.0; // init sim time

  drawUI(); // DRAWS INIT UI
}

// monitor updating
void Monitor_update(float RR, float signal) { // float RR is the value that we want to show on the screen as BPM
  long now = millis(); // TIME IN MILLISECONDS
  simBPM = RR;
  // FAST WAVEFORM
  if (now - lastUpdate >= SCROLL_MS) {
    lastUpdate = now;
    int newX = ppgX(signal); // COMPUTE NEXT WAVEFORM X CORDINATE

    //SHIFT WAVEFORM BUFFER BY 1 PIXEL
    for (int i = 0; i < GRAPH_H - 1; i++) {
      waveBuf[i] = waveBuf[i + 1];
    }
    waveBuf[GRAPH_H - 1] = newX; // ADDS NEW VALUE AT BOTTOM

    drawGraph();

    // HEART BEAT TIMING
    beatFrames++; // FRAME COUNTER
    int beatPeriod = (int)(50.0 * 60.0 / simBPM); // FRAMES PER BEAT

    if (beatFrames >= beatPeriod) {
      beatFrames = 0; // RESET FRAME COUNTER
      heartOn = !heartOn; // TOGGLE HEART SYMBOL
      drawHeart(); // REDRAW HEART

      // BPM UPDATES UPON BEAT
      //tft.fillRect(70, 56, 60, 10, C_BLACK); // CLEAR PREVIOUS BPM
      //tft.setTextColor(C_CYAN);
      //tft.setCursor(70, 56);
      //tft.print((int)simBPM);
      //tft.print(" BPM");
    }
  }

  // SLOW UI
  if (now - lastUIUpdate >= UI_UPDATE_MS) {
    lastUIUpdate = now;
    updateNumbers(); //UPDATES VALUES
  }

  // LIVE DOT DISABLED FOR NOW
  static long lastBlink = 0;
/*  if (now - lastBlink > 500) {
    lastBlink = now;
     liveDot = !liveDot;
    drawLiveDot();
  }*/
}

// GENERATES PPG (SIMULATED HEARTWAVE REPLACE THIS!!!!)
int ppgX(float signal) {
  float sig = constrain(signal, 0.0, 1.0); 
  int px = GRAPH_X + 2 + (int)(sig * (GRAPH_W - 6));
  return constrain(px, GRAPH_X + 1, GRAPH_RIGHT - 1);
}

// GRAPH
/*void drawGraph() {

  for (int i = 1; i < GRAPH_H; i++){
    tft.drawLine(preWaveBuf[i - 1], GRAPH_Y + i - 1, 
    preWaveBuf[i], GRAPH_Y + i, C_BLACK);
  } 
  tft.drawPixel(preWaveBuf[GRAPH_H - 1], GRAPH_Y + GRAPH_H - 1, C_RED);
  // Erase previous line and dot

  int midX = GRAPH_X + GRAPH_W / 2;
  //DRAW CERTICAL GRID DOTS
  for (int y = GRAPH_Y; y < GRAPH_Y + GRAPH_H; y += 3) {
    tft.drawPixel(midX, y, C_DARKGREY);
  }

  // DRAW WAVEFORM LINES CONNECTING EACH BUFFER POINTS 
  for (int i = 1; i < GRAPH_H; i++) {
    tft.drawLine(
      waveBuf[i - 1], GRAPH_Y + i - 1,
      waveBuf[i],     GRAPH_Y + i,
      C_RED
    );
  }

  // DRAW LAST PIXEL IN WHITE FOR CLARITY
  tft.drawPixel(waveBuf[GRAPH_H - 1],
                GRAPH_Y + GRAPH_H - 1,
                C_WHITE);
}
*/
void drawGraph() {
  // ERASE PREVIOUS LINES (COLOUR THEM BLACK)
  for (int i = 1; i < GRAPH_H; i++) {
    tft.drawLine(
      preWaveBuf[i - 1], GRAPH_Y + i - 1,
      preWaveBuf[i],     GRAPH_Y + i,
      C_BLACK
    );
  }
  // Erase previous end dot
  tft.drawPixel(preWaveBuf[GRAPH_H - 1],
                GRAPH_Y + GRAPH_H - 1,
                C_BLACK);
 
  // Redraw vertical centre grid line (erasing removes it)
  int midX = GRAPH_X + GRAPH_W / 2;
  for (int y = GRAPH_Y; y < GRAPH_Y + GRAPH_H; y += 3) {
    tft.drawPixel(midX, y, C_DARKGREY);
  }
 
  // Draw new waveform lines
  for (int i = 1; i < GRAPH_H; i++) {
    tft.drawLine(
      waveBuf[i - 1], GRAPH_Y + i - 1,
      waveBuf[i],     GRAPH_Y + i,
      C_RED
    );
  }
 
  // White dot at leading edge
  tft.drawPixel(waveBuf[GRAPH_H - 1],
                GRAPH_Y + GRAPH_H - 1,
                C_WHITE);
 
  // Save current buffer as previous for next frame
  memcpy(preWaveBuf, waveBuf, sizeof(waveBuf));
}
 
// UI
void drawUI() {
  tft.fillScreen(C_BLACK);

  // MUN JA SEVERIN NIMI 
  tft.setCursor(40, 150);
  tft.print("©seve&rasse");

  // Labels for bpm signal and channel
  tft.setTextColor(C_LIGHTGREY);

  tft.setCursor(45, 56);
  tft.print("HR:");

  tft.setCursor(45, 80);
  tft.print("SIGNAL:");

  tft.setCursor(45, 104);
  tft.print("CHANNEL:");

  // GRAPH BORDER
  tft.drawRect(GRAPH_X - 1, GRAPH_Y - 1, GRAPH_W + 2, GRAPH_H + 2, C_CYAN);
  // here was simBPM = SIM_BPM;
  tft.fillRect(70, 56, 60, 10, C_BLACK);
  tft.setTextColor(C_CYAN);
  tft.setCursor(70, 56);
  tft.print((int)simBPM);
  tft.print(" BPM");
  
  updateNumbers(); // DRAW INITIAL SIGNAL AND CHANNEL VALUES
  drawHeart(); // DRAW INITIAL HEART SYMBOL
}

// UPDATE VALUES
void updateNumbers() {
  // simBPM = SIM_BPM + (float)(random(-2, 3)) * 0.5;

  // HR BEEN MOVED TO BE UPDATED WHENEVER THERES A NEED FOR TI
  // HR
  
  tft.fillRect(70, 56, 60, 10, C_BLACK);
  tft.setTextColor(C_CYAN);
  tft.setCursor(70, 56); 
  tft.print((int)simBPM);
  tft.print(" BPM");

  // SIGNAL
  tft.fillRect(60, 92, 50, 10, C_BLACK);
  tft.setTextColor(C_GREEN);
  tft.setCursor(60, 92);
  tft.print("GOOD");

  // CHANNEL
  tft.fillRect(60, 112, 40, 10, C_BLACK);
  tft.setTextColor(C_LIGHTGREY);
  tft.setCursor(60, 112);
  tft.print("GREEN");
}

// BEATING <3 SYMBOL
void drawHeart() {
  tft.fillRect(110, 68, 16, 12, C_BLACK);
  tft.setTextColor(heartOn ? C_RED : C_DARKGREY);
  tft.setCursor(110, 70);
  tft.print("<3");
}

// LIVE DOT
/*void drawLiveDot() {
  uint16_t color = liveDot ? C_RED : C_DARKGREY;
  tft.fillCircle(120, 6, 3, color);
}*/
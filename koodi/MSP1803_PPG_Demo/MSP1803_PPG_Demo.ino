// MSP1803 TFT Display — Vertical + Hospital UI + Heart Pulse

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <math.h>

// PINS
#define TFT_CS 8 // DIGITAL PIN 8
#define TFT_RST 9 // DIGITAL PIN 9
#define TFT_DC 10 // DIGITAL PIN 10

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

// GRAPH
#define GRAPH_X 1 // X START ON GRAPH
#define GRAPH_Y 1 // Y START ON GRAPH
#define GRAPH_W 38 // WIDTH OF GRAPH
#define GRAPH_H 126 // HEIGTH OF GRAPH
#define GRAPH_RIGHT (GRAPH_X + GRAPH_W) // RIGHT EDGES X CORDINATE 

// SIMULATION (SIM)
#define SIM_BPM 72.0 //72 BPM SIMULATED HEARTRATE
#define SCROLL_MS 200 // WAVEFORM UPDATE SPEED (ms)

// STATE
float simTime = 0.0; // SIMULATION TIME FOR WAVEFORM
float simBPM = SIM_BPM; // CURRENT HEART RATE 

int waveBuf[GRAPH_H]; // BUFFER FOR HORIZONTAL PIXELS

long lastUpdate = 0; // TIMESTAMP OF THE LAST WAVEFORM UPDATE
long lastUIUpdate = 0; // TIMESTAMP OF LAST SLOW UI UPDATE
#define UI_UPDATE_MS 1000 // SLOW UI REFRESHAL INTERVAL (ms)

// <3 + LIVE

// TURNED OFF LIVE DOT BECAUSE IT WAS ADDING LAG A BIT
// bool liveDot = false;
bool heartOn = false; // TURNS TRUE WHEN HEART IS RED
int beatFrames = 0; // FRAME COUNTER TO CONTROL HEARTBEAT TIMING 

// SETUP
void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3); // DISPLAY ROTATION
  tft.fillScreen(C_BLACK); //CLEAR SCREEN

  for (int i = 0; i < GRAPH_H; i++) {
    float t = (float)i / 50.0; // init time step
    waveBuf[i] = ppgX(t); // fill buffer with PPG values
  }

  simTime = (float)GRAPH_H / 50.0; // init sim time

  drawUI(); // DRAWS INIT UI
}

// LOOP
void loop() {
  long now = millis(); // TIME IN MILLISECONDS

  // FAST WAVEFORM 
  if (now - lastUpdate >= SCROLL_MS) {
    lastUpdate = now;

    simTime += 1.0 / 50.0; // INCREMENT SIMULATION TIME
    int newX = ppgX(simTime); // COMPUTE NEXT WAVEFORM X CORDINATE

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
      beatFrames = 0; //RESET FRAME COUNTER
      heartOn = !heartOn; // TOGGLE HEART SYMBOL
      drawHeart(); // REDRAW HEART

      // BPM UPDATES UPON BEAT
      tft.fillRect(70, 56, 60, 10, C_BLACK); //CLEAR PREVIOUS BPM
      tft.setTextColor(C_CYAN);
      tft.setCursor(70, 56);
      tft.print((int)simBPM);
      tft.print(" BPM");
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
int ppgX(float t) {
  float phase = fmod(t * (simBPM / 60.0) * TWO_PI, TWO_PI);

  float systolic = exp(-pow((phase - 0.9) / 0.28, 2));
  float dicrotic = exp(-pow((phase - 1.85) / 0.22, 2)) * 0.28;
  float diastolic = exp(-pow((phase - 3.8) / 1.3, 2)) * 0.12;

  float sig = systolic + dicrotic + diastolic;

  sig += (float)(random(-2, 3)) / 120.0;
  sig = constrain(sig, 0.0, 1.0);

  int px = GRAPH_X + 2 + (int)(sig * (GRAPH_W - 6));
  return constrain(px, GRAPH_X + 1, GRAPH_RIGHT - 1);
}

// GRAPH
void drawGraph() {
  tft.fillRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, C_BLACK); // CLEAR GRAPH ARRAY

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
  simBPM = SIM_BPM;  // initial BPM
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
  simBPM = SIM_BPM + (float)(random(-2, 3)) * 0.5;

  // HR BEEN MOVED TO BE UPDATED WHENEVER THERES A NEED FOR TI
  // HR
  /* 
  tft.fillRect(70, 56, 60, 10, C_BLACK);
  tft.setTextColor(C_CYAN);
  tft.setCursor(70, 56); 
  tft.print((int)simBPM);
  tft.print(" BPM");*/

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
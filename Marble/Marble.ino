// Marble patterns //

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define TFT_DC    43
#define TFT_RST   -1 
#define TFT_CS    44
#define TFT_MOSI  47
#define TFT_SCLK  21
#define TFT_BL    48

#define PWR_LED   3 // power led
#define PIN_BUT   0 // boot button

#define WIDTH   240
#define HEIGHT  240
#define SCR     (WIDTH * HEIGHT)
#define Imax    4
#define IImax   4 

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  uint16_t *col = NULL;
  float a = 20.0f;
  float b = 0.02f;  
  float x, y;

  float FX[2][IImax];
  float FY[2][IImax];

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  for (int II = 0 ; II < IImax ; II++ ){
    FX[0][II] = randomf(0.04f, 0.08f); FX[1][II] = randomf(0.0f, 0.01f);
    FY[0][II] = randomf(0.04f, 0.08f); FY[1][II] = randomf(0.0f, 0.01f) ;
  }

}

void setup(void) {

  pinMode(TFT_BL, OUTPUT);
  pinMode(PWR_LED, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  digitalWrite(PWR_LED, LOW);
  pinMode(PIN_BUT, INPUT_PULLUP);

  srand(time(NULL));

  tft.begin();
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  tft.initDMA();

  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {

      x = i;
      y = j; 
    
      for (int I = 0 ; I < Imax ; I++ ){
        float dx = 0 ;
        float dy = 0 ;
        for (int II = 0 ; II < IImax ; II++ ){
          dx = a * sinf(y*FX[0][II]) * sinf(y*FX[1][II]) + dx;  
          dy = a * sinf(x*FY[0][II]) * sinf(x*FY[1][II]) + dy;
        }
        x = x + dx;
        y = y + dy;
      }
    
      uint8_t coll = 128.0f + (127.0f * sinf((x+y)*b));
      col[i+j*WIDTH] = color565(coll, coll, coll);
      
    }
  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
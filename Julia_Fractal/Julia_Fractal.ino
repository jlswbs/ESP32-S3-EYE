// Julia fractal //

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
#define ITER    300

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float mapfloat(uint16_t x, uint16_t in_min, uint16_t in_max, float out_min, float out_max) { return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  float zoom = 0.5f;
  uint16_t *col = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  zoom = randomf(0.5f, WIDTH);

}

void draw_julia(float c_r, float c_i, float zoom) {

  float new_r = 0.0, new_i = 0.0, old_r = 0.0, old_i = 0.0;

  for(int x = 0; x < WIDTH; x++) {
    for(int y = 0; y < HEIGHT; y++) {     

      old_r = 1.5f * (x - WIDTH / 2) / (0.5f * zoom * WIDTH);
      old_i = (y - HEIGHT / 2) / (0.5f * zoom * HEIGHT);
      uint16_t i = 0;

      while ((old_r * old_r + old_i * old_i) < 4.0f && i < ITER) {
        new_r = old_r * old_r - old_i * old_i ;
        new_i = 2.0f * old_r * old_i;
        old_r = new_r+c_r;
        old_i = new_i+c_i;
        i++;
      }

      if (i < 100) col[x+y*WIDTH] = color565(255,255,map(i,0,50,255,0));
      if(i<200) col[x+y*WIDTH] = color565(255,map(i,0,50,255,0),0);        
      else col[x+y*WIDTH] = color565(map(i,0,50,255,0),0,0);

    }
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

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  draw_julia(-0.8f, 0.156f, zoom);

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
// Eye candy patterns //

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
#define NUMS    64

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  uint16_t *col = NULL;
  float *p = NULL;
  float *v = NULL;
  float *a = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);
  memset(p, 0, 4*SCR);
  memset(v, 0, 4*SCR);
  memset(a, 0, 4*SCR);
  
  for (int i = 0; i < NUMS; i++) v[(1+esp_random()%(WIDTH-2))+(1+esp_random()%(HEIGHT-2))*WIDTH] = randomf(0.0f, TWO_PI);

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

  p = (float*)ps_malloc(4*SCR);
  v = (float*)ps_malloc(4*SCR);
  a = (float*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for (int y = 1; y < HEIGHT-1; y++) {

    for (int x = 1; x < WIDTH-1; x++) {

      a[x+y*WIDTH] = (v[(x-1)+y*WIDTH] + v[(x+1)+y*WIDTH] + v[x+(y-1)*WIDTH] + v[x+(y+1)*WIDTH]) * 0.25f - v[x+y*WIDTH];

    }
    
  }
  
  for (int i = 0; i < SCR; i++){

    v[i] += a[i];
    p[i] += v[i];
    uint8_t coll = 255 * sinf(p[i]);
    col[i] = color565(coll, coll, coll);

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
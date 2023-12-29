// Hopalong chaotic map //

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
#define ITER    5000

float sign(float a) { return (a == 0.0f) ? 0.0f : (a<0.0f ? -1.0f : 1.0f); }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  uint16_t *col = NULL;
  float x = 1.0f;
  float y = 0.0f;
  float a, b, c;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  x = 1.0f;
  y = 0.0f;
  a = expf(randomf(0.1f, 1.0f) * logf(HEIGHT>>2));
  b = expf(randomf(0.1f, 1.0f) * logf(HEIGHT>>2));
  c = randomf(0.1f, 1.0f) * (HEIGHT>>2);

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

  uint16_t coll = esp_random();
  
  for (int i=0; i<ITER; i++) {
    
    float nx = x;
    float ny = y;
        
    x = ny - 1.0f - sqrtf(fabs(b * (nx - 1.0f) - c)) * sign(nx - 1.0f);
    y = a - (nx - 1.0f);

    int ax = constrain((WIDTH/2) + x, 0, WIDTH);
    int ay = constrain((HEIGHT/2) + y, 0, HEIGHT);
      
    if (ax>0 && ax<WIDTH && ay>0 && ay <HEIGHT) col[ax+ay*WIDTH] = coll;
        
  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
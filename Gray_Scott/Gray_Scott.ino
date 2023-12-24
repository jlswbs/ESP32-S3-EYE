// Gray-Scott reaction diffusion //

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
#define ITER    48

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  uint16_t *col = NULL;
  float diffU = 0.16f;
  float diffV = 0.08f;
  float paramF = 0.035f;
  float paramK =  0.06f;
  float *gridU = NULL;  
  float *gridV = NULL;
  float *dU = NULL;
  float *dV = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  diffU = randomf(0.0999f, 0.1999f);
  diffV = randomf(0.0749f, 0.0849f);
  paramF = randomf(0.0299f, 0.0399f);
  paramK = randomf(0.0549f, 0.0649f);

  int seed = esp_random() % (SCR>>4);

  for(int i = 0 ; i < SCR ; i++) {
    
    int px = (i % WIDTH) - (WIDTH / 2);
    int py = (i / HEIGHT) - (WIDTH / 2);

    if(px*px + py*py < seed) {
      gridU[i] = 0.5f * randomf(0.0f, 2.0f);
      gridV[i] = 0.25f * randomf(0.0f, 2.0f);
    } else {
      gridU[i] = 1.0f;
      gridV[i] = 0.0f;
    }
  
  }

}

void timestep(float F, float K, float diffU, float diffV) {

  for (int j = 1; j < HEIGHT-1; j++) {
    for (int i = 1; i < WIDTH-1; i++) {
            
      float u = gridU[i+j*WIDTH];
      float v = gridV[i+j*WIDTH];          
      float uvv = u * v * v;   
      float lapU = (gridU[(i-1)+j*WIDTH] + gridU[(i+1)+j*WIDTH] + gridU[i+(j-1)*WIDTH] + gridU[i+(j+1)*WIDTH] - 4.0f * u);
      float lapV = (gridV[(i-1)+j*WIDTH] + gridV[(i+1)+j*WIDTH] + gridV[i+(j-1)*WIDTH] + gridV[i+(j+1)*WIDTH] - 4.0f * v);
          
      dU[i+j*WIDTH] = diffU * lapU - uvv + F * (1.0f-u);
      dV[i+j*WIDTH] = diffV * lapV + uvv - (K+F) * v;
      gridU[i+j*WIDTH] += dU[i+j*WIDTH];
      gridV[i+j*WIDTH] += dV[i+j*WIDTH];      
          
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

  gridU = (float*)ps_malloc(4*SCR);
  gridV = (float*)ps_malloc(4*SCR);
  dU = (float*)ps_malloc(4*SCR);
  dV = (float*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for (int k = 0; k < ITER; k++) timestep(paramF, paramK, diffU, diffV);

  for(int i = 0; i < SCR; i++){
    
    uint8_t coll = 255.0f * sinf(gridU[i]);
    col[i] = color565(coll, coll, coll);

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
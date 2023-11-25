// Multi scale Turing patterns //

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

#define WIDTH  240
#define HEIGHT 240
#define SCR    (WIDTH * HEIGHT)    

  uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
  float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}
  float sinus[7] = { 0, sinf(TWO_PI/1.0f), sinf(TWO_PI/2.0f),  sinf(TWO_PI/3.0f),  sinf(TWO_PI/4.0f),  sinf(TWO_PI/5.0f),  sinf(TWO_PI/6.0f) };
  float cosinus[7] = { 0, cosf(TWO_PI/1.0f), cosf(TWO_PI/2.0f),  cosf(TWO_PI/3.0f),  cosf(TWO_PI/4.0f),  cosf(TWO_PI/5.0f),  cosf(TWO_PI/6.0f) };

  uint16_t *col = NULL;
  int level, levels, radius;
  int blurlevels, symmetry;
  float base;
  float stepScale;
  float stepOffset;
  float blurFactor;
  uint8_t colOffset;
  uint8_t *radii = NULL;
  float *stepSizes = NULL;
  float *grid = NULL;
  float *blurBuffer = NULL;
  float *bestVariation = NULL;
  uint8_t *bestLevel = NULL;
  bool *direction = NULL;
  float *activator = NULL;
  float *inhibitor = NULL;
  float *swap = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  symmetry = esp_random()%7;
  base = randomf(1.5f, 2.4f);
  stepScale = randomf(0.01f, 0.2f);
  stepOffset = randomf(0.01f, 0.4f);
  blurFactor = randomf(0.5f, 1.0f);

  levels = (int) (log(fmax(WIDTH,HEIGHT)) / logf(base)) - 1.0f;
  blurlevels = (int) fmax(0, (levels+1) * blurFactor - 0.5f);

  for (int i = 0; i < levels; i++) {
    int radius = (int)powf(base, i);
    radii[i] = radius;
    stepSizes[i] = logf(radius) * stepScale + stepOffset; 
  }
    
  for (int i = 0; i < SCR; i++) grid[i] = randomf(-1.0f, 1.0f);

}

int getSymmetry(int i, int w, int h){
  
  if(symmetry <= 1) return i;
  if(symmetry == 2) return SCR - 1 - i;
  int x1 = i % w;
  int y1 = i / w;
  float dx = x1 - w/2.0f;
  float dy = y1 - h/2.0f;
  int x2 = w/2 + (int)(dx * cosinus[symmetry] + dy * sinus[symmetry]);
  int y2 = h/2 + (int)(dx * -sinus[symmetry] + dy * cosinus[symmetry]);
  int j = x2 + y2 * w;
  return j<0 ? j+SCR : j>=SCR ? j-SCR : j;
  
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

  bestLevel = (uint8_t*)ps_malloc(4*SCR);
  grid = (float*)ps_malloc(4*SCR);
  blurBuffer = (float*)ps_malloc(4*SCR);
  bestVariation = (float*)ps_malloc(4*SCR);
  activator = (float*)ps_malloc(4*SCR);
  inhibitor = (float*)ps_malloc(4*SCR);
  swap = (float*)ps_malloc(4*SCR);
  direction = (bool*)ps_malloc(4*SCR);
  stepSizes = (float*)ps_malloc(WIDTH);
  radii = (uint8_t*)ps_malloc(WIDTH);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  if(symmetry >= 1) for(int i = 0; i < SCR; i++) grid[i] = grid[i] * 0.9f + grid[getSymmetry(i, WIDTH, HEIGHT)] * 0.1f;
   
  memcpy(activator, grid, 4*SCR);

  for (level = 0; level < levels - 1; level++) {

    int radius = radii[level];

    if(level <= blurlevels){
        
      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          int t = y * WIDTH + x;
          if (y == 0 && x == 0) blurBuffer[t] = activator[t];
          else if (y == 0) blurBuffer[t] = blurBuffer[t - 1] + activator[t];
          else if (x == 0) blurBuffer[t] = blurBuffer[t - WIDTH] + activator[t];
          else blurBuffer[t] = blurBuffer[t - 1] + blurBuffer[t - WIDTH] - blurBuffer[t - WIDTH - 1] + activator[t];
        }
      }
    }

    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        int minx = max(0, x - radius);
        int maxx = min(x + radius, WIDTH - 1);
        int miny = max(0, y - radius);
        int maxy = min(y + radius, HEIGHT - 1);
        int area = (maxx - minx) * (maxy - miny);

        int nw = miny * WIDTH + minx;
        int ne = miny * WIDTH + maxx;
        int sw = maxy * WIDTH + minx;
        int se = maxy * WIDTH + maxx;

        int t = y * WIDTH + x;
        inhibitor[t] = (blurBuffer[se] - blurBuffer[sw] - blurBuffer[ne] + blurBuffer[nw]) / area;
      }
    }
        
    for (int i = 0; i < SCR; i++) {
      float variation = fabs(activator[i] - inhibitor[i]);
      if (level == 0 || variation < bestVariation[i]) {
        bestVariation[i] = variation;
        bestLevel[i] = level;
        direction[i] = activator[i] > inhibitor[i];
      }
    }

    if(level==0) {
      memcpy(activator, inhibitor, 4*SCR);
    } else {
      memcpy(swap, activator, 4*SCR);
      memcpy(activator, inhibitor, 4*SCR);
      memcpy(inhibitor, swap, 4*SCR);
    }
            
  }

  float smallest = MAXFLOAT;
  float largest = -MAXFLOAT;

  for (int i = 0; i < SCR; i++) {
    float curStep = stepSizes[bestLevel[i]];
    if (direction[i]) {
      grid[i] += curStep;    
    } else {
      grid[i] -= curStep;       
    }
    smallest = fmin(smallest, grid[i]);
    largest = fmax(largest, grid[i]);     
  }

  float range = (largest - smallest) / 2.0f;

  for(int i = 0; i < SCR; i++){     
    grid[i] = ((grid[i] - smallest) / range) - 1.0f;
    uint8_t coll = 128 + (127.0f * grid[i]);
    col[i] = color565(coll, coll, coll);

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
// FitzHugh-Nagumo reaction diffusion //

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

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float randomf(float minf, float maxf) {return minf + (esp_random()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

  uint16_t *col = NULL;
  float reactionRate = 0.2f;
  float diffusionRate = 0.01f;
  float kRate = 0.4f;
  float fRate = 0.09f;  
  float *gridU = NULL;  
  float *gridV = NULL;
  float *gridNext = NULL;
  float *diffRateUYarr = NULL;
  float *diffRateUXarr = NULL;
  float *farr = NULL;
  float *karr = NULL;
  float *temp = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  diffusionRate = randomf(0.01f, 0.05f);
  kRate = randomf(0.1f, 0.5f);
  fRate = randomf(0.04f, 0.09f);
  
  for(int i=0;i<SCR;++i){
      
    gridU[i] = 0.5f + randomf(-0.01f, 0.01f);
    gridV[i] = 0.25f + randomf(-0.01f, 0.01f);

  }

  setupF();
  setupK();

  for(int i=0;i<SCR;++i){
    
    diffRateUYarr[i] = randomf(0.03f, 0.05f);
    diffRateUXarr[i] = randomf(0.03f, 0.05f);
      
  }

}

void diffusionV(){
  
  for(int i=0;i<WIDTH;++i){
    for(int j=0;j<HEIGHT;++j){

      gridNext[i+j*WIDTH] = gridV[i+j*WIDTH]+diffusionRate*4.0f*(gridV[((i-1+WIDTH)%WIDTH)+j*WIDTH]+gridV[((i+1)%WIDTH)+j*WIDTH]+gridV[i+((j-1+HEIGHT)%HEIGHT)*WIDTH]+gridV[i+((j+1)%HEIGHT)*WIDTH]-4.0f*gridV[i+j*WIDTH]);
    
    }
  }
  
  memcpy(temp, gridV, 4*SCR);
  memcpy(gridV, gridNext, 4*SCR); 
  memcpy(gridNext, temp, 4*SCR);
  
}

void diffusionU(){
  
  for(int i=0;i<WIDTH;++i){
    for(int j=0;j<HEIGHT;++j){

      gridNext[i+j*WIDTH] = gridU[i+j*WIDTH]+4.0f*(diffRateUXarr[i+j*WIDTH]*(gridU[((i-1+WIDTH)%WIDTH)+j*WIDTH]+gridU[((i+1)%WIDTH)+j*WIDTH]-2.0f*gridU[i+j*WIDTH])+diffRateUYarr[i+j*WIDTH]*(gridU[i+((j-1+HEIGHT)%HEIGHT)*WIDTH]+gridU[i+((j+1)%HEIGHT)*WIDTH]-2.0f*gridU[i+j*WIDTH]));
    
    }
  }
  
  memcpy(temp, gridU, 4*SCR);
  memcpy(gridU, gridNext, 4*SCR); 
  memcpy(gridNext, temp, 4*SCR);
  
}

void reaction(){

  for(int i=0;i<SCR;++i){

    gridU[i] = gridU[i]+4.0f*(reactionRate*(gridU[i]-gridU[i]*gridU[i]*gridU[i]-gridV[i]+karr[i]));
    gridV[i] = gridV[i]+4.0f*(reactionRate*farr[i]*(gridU[i]-gridV[i]));
    
  }

}

void setupF(){
  
  for(int i=0;i<WIDTH;++i){
  
    for(int j=0;j<HEIGHT;++j) farr[i+j*WIDTH] = 0.01f + i * fRate / WIDTH;
  
  }

}

void setupK(){

  for(int i=0;i<WIDTH;++i){
  
    for(int j=0;j<HEIGHT;++j) karr[i+j*WIDTH] = 0.06f + j * kRate / HEIGHT;
    
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
  gridNext = (float*)ps_malloc(4*SCR);
  diffRateUYarr = (float*)ps_malloc(4*SCR);
  diffRateUXarr = (float*)ps_malloc(4*SCR);
  farr = (float*)ps_malloc(4*SCR);
  karr = (float*)ps_malloc(4*SCR);
  temp = (float*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  diffusionU();
  diffusionV();
  reaction();

  for(int i = 0; i < SCR; i++){

    uint8_t coll = 255.0f * gridU[i];
    col[i] = color565(coll, coll, coll);

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
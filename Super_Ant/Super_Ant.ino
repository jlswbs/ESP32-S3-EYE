// Super Langton's ant cellular automata //

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
#define ITER    240
#define LENGHT  32

  uint16_t *col = NULL;
  uint8_t *state = NULL;
  uint16_t antX = WIDTH/2;
  uint16_t antY = HEIGHT/2;
  uint8_t direction;
  uint8_t stateCount;
  bool type[LENGHT];
  uint16_t stateCols[LENGHT];

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);
  memset((uint8_t *) state, 0, 4*SCR);  

  antX = WIDTH/2;
  antY = HEIGHT/2;
  stateCount = 2 + esp_random()%(LENGHT-2);
  direction = esp_random()%4;
  for(int i = 0; i < stateCount; i++) type[i] = esp_random()%2;
  for(int i = 0; i < stateCount; i++) stateCols[i] = esp_random();

}

void turn(int angle){
  
  if(angle == 0){
    if(direction == 0){
      direction = 3;
    } else {
      direction--;
    }
  } else {
    if(direction == 3){
      direction = 0;
    } else {
      direction++;
    }
  }
}

void move(){
  
  if(antY == 0 && direction == 0){
    antY = HEIGHT-1;
  } else {
    if(direction == 0 ){
      antY--;
    }
  }
  if(antX == WIDTH-1 && direction == 1){
    antX = 0;
  } else {
    if(direction == 1){
      antX++;
    }
  }
  if(antY == HEIGHT-1 && direction == 2){
   antY = 0; 
  } else {
    if(direction == 2){
      antY++;
    }
  }
  if(antX == 0 && direction == 3){
    antX = WIDTH-1;
  } else {
    if(direction == 3){
      antX--;
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

  state = (uint8_t*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for(int i = 0; i < ITER; i++) {

    move();
    turn(type[(state[antX+antY*WIDTH]%stateCount)]);
    state[antX+antY*WIDTH]++;
    col[antX+antY*WIDTH] = stateCols[(state[antX+antY*WIDTH]%stateCount)];

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
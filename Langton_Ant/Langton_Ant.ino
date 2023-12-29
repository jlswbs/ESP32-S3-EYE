// Langton's ant cellular automata //

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

  uint16_t *col = NULL;
  bool *state = NULL;
  int antLoc[2];
  int antDirection;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  memset((bool *) state, 0, 4*SCR);  
  antDirection = 1 + esp_random()%4;
  antLoc[0] = esp_random()%WIDTH;
  antLoc[1] = esp_random()%HEIGHT;

}

void turnLeft(){

  if (antDirection > 1) antDirection--;
  else antDirection = 4;

}

void turnRight(){

  if (antDirection < 4) antDirection++;
  else antDirection = 1;

}

void moveForward(){

  if (antDirection == 1) antLoc[0]--;
  if (antDirection == 2) antLoc[1]++;
  if (antDirection == 3) antLoc[0]++;
  if (antDirection == 4) antLoc[1]--;

  if (antLoc[0] < 0) antLoc[0] = WIDTH-1;
  if (antLoc[0] > WIDTH-1) antLoc[0] = 0;
  if (antLoc[1] < 0) antLoc[1] = HEIGHT-1;
  if (antLoc[1] > HEIGHT-1) antLoc[1] = 0;

}

void updateScene(){

  moveForward();

  if (state[antLoc[0]+antLoc[1]*WIDTH] == 0){
    state[antLoc[0]+antLoc[1]*WIDTH] = 1;
    turnRight();
  } else {
    state[antLoc[0]+antLoc[1]*WIDTH] = 0;
    turnLeft();
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

  state = (bool*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for(int i = 0; i < ITER; i++) updateScene();

  for (int i = 0; i < SCR; i++) {
 
    if(state[i] == 1) col[i] = TFT_WHITE;
    else col[i] = TFT_BLACK;

  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
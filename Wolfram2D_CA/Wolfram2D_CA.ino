// Wolfram 2D cellular automata //

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

  uint16_t *col = NULL;
  bool *state = NULL;
  bool *newstate = NULL;
  bool rules[10];

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  for(int i = 0; i < 10; i++) rules[i] = esp_random()%2;

  memset(newstate, 0, SCR);
  memset(state, 0, SCR);
  
  state[(WIDTH/2)+(HEIGHT/2)*WIDTH] = 1;
  state[(WIDTH/2)+((HEIGHT/2)-1)*WIDTH] = 1;
  state[((WIDTH/2)-1)+((HEIGHT/2)-1)*WIDTH] = 1;
  state[((WIDTH/2)-1)+(HEIGHT/2)*WIDTH] = 1;

}

uint8_t neighbors(int i) {

  uint16_t x = i % WIDTH;
  uint16_t y = i / WIDTH;
  uint8_t result = 0;

  if(y > 0 && state[x+(y-1)*WIDTH] == 1) result = result + 1;
  if(x > 0 && state[(x-1)+y*WIDTH] == 1) result = result + 1;
  if(x < WIDTH-1 && state[(x+1)+y*WIDTH] == 1) result = result + 1;
  if(y < HEIGHT-1 && state[x+(y+1)*WIDTH] == 1) result = result + 1;
  
  return result;
 
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

  state = (bool*)ps_malloc(SCR);
  newstate = (bool*)ps_malloc(SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for(int i = 0; i < SCR; i++){
    
    uint8_t totalNeighbors = neighbors(i);
            
    if(state[i] == 0 && totalNeighbors == 0)      {newstate[i] = rules[0]; col[i] = TFT_WHITE;}
    else if(state[i] == 1 && totalNeighbors == 0) {newstate[i] = rules[1]; col[i] = TFT_RED;}
    else if(state[i] == 0 && totalNeighbors == 1) {newstate[i] = rules[2]; col[i] = TFT_GREEN;}
    else if(state[i] == 1 && totalNeighbors == 1) {newstate[i] = rules[3]; col[i] = TFT_BLUE;}
    else if(state[i] == 0 && totalNeighbors == 2) {newstate[i] = rules[4]; col[i] = TFT_YELLOW;}
    else if(state[i] == 1 && totalNeighbors == 2) {newstate[i] = rules[5]; col[i] = TFT_BLUE;}
    else if(state[i] == 0 && totalNeighbors == 3) {newstate[i] = rules[6]; col[i] = TFT_MAGENTA;}
    else if(state[i] == 1 && totalNeighbors == 3) {newstate[i] = rules[7]; col[i] = TFT_CYAN;}
    else if(state[i] == 0 && totalNeighbors == 4) {newstate[i] = rules[8]; col[i] = TFT_RED;}
    else if(state[i] == 1 && totalNeighbors == 4) {newstate[i] = rules[9]; col[i] = TFT_BLACK;}
      
  }
 
  memcpy(state, newstate, SCR);

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
// Belousov-Zabotinsky reaction cellular automata //

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
  uint8_t *cells = NULL;
  uint8_t *nextcells = NULL;
  int dir[2][4] = {{0, 2, 0, -2},{-2, 0, 2, 0}};

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

  memset((uint8_t *) cells, 0, 4*SCR);
  memset((uint8_t *) nextcells, 0, 4*SCR);

  cells[(esp_random()%WIDTH)+(esp_random()%HEIGHT)*WIDTH] = 1;

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

  cells = (uint8_t*)ps_malloc(4*SCR);
  nextcells = (uint8_t*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for(int y = 0; y < HEIGHT; y=y+2){
    
    for(int x = 0; x < WIDTH; x=x+2){
      
      if(cells[x+y*WIDTH] == 0) col[x+y*WIDTH] = TFT_BLACK;
      else if(cells[x+y*WIDTH] == 1) col[x+y*WIDTH] = TFT_RED;
      else col[x+y*WIDTH] = TFT_WHITE;
      
    }
  }

  for(int y = 0; y < HEIGHT; y=y+2){
    
    for(int x = 0; x < WIDTH; x=x+2){
      
      if(cells[x+y*WIDTH] == 1) nextcells[x+y*WIDTH] = 2;
      else if(cells[x+y*WIDTH] == 2) nextcells[x+y*WIDTH] = 0;
      else {
        
        nextcells[x+y*WIDTH] = 0;
        
        for(int k = 0; k < 4; k++){
          int dx = x + dir[0][k];
          int dy = y + dir[1][k];
          if(0 <= dx && dx < WIDTH && 0 <= dy && dy < HEIGHT && cells[dx+dy*WIDTH] == 1) nextcells[x+y*WIDTH] = 1;
        }
      
      }
    }
  }

  memcpy(cells, nextcells, 4*SCR);

  cells[(esp_random()%WIDTH)+(esp_random()%HEIGHT)*WIDTH] = 1;
  
  delayMicroseconds(240);

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
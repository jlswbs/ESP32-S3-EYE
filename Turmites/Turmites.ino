// Turmites cellular automata //

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
  uint8_t *world = NULL;
  int posx, posy;
  int oldposx, oldposy;
  int state;
  int dir;
  int last_filled;
  int current_col;
  int next_col[4][4];
  int next_state[4][4];
  int directions[4][4];

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);
  memset((uint8_t *) world, 0, 4*SCR);  

  state = esp_random()%4;
  dir = 0;
  posx = WIDTH/2;
  posy = HEIGHT/2;
  
  for(int j=0; j<4; j++){   
    for(int i=0; i<4; i++){         
      next_col[i][j] = esp_random()%4;
      next_state[i][j] = esp_random()%4;
      directions[i][j] = esp_random()%8;
    }   
  }

  world[posx+posy*WIDTH] = esp_random()%4;

}

void move_turmite(){
  
  int cols = world[posx+posy*WIDTH];
  
  oldposx = posx;
  oldposy = posy;
  current_col = next_col[cols][state];
  world[posx+posy*WIDTH] = next_col[cols][state];
  state = next_state[cols][state];    

  dir = (dir + directions[cols][state]) % 8;

  switch(dir){
    case 0: posy--; break;
    case 1: posy--; posx++; break;
    case 2: posx++; break;
    case 3: posx++; posy++; break;
    case 4: posy++; break;
    case 5: posy++; posx--; break;
    case 6: posx--; break;
    case 7: posx--; posy--; break;
  }

  if(posy < 0) posy = HEIGHT-1;
  if(posy >= HEIGHT) posy = 0;
  if(posx < 0) posx = WIDTH-1;
  if(posx >= WIDTH) posx=0;
  
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

  world = (uint8_t*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  for (int i = 0; i < ITER; i++) {

    move_turmite();

    switch(current_col){
      case 0: col[oldposx + oldposy * WIDTH] = TFT_RED; break;
      case 1: col[oldposx + oldposy * WIDTH] = TFT_GREEN; break;
      case 2: col[oldposx + oldposy * WIDTH] = TFT_BLUE; break;
      case 3: col[oldposx + oldposy * WIDTH] = TFT_WHITE; break;
    }
    
  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
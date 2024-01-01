// Conway's game of life cellular automata //

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
  bool *grid = NULL;
  bool current;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);
  memset((bool *) grid, 0, 4*(2*SCR));   
    
  for (int i = 0; i < SCR; i++) grid[i] = esp_random()%2;

}

void rungrid(){
  
  int x, y;
  int count;
  int value = 0;
  bool new_grid;

  new_grid = 1 - current;
  
  for (y = 0; y < HEIGHT; y++) {
  
    for (x = 0; x < WIDTH; x++) {
      
      count = neighbours(x, y);
      
      if (count < 2 || count > 3) { value = 0; }
      else if (count == 3) { value = 3; }
      else { value = grid[(current*SCR)+(x+y*WIDTH)]; }
    
      grid[(new_grid*SCR)+(x+y*WIDTH)] = value;

      if(grid[(current*SCR)+(x+y*WIDTH)]) col[x+y*WIDTH] = TFT_WHITE;
      else col[x+y*WIDTH] = TFT_BLACK;
    
    }
  }
  
  current = new_grid;

}

int neighbours(int x, int y){
  
  int i, j;
  int result = 0;

  x--;
  y--;
  
  for (i = 0; i < 3; i++) {
  
    if (y < 0 || y > (HEIGHT - 1)) continue;

    for (j = 0; j < 3; j++) {
      if (x < 0 || x > (WIDTH - 1)) continue;
      if (i==1 && j == 1) { x++; continue; }
      if (grid[(current*SCR)+(x+y*WIDTH)]) result++;
      x++;
    }
    y++;
    x -= 3;
  }
  
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

  grid = (bool*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  rungrid();

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
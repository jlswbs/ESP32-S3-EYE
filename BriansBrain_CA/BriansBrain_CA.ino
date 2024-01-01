// Brian's brain cellular automata //

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
#define DENSITY     7
#define READY       0
#define REFRACTORY  1
#define FIRING      2

  uint16_t *col = NULL;
  uint8_t *world = NULL;
  uint8_t *temp = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

uint8_t weighted_randint(int true_weight){
  
    int choice = esp_random() % 10;
    
    if (choice > true_weight) return 1;
    else return 0;
}

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);
  memset((uint8_t *) temp, 0, 4*SCR);

  for (int i = 0; i < SCR; i++){
    int r = weighted_randint(DENSITY);
    if (r == 1) world[i] = FIRING;
    else world[i] = READY;
  }

}

uint8_t count_neighbours(uint8_t world[SCR], int x_pos, int y_pos){
  
    int x, y, cx, cy, cell;
    int count = 0;

    for (y = -1; y < 2; y++) {
        for (x = -1; x < 2; x++) {
            cx = x_pos + x;
            cy = y_pos + y;
            if ( (0 <= cx && cx < WIDTH) && (0 <= cy && cy < HEIGHT)) {
                cell = world[(x_pos + x) + (y_pos + y) * WIDTH];
                if (cell == FIRING) count ++;
            }
        }
    }
  return count;
}


void apply_rules(uint8_t world[SCR]){
  
  int cell, neighbours;

  memcpy(temp, world, 4*SCR);

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++){
      cell = temp[x+y*WIDTH];          
      if (cell == READY) {
        neighbours = count_neighbours(temp, x, y);
        if (neighbours == 2) world[x+y*WIDTH] = FIRING; }
      else if (cell == FIRING) world[x+y*WIDTH] = REFRACTORY;
      else world[x+y*WIDTH] = READY;
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

  world = (uint8_t*)ps_malloc(4*SCR);
  temp = (uint8_t*)ps_malloc(4*SCR);
  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

  rndrule();

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  apply_rules(world);

  for (int i = 0; i < SCR; i++) {
    if (world[i] == FIRING) col[i] = TFT_BLUE;    
    else if (world[i] == REFRACTORY) col[i] = TFT_WHITE;
    else col[i] = TFT_BLACK; 
  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
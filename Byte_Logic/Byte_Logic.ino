// ByteBeat logical patterns //

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
  long t, cnt;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;


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

  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

}

void loop() {

  memset((uint16_t *) col, 0, 4*SCR);

  for (int y = 0; y < HEIGHT; ++y) {

    for (int x = 0; x < WIDTH; ++x){

      uint16_t coll = cnt + (t^t>>10) - (t|t>>9);
      col[(coll%WIDTH) + y * WIDTH] = coll;

      t++;

    }

  }

  cnt++;
  delay(120);

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
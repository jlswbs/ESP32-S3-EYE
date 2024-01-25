// Terrain perlin noise //

#include <TFT_eSPI.h>
#include <FastLED.h>
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

  uint16_t x_noise = 0;
  uint16_t y_noise = 0;
  uint16_t y_noise_speed = 5;
  uint16_t x_noise_speed = 5;

  uint8_t terrain_noise[WIDTH][HEIGHT];
  uint8_t feature[WIDTH][HEIGHT];

  uint16_t *col = NULL;

  uint16_t *dmaBuffer = NULL;
  uint16_t *dmaBufferPtr = dmaBuffer;

void rndrule(){

  memset((uint16_t *) col, 0, 4*SCR);

}

uint8_t combined_noise(int x_offset, int y_offset) {

    uint8_t zeroNoise = inoise16(x_offset * 450, y_offset * 450) >> 8;
    uint8_t primaryNoise = inoise16(x_offset * 650, y_offset * 650) >> 8;
    uint8_t fineNoise = inoise16(x_offset * 1050, y_offset * 1050) >> 8;
    uint8_t finerNoise = inoise16(x_offset * 2048, y_offset * 2048) >> 8;
    return ((zeroNoise / 4) + (primaryNoise / 4) + (fineNoise / 4) + (finerNoise / 4));

}
enum FeatureType {
    NONE,
    TREE,
    FLOWER
};

void fill_noise16() {

  for (int y = 0; y < HEIGHT; y++) {

    int y_offset = y_noise + y;
    for (int x = 0; x < WIDTH; x++) {
      int x_offset = x_noise + x;
      uint8_t noise = combined_noise(x_offset, y_offset);
      terrain_noise[x][y] = noise;
      uint8_t randomVal = esp_random();
      if (noise > 150 && noise < 152 || noise > 155 && noise < 158) feature[x][y] = TREE;
      else feature[x][y] = NONE;
    }

  }

    y_noise += y_noise_speed;
    x_noise += x_noise_speed;

}

const uint8_t oceanThreshold = 130;
const uint8_t desertThreshold = 135;
const uint8_t landThreshold = 150;
const uint8_t mountainThreshold = 195;
const uint8_t highMountainThreshold = 210;

uint16_t get_color(uint8_t noiseValue, int x, int y) {

  uint8_t features = feature[x][y];
  if (features == TREE) return TFT_DARKGREY;
  else if (features == FLOWER) return TFT_RED;
  else if (noiseValue < oceanThreshold) {
    uint8_t randomVal = esp_random();
    if (randomVal < 10) return TFT_SKYBLUE;
    return TFT_BLUE;
  } 
  else if (noiseValue < desertThreshold) return TFT_YELLOW;
  else if (noiseValue < landThreshold) return TFT_GREEN;
  else if (noiseValue < mountainThreshold) return TFT_LIGHTGREY;
  else if (noiseValue < highMountainThreshold) return TFT_YELLOW;
  else return TFT_WHITE;

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

  col = (uint16_t*)ps_malloc(4*SCR);
  dmaBuffer = (uint16_t*)ps_malloc(16*16);

}

void loop() {

  if(digitalRead(PIN_BUT) == false) rndrule();

  fill_noise16();

  x_noise_speed++;
  y_noise_speed++;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      col[x+y*WIDTH] = get_color(terrain_noise[x][y], x, y);
    }
  }

  tft.startWrite();
  tft.pushImageDMA(0, 0, WIDTH, HEIGHT, (uint16_t *)col, dmaBufferPtr);
  tft.endWrite();

}
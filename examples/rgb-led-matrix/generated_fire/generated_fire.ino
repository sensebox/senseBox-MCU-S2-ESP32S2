
// taken from here https://github.com/mic159/NeoFire/tree/master and adjusted
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

#define PIN 2
#define WIDTH 12
#define HEIGHT 8

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, HEIGHT, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[HEIGHT][WIDTH]={
    {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  },
    {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  },
    {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  },
    {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  },
    {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 },
    {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 },
    {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 },
    {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128}
};

//these are the hues for the fire, 
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[HEIGHT][WIDTH]={
    {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25},
    {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19},
    {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16},
    {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13},
    {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11},
    {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 },
    {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 },
    {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 }
};

unsigned char matrixValue[HEIGHT][WIDTH];
unsigned char line[WIDTH];
int pcnt = 0;

//Converts an HSV color to RGB color
uint16_t HSVtoRGB(uint8_t ih, uint8_t is, uint8_t iv) {
  float r, g, b, h, s, v; //this function works with floats between 0 and 1
  float f, p, q, t;
  int i;

  h = (float)(ih / 256.0);
  s = (float)(is / 256.0);
  v = (float)(iv / 256.0);

  //if saturation is 0, the color is a shade of grey
  if(s == 0.0) {
    b = v;
    g = b;
    r = g;
  }
  //if saturation > 0, more complex calculations are needed
  else
  {
    h *= 6.0; //to bring hue to a number between 0 and 6, better for the calculations
    i = (int)(floor(h)); //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
    f = h - i;//the fractional part of h

    p = (float)(v * (1.0 - s));
    q = (float)(v * (1.0 - (s * f)));
    t = (float)(v * (1.0 - (s * (1.0 - f))));

    switch(i)
    {
      case 0: r=v; g=t; b=p; break;
      case 1: r=q; g=v; b=p; break;
      case 2: r=p; g=v; b=t; break;
      case 3: r=p; g=q; b=v; break;
      case 4: r=t; g=p; b=v; break;
      case 5: r=v; g=p; b=q; break;
      default: r = g = b = 0; break;
    }
  }
  return matrix.Color(r * 255.0, g * 255.0, b * 255.0);
}

/**
 * Randomly generate the next line (matrix row)
 */
void generateLine(){
  for(uint8_t x=0; x<WIDTH; x++) {
    line[x] = random(64, 255);
  }
}

/**
 * shift all values in the matrix up one row
 */
void shiftUp() {
  for (uint8_t y=HEIGHT-1; y>0; y--) {
    for (uint8_t x=0; x<WIDTH; x++) {
      matrixValue[y][x] = matrixValue[y-1][x];
    }
  }
  
  for (uint8_t x=0; x<WIDTH; x++) {
    matrixValue[0][x] = line[x];
  }
}

/**
 * draw a frame, interpolating between 2 "key frames"
 * @param pcnt percentage of interpolation
 */
void drawFrame(int pcnt) {
  int nextv;
  
  //each row interpolates with the one before it
  for (unsigned char y=HEIGHT-1; y>0; y--) {
    for (unsigned char x=0; x<WIDTH; x++) {
      nextv = 
          (((100.0-pcnt)*matrixValue[y][x] 
        + pcnt*matrixValue[y-1][x])/100.0) 
        - valueMask[y][x];
      uint16_t color = HSVtoRGB(
        hueMask[y][x], // H
        255, // S
        (uint8_t)max(0, nextv) // V
      );

      matrix.drawPixel(x, y, color);
    }
  }
  
  //first row interpolates with the "next" line
  for(unsigned char x=0; x<WIDTH; x++) {
    uint16_t color = HSVtoRGB(
      hueMask[0][x], // H
      255,           // S
      (uint8_t)(((100.0-pcnt)*matrixValue[0][x] + pcnt*line[x])/100.0) // V
    );
    matrix.drawPixel(x, 0, color);
  }
}

void setup() {
  matrix.setBrightness(30);
  matrix.begin();
  randomSeed(analogRead(0));
  generateLine();
  //init all pixels to zero
  memset(matrixValue, 0, sizeof(matrixValue));
}

void loop() {
  if (pcnt >= 100) {
    shiftUp();
    generateLine();
    pcnt = 0;
  }
  drawFrame(pcnt);
  matrix.show();
  pcnt+=30;
  delay(5);
}
#include "LPD8806.h"
#include "SPI.h"

// Definitions:

#define COLOR_WHITE  strip.Color(127, 127, 127)
#define COLOR_GREEN  strip.Color(0,   127, 0  )
#define COLOR_BLUE   strip.Color(0,   0,   127)
#define COLOR_RED    strip.Color(127, 0,   0  )
#define COLOR_YELLOW strip.Color(127, 127, 0  )
#define COLOR_CYAN   strip.Color(0,   127, 127)
#define COLOR_PURPLE strip.Color(127, 0,   127)
#define COLOR_OFF    strip.Color(0,   0,   0  )

/*****************************************************************************/

// Number of RGB LEDs in strand:
int nLEDs = 32;
int bars[13];   // for incoming serial data
int temp;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

void setup() {
  // Start up the LED strip
  strip.begin();
  Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
  Serial.println("Serial Monitor Connected");
  // Set all bars to zero before starting communication
  for(int i=0;i<13;i++)
  {
    bars[i] = 0;
  }
  // Update the strip, to start they are all 'off'
  strip.show();
}


void loop() {
  
  int i,j;
  ///////////////////
  /* LED-only test //
  ///////////////////
  
  // Send a simple pixel chase in...
  colorChase(COLOR_WHITE, 50); // White
  colorChase(strip.Color(127,   0,   0), 50); // Red
  colorChase(strip.Color(127, 127,   0), 50); // Yellow
  colorChase(strip.Color(  0, 127,   0), 50); // Green
  colorChase(strip.Color(  0, 127, 127), 50); // Cyan
  colorChase(strip.Color(  0,   0, 127), 50); // Blue
  colorChase(strip.Color(127,   0, 127), 50); // Violet

  // Fill the entire strip with...
  colorWipe(strip.Color(127,   0,   0), 50);  // Red
  colorWipe(strip.Color(  0, 127,   0), 50);  // Green
  colorWipe(strip.Color(  0,   0, 127), 50);  // Blue
  rainbow(10);
  rainbowCycle(0);  // make it go through the cycle fairly fast
  
  ///////////////////////////
  *///End of LED-only test //
  ///////////////////////////
  
  ///////////////////
  // SPI-Read Code //
  ///////////////////
  for(i=0; i<13; i++)
  {
    if (Serial.available() > 0) 
    {
      // read the incoming byte:
      //bars[i] = Serial.read();
      bars[i] = Serial.parseInt();
      Serial.print("Received: ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(bars[i]);
    }
    else
    {
      if (Serial.available() > 0) 
      {
        // read the incoming byte:
        //bars[i] = Serial.read();
        bars[i] = Serial.parseInt();
        Serial.print("Received: ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(bars[i]);
      }
    }
  }
  /* //Error checking code
  if(Serial.available() > 0) 
  {
    temp = Serial.parseInt();
    if(temp != 255)
    {
      Serial.println("Umm...");
      // If we get here, something went wrong- print a color outside of the normal range of the single-strip test
      strip.setPixelColor(18,COLOR_WHITE);
    }
  }*/
  ///////////////////////////
  ////End of SPI-Read Code //
  ///////////////////////////
  
  //////////////////////////////////
  // Single-strip value test code //
  //////////////////////////////////
  
  // Use this code to test the range of values from the FFT
  // Assign one group of 3 LEDs for each bar and test the value based on color
  for(i=0; i<13; i++)
  {
    if(bars[i] == 0)
    {
      strip.setPixelColor(i,COLOR_RED);
    }
    else if(bars[i] < 40)
    {
      strip.setPixelColor(i,COLOR_YELLOW);
    }
    else if(bars[i] < 60)
    {
      strip.setPixelColor(i,COLOR_GREEN);
    }
    else if(bars[i] < 80)
    {
      strip.setPixelColor(i,COLOR_CYAN);
    }
    else if(bars[i] < 100)
    {
      strip.setPixelColor(i,COLOR_BLUE);
    }
    else if(bars[i] < 120)
    {
      strip.setPixelColor(i,COLOR_PURPLE);
    }
    else if(bars[i] < 140)
    {
      strip.setPixelColor(i,COLOR_WHITE);
    }
  }
  
  /////////////////////////////////////
  //// End of single-strip test code //
  /////////////////////////////////////
  
  //////////////////////////
  /* 13-Strip Output Code //
  //////////////////////////
  
  for(i=0; i<13; i++)
  {
    if(bars[i] < 20)
    {
      for(j=0; j<2; j++)
      {
        strip[i].setPixelColor(j,COLOR_RED);
      }
      for(j=2; j<nLEDs; j++)
      {
        strip[i].setPixelColor(j,COLOR_WHITE);
      }
    }
    else if(bars[i] < 40)
    {
    }
  }
  
  ///////////////////////////
  */// 13-Strip Output End //
  ///////////////////////////
}

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 384; j++) {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  
  for (j=0; j < 384 * 5; j++) {     // 5 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 384-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 384 / strip.numPixels()) + j) % 384) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Fill the dots progressively along the strip.
void colorWipe(uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// Chase one dot down the full strip.
void colorChase(uint32_t c, uint8_t wait) {
  int i;

  // Start by turning all pixels off:
  for(i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, 0);

  // Then display one pixel at a time:
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c); // Set new pixel 'on'
    strip.show();              // Refresh LED states
    strip.setPixelColor(i, 0); // Erase pixel, but don't refresh!
    delay(wait);
  }

  strip.show(); // Refresh to turn off last pixel
}

/* Helper functions */

//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}

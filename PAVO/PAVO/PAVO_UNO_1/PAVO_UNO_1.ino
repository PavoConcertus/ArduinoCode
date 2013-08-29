#include "fix_fft.h"
#include "LPD8806.h"
#include "SPI.h"

#define COLOR_WHITE   strip.Color(127,127,127)
#define COLOR_RED     strip.Color(127,0,0)
#define COLOR_GREEN   strip.Color(0,127,0)
#define COLOR_BLUE    strip.Color(0,0,127)
#define COLOR_CYAN    strip.Color(0,127,127)
#define COLOR_YELLOW  strip.Color(127,127,0)
#define COLOR_PURPLE  strip.Color(127,0,127)

#define DIM_WHITE     strip.Color(63,63,63)
#define DIM_RED       strip.Color(63,0,0)
#define DIM_GREEN     strip.Color(0,63,0)
#define DIM_BLUE      strip.Color(0,0,63)
#define DIM_CYAN      strip.Color(0,63,63)
#define DIM_YELLOW    strip.Color(63,63,0)
#define DIM_PURPLE    strip.Color(63,0,63)

#define FADE_B2G      strip.Color(0,c,(127-c))
#define FADE_G2B      strip.Color(0,(127-c),c)
#define FADE_Y2C      strip.Color((127-c),127,c)
#define FADE_C2Y      strip.Color(c,127,(127-c))
#define FADE_R2P      strip.Color(127,0,c)
#define FADE_P2R      strip.Color(127,0(127-c))

#define DIM_B2G      strip.Color(0,(c/2),(63-(c/2)))
#define DIM_G2B      strip.Color(0,(63-(c/2)),(c/2))
#define DIM_Y2C      strip.Color((63-(c/2)),63,(c/2))
#define DIM_C2Y      strip.Color((c/2),63,(63-(c/2)))
#define DIM_R2P      strip.Color(63,0,(c/2))
#define DIM_P2R      strip.Color(63,0(63-(c/2)))
 

// Number of RGB LEDs in strand:
int nLEDs = 180;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// Arrays used for FFT
char im[128];
char data[128]; 
char shortdata[13];

// Define Pins
int adcPin = 0;

// Counters used in colour fading
int c;
boolean c_rising;

void setup()
{
  strip.begin();
  Serial.begin(9600);
  strip.show();
  c_rising = true;
}

void loop()
{
  // FFT Code
  
  int static i = 0;
  static long tt;
  int val;
   
   if (millis() > tt)
   {
      if (i < 128)
      {
        val = analogRead(adcPin);
        data[i] = val / 4 - 128;
        im[i] = 0;
        i++;   
        
      }
      else
      {
        //this could be done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
        
        // I am only interessted in the absolute value of the transformation
        for (i=0; i< 64;i++){
           data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
        }
        
        // reduce the data down to 13 bars
        for(i=0;i<13;i++)
        {
          // Average 5 bars of the 64
          shortdata[i] = (data[(5*i)] + data[(5*i)+1] + data[(5*i)+2] + data[(5*i)+3] + data[(5*i)+4])/5;
          
          //Ignore upper 25 channels because they didn't react during the radio test
          //shortdata[i] = (data[(3*i)] + data[(3*i)+1] + data[(3*i)+2])/3;
          
          // Write FFT results to serial
          //Serial.write(shortdata[i]); // send the bar over serial
          //Serial.print("Bar ");
          //Serial.print(i);
          //Serial.print(" : ");
          Serial.println((int)shortdata[i]); // Print data
          //Serial.println(" ");
        }
        // 180 / 13 = 13.85 LED triplets per bar
        // However, strip can only be cut into groups of two, so
	// 12 LED triplets per bar will be used
       
        // Manage color fading variables
	if(c_rising && c < 127)
	{
		c++;
	}
	else if(c >= 127)
	{
		c--;
		c_rising = false;
	}
	if(!c_rising && c > 0)
	{
		c--;
	}
	else if(c <= 0)
	{
		c++;
		c_rising = true;
	}

        
        // For each bar,
        for(int i=0; i < 13 ; i++)
        {
          if(shortdata[i] < 5)
          {
		// Turn on one LED triplet
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,DIM_G2B);
		strip.setPixelColor(12*i+2,DIM_G2B);
		strip.setPixelColor(12*i+3,DIM_G2B);
		strip.setPixelColor(12*i+4,DIM_Y2C);
		strip.setPixelColor(12*i+5,DIM_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 10)
          {
                // Turn on two LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,DIM_G2B);
		strip.setPixelColor(12*i+3,DIM_G2B);
		strip.setPixelColor(12*i+4,DIM_Y2C);
		strip.setPixelColor(12*i+5,DIM_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 20)
          {
                // Turn on three LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,DIM_G2B);
		strip.setPixelColor(12*i+4,DIM_Y2C);
		strip.setPixelColor(12*i+5,DIM_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 40)
          {
                // Turn on four LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,DIM_Y2C);
		strip.setPixelColor(12*i+5,DIM_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 50)
          {
                // Turn on five LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,DIM_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 60)
          {
                // Turn on six LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,DIM_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 70)
          {
                // Turn on seven LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,DIM_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 80)
          {
                // Turn on eight LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,FADE_Y2C);
		strip.setPixelColor(12*i+8,DIM_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 90)
          {
                // Turn on nine LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,FADE_Y2C);
		strip.setPixelColor(12*i+8,FADE_R2P);
		strip.setPixelColor(12*i+9,DIM_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 100)
          {
             // Turn on ten LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,FADE_Y2C);
		strip.setPixelColor(12*i+8,FADE_R2P);
		strip.setPixelColor(12*i+9,FADE_R2P);
		strip.setPixelColor(12*i+10,DIM_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else if(shortdata[i] < 110)
          {
                // Turn on eleven LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,FADE_Y2C);
		strip.setPixelColor(12*i+8,FADE_R2P);
		strip.setPixelColor(12*i+9,FADE_R2P);
		strip.setPixelColor(12*i+10,FADE_R2P);
		strip.setPixelColor(12*i+11,DIM_R2P);
          }
          else
          {
                // Turn on ALL LED triplets
	        strip.setPixelColor(12*i,FADE_G2B);
		strip.setPixelColor(12*i+1,FADE_G2B);
		strip.setPixelColor(12*i+2,FADE_G2B);
		strip.setPixelColor(12*i+3,FADE_G2B);
		strip.setPixelColor(12*i+4,FADE_Y2C);
		strip.setPixelColor(12*i+5,FADE_Y2C);
		strip.setPixelColor(12*i+6,FADE_Y2C);
		strip.setPixelColor(12*i+7,FADE_Y2C);
		strip.setPixelColor(12*i+8,FADE_R2P);
		strip.setPixelColor(12*i+9,FADE_R2P);
		strip.setPixelColor(12*i+10,FADE_R2P);
		strip.setPixelColor(12*i+11,FADE_R2P);
          }
        }
        strip.show();
      }
    
    tt = millis();
   }
   // end of FFT code
   
  
}

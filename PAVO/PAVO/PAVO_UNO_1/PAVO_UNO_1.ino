// Include files for libraries
#include "fix_fft.h"
#include "LPD8806.h"
#include "SPI.h"

// Solid, bright colors (Full Intensity)
#define COLOR_WHITE   strip.Color(127,127,127)
#define COLOR_RED     strip.Color(127,0,0)
#define COLOR_GREEN   strip.Color(0,127,0)
#define COLOR_BLUE    strip.Color(0,0,127)
#define COLOR_CYAN    strip.Color(0,127,127)
#define COLOR_YELLOW  strip.Color(127,127,0)
#define COLOR_PURPLE  strip.Color(127,0,127)

// Solid, dim colors (Half Intensity)
#define DIM_WHITE     strip.Color(63,63,63)
#define DIM_RED       strip.Color(63,0,0)
#define DIM_GREEN     strip.Color(0,63,0)
#define DIM_BLUE      strip.Color(0,0,63)
#define DIM_CYAN      strip.Color(0,63,63)
#define DIM_YELLOW    strip.Color(63,63,0)
#define DIM_PURPLE    strip.Color(63,0,63)

// Fading bright colors (you can make other ones if you follow the pattern I've demonstrated here:
// c starts at 0 and rises to 127, and then goes back to zero again)
#define FADE_B2G      strip.Color(0,c,(127-c))
#define FADE_G2B      strip.Color(0,(127-c),c)
#define FADE_Y2C      strip.Color((127-c),127,c)
#define FADE_C2Y      strip.Color(c,127,(127-c))
#define FADE_R2P      strip.Color(127,0,c)
#define FADE_P2R      strip.Color(127,0(127-c))

// Fading dim colors used for the negative space. Dim colors fade on a different scale which is why these are
// divided by 8. You could make them even dimmer by dividing by 16 and using 8 instead of 15.
/*
#define DIM_B2G      strip.Color(0,(c/8),(15-(c/8)))
#define DIM_G2B      strip.Color(0,(15-(c/8)),(c/8))
#define DIM_Y2C      strip.Color((15-(c/8)),15,(c/8))
#define DIM_C2Y      strip.Color((c/8),15,(15-(c/8)))
#define DIM_R2P      strip.Color(15,0,(c/8))
#define DIM_P2R      strip.Color(15,0(15-(c/8)))
*/

// This is a quick and ditry way to turn all dim colors to OFF (as requested by Alan)
// Comment this block out and uncomment one of the other DIM blocks to go back to dim negative space
#define DIM_B2G     0 
#define DIM_G2B     0
#define DIM_Y2C     0
#define DIM_C2Y     0
#define DIM_R2P     0
#define DIM_P2R     0

// These are sort-of-dim fading colors. Not curently being used in the visualization.
#define MID_B2G      strip.Color(0,(c/2),(63-(c/2)))
#define MID_G2B      strip.Color(0,(63-(c/2)),(c/2))
#define MID_Y2C      strip.Color((63-(c/2)),63,(c/2))
#define MID_C2Y      strip.Color((c/2),63,(63-(c/2)))
#define MID_R2P      strip.Color(63,0,(c/2))
#define MID_P2R      strip.Color(63,0(63-(c/2)))


// Total number of RGB LED triplets (180 for our application)
int nLEDs = 180;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// Arrays used for FFT
char im[128];          //Imaginary values, used in calculation
char data[128];        //Output of FFT, only the first 64 values are populated (the rest is used in calculation)
char shortdata[13];    //Output of FFT squished into 13 values via averaging

// Define Pins
int adcPin = 0;

// Counters used in colour fading
int c;
boolean c_rising;

void setup()
{
  //Initialize LPD strip object
  strip.begin();
  //Intitialize serial communication for output testing
  Serial.begin(9600);
  //Clear all LEDs
  strip.show();
  //Set the fade variable to start rising
  c_rising = true;
}

void loop()
{
  // This is an approximate sine wave, used for testing
  // When I tested with it, I didn't get a lot of response
  // but that was before I knew that the FFT only returns 0-15ish magnitude
  /*
  int sine256[]  = {
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,223,225,227,229,231,233,234,236,238,239,240,
  242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,
  221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
  76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
  33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124

};
  */
  
  // FFT Code starts here
  // Variables used in timing and calculation:
  int static i = 0;
  static long tt;
  int val;
  int x=0;
  
  // As often as possible:
   if (millis() > tt)
   {
     // Get 128 samples from the analog pin
      if (i < 128)
      {
        // Store the analog pin value in "val"
        val = analogRead(adcPin);
        // Here we scale it so we can have a low voltage input
        val = val*5;
        
        //Uncomment the line below to print the ADC value to the serial monitor
        // Serial.println(val);
        
        // Prepare the data for FFT
        data[i] = val / 4 - 128;
        im[i] = 0;
        i++;   
        
      }
      // Once 128 samples have been gathered,
      else
      {
        //Run the FFT
        fix_fft(data,im,7,0);
        
        // Get the absolute value of each data element
        for (i=0; i< 64;i++){
           data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
        }
        
        // Reduce the data down to 13 bars
        // Average 5 bars of the 64 (ignore data[0] because it's noisy)
        shortdata[0] = (data[1] + data[2] + data[3] + data[4] + data[5])/5;
        shortdata[1] = (data[6] + data[7] + data[8] + data[9] + data[10])/5;
        shortdata[2] = (data[11] + data[12] + data[13] + data[14] + data[15])/5;
        shortdata[3] = (data[16] + data[17] + data[18] + data[19] + data[20])/5;  
        shortdata[4] = (data[21] + data[22] + data[23] + data[24] + data[25])/5;
        shortdata[5] = (data[26] + data[27] + data[28] + data[29] + data[30])/5;
        shortdata[6] = (data[31] + data[32] + data[33] + data[34] + data[35])/5;
        shortdata[7] = (data[36] + data[37] + data[38] + data[39] + data[40])/5;
        shortdata[8] = (data[41] + data[42] + data[43] + data[44] + data[45])/5;
        shortdata[9] = (data[46] + data[47] + data[48] + data[49] + data[50])/5;
        shortdata[10] = (data[51] + data[52] + data[53] + data[54] + data[55])/5;
        shortdata[11] = (data[56] + data[57] + data[58] + data[59] + data[60])/5;
        shortdata[12] = (data[61] + data[62] + data[63])/3;
        
        // Scale all but the first bar (as requested by Isabel)
        // You can tweak the value for different results
        for(i=1;i<13;i++)
        {
          shortdata[i] = shortdata[i]*1.3;
        }
          
          
          // This block is used for printing out all 13 bars of FFT output with colons separating them
          // It's commented out for now because I thought it might be slowing down the lights
         /*
          Serial.print(" : ");
          Serial.print((int)shortdata[0]);
          Serial.print(" : ");
          Serial.print((int)shortdata[1]);
          Serial.print(" : ");
          Serial.print((int)shortdata[2]);
          Serial.print(" : ");
          Serial.print((int)shortdata[3]);
          Serial.print(" : ");
          Serial.print((int)shortdata[4]);
          Serial.print(" : ");
          Serial.print((int)shortdata[5]);
          Serial.print(" : ");
          Serial.print((int)shortdata[6]);
          Serial.print(" : ");
          Serial.print((int)shortdata[7]);
          Serial.print(" : ");
          Serial.print((int)shortdata[8]);
          Serial.print(" : ");
          Serial.print((int)shortdata[9]);
          Serial.print(" : ");
          Serial.print((int)shortdata[10]);
          Serial.print(" : ");
          Serial.print((int)shortdata[11]);
          Serial.print(" : ");
          Serial.print((int)shortdata[12]);
          Serial.println(" ");
      */
      
        // 180 / 13 = 13.85 LED triplets per bar
        // However, strip can only be cut into groups of two, so
	// 12 LED triplets per bar will be used
       
        // Manage color fading variables
        // Increment c until it hits 127
	if(c_rising && c < 127)
	{
		c++;
	}
        // At which point start dropping it back to zero
	else if(c >= 127)
	{
		c--;
		c_rising = false;
	}
        // Decrement c until it hits zero
	if(!c_rising && c > 0)
	{
		c--;
	}
        // At which point start raising it toward 127 again
	else if(c <= 0)
	{
		c++;
		c_rising = true;
	}

        
        // For each bar,
        for(int i=0; i < 13 ; i++)
        {
          // For each of these 'if' statements, you can change the value to
          // assign different cutoff values for the segments of light
          if(shortdata[i] < 1)
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
          else if(shortdata[i] < 2)
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
          else if(shortdata[i] < 4)
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
          else if(shortdata[i] < 5)
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
          else if(shortdata[i] < 6)
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
          else if(shortdata[i] < 7)
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
          else if(shortdata[i] < 8)
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
          else if(shortdata[i] < 9)
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
          else if(shortdata[i] < 10)
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
          else if(shortdata[i] < 11)
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
          else if(shortdata[i] < 12)

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
        // After all the changes have been made, update the strip
        // You MUST have this line in order for the strip to update
        strip.show();
      }
    
    // This is a timing thingy, don't touch plz
    tt = millis();
   }
}

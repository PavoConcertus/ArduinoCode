// Include files for libraries
#include "fix_fft.h"

// Arrays used for FFT
char im[128];         // Imaginary array for calculations
char data[128];       // Data output of FFT. First 64 elements are populated, the rest is just there to aid calculations
char shortdata[13];   // Output of FFT squashed into 13 bars.

// Define Pins
int adcPin = 0;

void setup()
{
  pinMode(0,INPUT); // Sets pin 0 as an input for a switch. This will be useful if you want to have 2 different modes
  // Initialize serial communication so we can view things in the serial montior
  Serial.begin(9600);
  // Turn off all LEDs
}

void loop()
{
  // FFT Code
  
  // These variables are used in timing and calculation
  int static i = 0;
  static long tt;
  int val;
   
   // As often as possible,
   if (millis() > tt)
   {
     // Grab 128 samples
      if (i < 128)
      {
        // Read in from analog pin
        val = analogRead(adcPin);
        data[i] = val / 4 - 128;
        im[i] = 0;
        i++;   
        
      }
      else
      {
        //Calculate FFT
        fix_fft(data,im,7,0);
        
        // Get absolute value of data
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
        
        // Pitch mode if the switch is OFF
        if(digitalRead(0) == LOW)
        {
          int greatest = 0;
          // Find the greatest bar
          for(int i=1; i<13; i++)
          {
            if(shortdata[i] > shortdata[greatest])
            {
              greatest = i;
            }
          }
        
          // Turn off all pipes
          for(int i=0; i < 13; i++)
          {
            digitalWrite(i,LOW);
          }
        
          // Turn on the pipe whose frequency has the greatest magnitude
          digitalWrite(greatest,HIGH);
        }
        // Else, beat mode
        else 
        {
          // If beat detected, (you can change this number until it works reasonably well)
          if(shortdata[i] > 20)
          {
            // Turn on all pipes
            for(int i=1; i < 14; i++)
            {
              digitalWrite(i,HIGH);
            }
            
          }
          else
          {
            // Else, Turn off all pipes
            for(int i=1; i < 14; i++)
            {
              digitalWrite(i,LOW);
            }
          }
        }
      }
    // Timing thing, do not eat
    tt = millis();
   }
   // end of FFT code
   
  
}

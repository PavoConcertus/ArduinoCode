#include "fix_fft.h"

// Arrays used for FFT
char im[128];
char data[128]; 
char shortdata[13];

// Define Pins
int adcPin = 0;

void setup()
{
  Serial.begin(9600);
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
          Serial.println((int)shortdata[i]); // Send data to Rx Pin of the DUE
          //Serial.println(" ");
        }
        
        /*
        for(i=0;i<30;i++)
        {
          Serial.print(" : ");
          Serial.print((int)data[i]);
        }*/
        
        //Serial.println(255);
        // After 13 bars have been sent, send a check bit
        // This is to ensure that the reads don't get out of sync somehow
        //Serial.write(255);
      }
    
    tt = millis();
   }
   // end of FFT code
   
  
}

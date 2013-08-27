/*
void setup() 
{
  Serial.begin(9600);
}
void loop() 
{
  int incoming;
  
  if (Serial.available()) {
    delay(1);
    
    while (Serial.available() > 0)
    {
      incoming=Serial.read();
      Serial.print(incoming);
      Serial.println(" received!");
    }
  }										

}*/

/*
For more details see: http://projectsfromtech.blogspot.com/

Connect the Rx pin of this board to the Tx of the board running Serial_Comm_Tx.ino
Connect the Grounds of the two boards
Open Serial Monitor

Receives integer value and prints it to the serial monitor
*/
int val;

void setup()
{
  Serial.begin(9600);
  Serial.println("Serial Monitor Connected");
}

void loop()
{
  int incoming = Serial.available();
  
  if (incoming > 0)
  {
   val = Serial.parseInt();  //Reads integers as integer rather than ASCI. Anything else returns 0
   Serial.println(val*2);
  }
}


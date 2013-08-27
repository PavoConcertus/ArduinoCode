/*
void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly: 
  Serial.println(22);
  delay(1000);
}*/

/*
For more details see: http://projectsfromtech.blogspot.com/

Connect the Tx pin of this board to the Rx of the board running Serial_Comm_Rx.ino
Connect the Grounds of the two boards

Transmits numbers 0-10 to the Rx board every second
Blinks the LED on pin 13 when every number is tranmitted
*/

const int led = 13;
void setup()
{
  Serial.begin(9600);
  //pinMode(led, OUTPUT);
}

void loop()
{
 for (int i = 0; i <= 10; i++)
 {
   Serial.println (i); //Send i to Rx Arduino
   //digitalWrite(led, HIGH);
   //delay(100);
   //digitalWrite(led, LOW);
   delay(1000);   
 }
}


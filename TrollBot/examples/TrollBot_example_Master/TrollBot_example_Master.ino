/*
This example requires 3 arduinos, 3 nRF24 chips, a potentiometer and an LED.
This sketch is the Master node. Upload the two other examples to the two other arduinos, 
one called Alf and the other Bob. An LED connected to pin 6 on Alf shall be dimmed by a 
potentiometer connected to pin A0 on Bob. The code controlling Alf and Bob is run on this 
master node.

Connect the nRF24 chip as follows: 
GND-GND
VCC-3.3V
CE-8
CS-10
SCK-13
MOSI-11
MISO-12

Remember to have the following libraries in your library folder:
RF24Network.h
RF24.h
SPI.h
Servo.h
*/

#include<TrollBot.h>                // Include the library

// Initiate the nodes. The master is always 00, the child of master can be 01-05, and
// the child of 01 can be 011-051...
TrollBot Master(00 /*address*/,100 /*channel*/);
TrollBot Alf(01,100);
TrollBot Bob(02,100);

void setup() {
  Master.setup();                   // Run the setup functions for the master
  
  Alf.pinMode(6,OUTPUT);            // Set pin 6 as output on Alf
}

void loop() {
  Master.masterLoop();              // The masterLoop function must be run regularly
  
  int pot=Bob.analogRead(A0);       // Reads and stores the potentiometer value from Bob
  int light=map(pot,0,1023,0,255);  // Map the value
  Alf.analogWrite(6,light);         // Write the value onto the LED on Alf
}

/*
Code comparison of arduino based networking tools for prototyping wirelessly communicating prototypes

Scenario: Capture sensor data (potentiometer) from one unit and send it to another unit for further processing (mapping the value 
and using it for fading an LED).
Measure: number of essential lines of code to achieve the functionality described above. We don't count the line if it is purely 
for visual or debug purposes, e.g. we don't count a line containing a single curly bracket, a pin number definition, or messages 
printed to the serial output for debug purposes.
*/


//***************************nRF24 with TrollBot
//Master node
#include "TrollBot.h"

TrollBot Master(00, 100);
TrollBot Alf(01,100);

void setup() {
	Master.setup();
}

void loop() {
	Master.masterLoop();
	int pot = Alf.analogRead(A0);
	int light = map(pot, 0, 1023, 0, 255);
	analogWrite(6, light);
}//Lines = 10

//Child node "Alf"
#include"TrollBot.h"

TrollBot Alf(01,100);

void setup() {
	Alf.setup();
}

void loop() {
	Loop();
}//Lines = 6
//Lines total = 16

//***************************nRF24
//Master node (receive data)
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(8, 10);
RF24Network network(radio);

void setup(){
	SPI.begin();
	radio.begin();
	network.begin(100, 00);
}

void loop(){
	network.update();
	RF24NetworkHeader fromHeader;
	network.peek(fromHeader);
	uint16_t receiveData;
	if (fromHeader.from_node == 01) {
		network.read(fromHeader, &receiveData, sizeof(receiveData));
	
	int light = map(receiveData, 0, 1023, 0, 255);
	
	analogWrite(6, light);
	}
}//Lines = 18

//Child node 02 (read and send sensor data)
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(8, 10);
RF24Network network(radio);

void setup(){
	SPI.begin();
	radio.begin();
	network.begin(100, 01);
	
	toHeader = RF24NetworkHeader(00);
}

void loop(){
	network.update();
	if (network.available())
	{
		int pot = analogRead(A0);
		network.write(toHeader, &pot, sizeof(pot));
	}
}//Lines = 15
//Lines total = 33


//***************************ESP8266, with examples from https://www.instructables.com/id/WiFi-Communication-Between-Two-ESP8266-Based-MCU-T/
//The examples are changed so they function as the scenario explained above

//Client (receive sensor data)
/*  NodeMCU, OLED 128x64 I2C
 *  Receives temperature data from the server and shows it on an OLED screen
 *  Server: DTH_server_02.ino
 */ 
/*  NodeMCU, OLED 128x64 I2C
    Receives temperature data from the server and shows it on an OLED screen
    Server: DTH_server_02.ino
*/
#include <SPI.h>
#include <ESP8266WiFi.h>

IPAddress server(192, 168, 0, 80);  // fix IP of the server
WiFiClient client;

//int status = WL_IDLE_STATUS;
char ssid[] = "yourSSIDHere";
char pass[] = "yourPasswordHere";
byte xc = 1, yc = 23, dx = 1, dy = 1;
unsigned long askTimer = 0;
unsigned long oledTimer = 0;
String answer;
uint8_t pinD1 = 5;    // I2C Bus SCL (clock)
uint8_t pinD2 = 4;    // I2C Bus SDA (data)
//uint8_t pinD3 = 0;
//uint8_t pinD4 = 2;


void setup() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    //    Serial.print(".");
    delay(500);
  }
}

void loop () {
  if (millis() - askTimer > 2340) {         // time between two connection to the server
    client.connect(server, 80);             // connects to the server
    client.println("Haliho szerver!\r");    // trigger message to the server, its value is scrapped
    answer = client.readStringUntil('\r');  // received the server's answer
    client.flush();
    askTimer = millis();

    int potVal = answer.toInt();
    int light = map(potVal, 0, 1023, 0, 255);
    analogWrite(6, light);
  }
}//Lines = 16

//Server (read and send sensor data)
/* Wemos D1 mini, OLED 128x64 I2C, DHT22
 * Sends the temperature value to the client(s)
 * v2: server restart after 30 sec idle time
 */
#include <SPI.h>                    // SD
#include <ESP8266WiFi.h>

WiFiServer server(80);              // launches the server
IPAddress ip(192, 168, 0, 80);      // fix IP of the server
IPAddress gateway(192,168,0,1);     // WiFi router's IP
IPAddress subnet(255,255,255,0);

char ssid[] = "yourSSIDHere";
char pass[] = "yourPasswordHere";
byte xc = 1, yc = 23, dx = 1, dy = 1;
unsigned long DHTtimer = 0;
int pot;
unsigned long clientTimer = 0;

void setup() {
  server_start(0);                            // starts the WiFi server
  delay(2000);
}

void loop() {
  if (millis() > DHTtimer + 2000) {
    pot = analogRead(A0);
    if (isnan(pot)) {
      return;
    } else {
      DHTtimer = millis();
    }
  }
  WiFiClient client = server.available();
  if (client) {
    if (client.connected()) {
      String request = client.readStringUntil('\r');    // reads the message from the client
      client.flush();
      client.println(String(pot));        // sends the temperature to the client
    }
    client.stop();                         // disconnects the client
    clientTimer = millis();
  }
  if (millis() - clientTimer > 30000) {    // stops and restarts the WiFi server after 30 sec
    WiFi.disconnect();                     // idle time
    delay(500);
    server_start(1);
  }
}

void server_start(byte restart) {
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();
  delay(500);
  clientTimer = millis();
}//Lines = 25
//Lines total = 41



//***************************HC-05 and HC-06 Bluetooth, with examples from http://www.martyncurrey.com/arduino-to-arduino-by-bluetooth/
//The examples are changed so they function as the scenario explained above

//First we need to pair the devices. Example from http://www.martyncurrey.com/connecting-2-arduinos-by-bluetooth-using-a-hc-05-and-a-hc-06-pair-bind-and-link/

//HC-05 Set Up
// Basic Bluetooth sketch HC-05_02_38400
// Connect the HC-05 module and communicate using the serial monitor
//
// The HC-05 defaults to commincation mode when first powered on.
// Needs to be placed in to AT mode
// After a factory reset the default baud rate for communication mode is 38400
//
//
//  Pins
//  BT VCC to Arduino 5V out. 
//  BT GND to GND
//  BT RX to Arduino pin 3 (through a voltage divider)
//  BT TX to Arduino pin 2 (no need voltage divider)
 
 
#include <SoftwareSerial.h>
SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-05 TX to Arduino pin 2 RX. 
// Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider.
 
void setup() 
{
    // start th serial communication with the host computer
    Serial.begin(9600);
    Serial.println("Arduino with HC-05 is ready");
 
    // start communication with the HC-05 using 38400
    BTserial.begin(38400);  
    Serial.println("BTserial started at 38400");
}
 
void loop()
{
 
     // Keep reading from HC-05 and send to Arduino Serial Monitor
    if (BTserial.available())
    {  
        Serial.write(BTserial.read());
    }
 
    // Keep reading from Arduino Serial Monitor and send to HC-05
    if (Serial.available())
    {
 
        // mirror the commands back to the serial monitor
        // makes it easy to follow the commands
        Serial.write(Serial.read());   
        BTserial.write(Serial.read());  
    }
}
//Commands to type in the serial monitor:
//AT+PSWD1234
//AT+RMAAD
//AT+ROLE=1
//AT+RESET
//AT+CMODE=0
//AT+INQM=0,5,9
//AT+INIT
//AT+INQ
//AT+PAIR=<addr>,<timeout>
//AT+CMODE=1
//Setup lines = 21


//HC-06 setup
// Basic bluetooth test sketch. HC-06_01_9600
// HC-06 ZS-040 
// 
// 
//  Uses hardware serial to talk to the host computer and software serial for communication with the bluetooth module
//
//  Pins
//  BT VCC to Arduino 5V out. 
//  BT GND to GND
//  BT RX to Arduino pin 3 (through a voltage divider)
//  BT TX to Arduino pin 2 (no need voltage divider)
//
//  When a command is entered in the serial monitor on the computer 
//  the Arduino will relay it to the bluetooth module and display the result.
//
//  These HC-06 modules require capital letters and no line ending
//
 
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(2, 3); // RX | TX
 
void setup() 
{
    Serial.begin(9600);
    Serial.println("Arduino with HC-06 is ready");
 
    // HC-06 default baud rate is 9600
    BTSerial.begin(9600);  
    Serial.println("BTserial started at 9600");
}
 
void loop()
{
 
  // Keep reading from HC-06 and send to Arduino Serial Monitor
  if (BTSerial.available())
    Serial.write(BTSerial.read());
 
  // Keep reading from Arduino Serial Monitor and send to HC-06
  if (Serial.available())
  BTSerial.write(Serial.read());
}
//Commands to type in the serial monitor:
//AT+PSWD1234
//AT+BIND=<addr>,<timeout>
//AT+LINK=<addr>
//Setup lines = 14


//Now that the two units connect to each other automatically, we can program them for the scenario.
//Slave (receive data)
/*
* Sketch: Arduino2Arduino_SLAVE_01
* By Martyn Currey
* 08.04.2016
* Written in Arduino IDE 1.6.3
*
* Receive commands through a serial connection and turn a LED on or OFF
* There is no error checking and this sketch receives only
* Commands should be contained within the start and end markers < and >
*
* D8 - software serial RX
* D9 - software serial TX
* D3 - LED
*
*/
 
// AltSoftSerial uses D9 for TX and D8 for RX. While using AltSoftSerial D10 cannot be used for PWM.
// Remember to use a voltage divider on the Arduino TX pin / Bluetooth RX pin
// Download AltSoftSerial from https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
 
#include <AltSoftSerial.h>
AltSoftSerial BTserial; 
 
// Variables used for incoming data
 
const byte maxDataLength = 20;          // maxDataLength is the maximum length allowed for received data.
char receivedChars[maxDataLength+1] ;
boolean newData = false;               // newData is used to determine if there is a new command
 
void setup()  
{
    //  open software serial connection to the Bluetooth module.
    BTserial.begin(9600);
 
    newData = false;
 
} // void setup()
 
 
 
void loop()  
{
         recvWithStartEndMarkers();                // check to see if we have received any new commands
         if (newData)  {
			newData = false;
			String data = receivedChars;
			int pot = atoi(receivedChars);
			int light = map(pot, 0, 1023, 0, 255);
			analogWrite(6, light);
		 }    // if we have a new command do something
}
 
// function recvWithStartEndMarkers by Robin2 of the Arduino forums
// See  http://forum.arduino.cc/index.php?topic=288234.0
void recvWithStartEndMarkers() 
{
     static boolean recvInProgress = false;
     static byte ndx = 0;
     char startMarker = '<';
     char endMarker = '>';
 
     if (BTserial.available() > 0) 
     {
          char rc = BTserial.read();
          if (recvInProgress == true) 
          {
               if (rc != endMarker) 
               {
                    if (ndx < maxDataLength) { 
						receivedChars[ndx] = rc; ndx++; 
					}
               }
               else 
               {
                     receivedChars[ndx] = '\0'; // terminate the string
                     recvInProgress = false;
                     ndx = 0;
                     newData = true;
               }
          }
          else if (rc == startMarker) { recvInProgress = true; }
     }
 
}//Lines = 26

//Master (read and send sensor data)
/*
* Sketch: Arduino2Arduino_MASTER_01
* By Martyn Currey
* 08.04.2016
* Written in Arduino IDE 1.6.3
*
* Send commands through a serial connection to turn a LED on and OFF on a remote Arduino
* There is no error checking and this sketch sends only
* Commands should be contained within the start and end markers < and >
*
* D8 - AltSoftSerial RX
* D9 - AltSoftSerial TX
*
*/
 
// AltSoftSerial uses D9 for TX and D8 for RX. While using AltSoftSerial D10 cannot be used for PWM.
// Remember to use a voltage divider on the Arduino TX pin / Bluetooth RX pin
// Download AltSoftSerial from https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
 
#include <AltSoftSerial.h>
AltSoftSerial BTserial; 
 
void setup()  
{
    //  open software serial connection to the Bluetooth module.
    BTserial.begin(9600); 
 
} // void setup()
 
 
void loop()  
{
	int pot = analogRead(A0);
	BTserial.print('<');
 	BTserial.print(pot);
	BTserial.println('>');
}//Lines = 9
//Lines total = 70


//***************************XBee, with examples from https://www.ardumotive.com/how-to-use-xbee-modules-as-transmitter--receiver-en.html
//The examples are changed so they function as the scenario explained above

//First we need to pair the devices. XBee has its own program for this step. The following must be set to pair the devices:

//Transmitter
//Set the CH field
//Set the ID field
//Set the CE field

//Receiver
//Set the CH field
//Set the ID field
//Set the CE field

//Setup lines = 6

//Read and send sensor data
/* 	~ Simple Arduino - xBee Transmitter sketch ~

	Read an analog value from potentiometer, then convert it to PWM and finally send it through serial port to xBee.
	The xBee serial module will send it to another xBee (resiver) and an Arduino will turn on (fade) an LED.
	The sending message starts with '<' and closes with '>' symbol. 
	
	Dev: Michalis Vasilakis // Date:2/3/2016 // Info: www.ardumotive.com // Licence: CC BY-NC-SA                    */

//Constants: 
const int potPin = A0; //Pot at Arduino A0 pin 
//Variables:
int value ; //Value from pot

void setup() {
	//Start the serial communication
	Serial.begin(9600); //Baud rate must be the same as is on xBee module
}

void loop() {
  //Read the analog value from pot and store it to "value" variable
 	value = analogRead(A0);
	//Send the message:
	Serial.print('<'); 	//Starting symbol
 	Serial.print(value);//Value from 0 to 255
	Serial.println('>');//Ending symbol
}//Lines = 7

//Receive sensor data
/* 	~ Simple Arduino - xBee Receiver sketch ~

	Read an PWM value from Arduino Transmitter to fade an LED
	The receiving message starts with '<' and closes with '>' symbol.
	
	Dev: Michalis Vasilakis // Date:2/3/2016 // Info: www.ardumotive.com // Licence: CC BY-NC-SA                    */

//Variables
bool started= false;//True: Message is strated
bool ended 	= false;//True: Message is finished 
char incomingByte ; //Variable to store the incoming byte
char msg[4];		//Message - array from 0 to 3 (4 values, 0-1023)
byte index;			//Index of array

void setup() {
	//Start the serial communication
	Serial.begin(9600); //Baud rate must be the same as is on xBee module
}

void loop() {
	
  while (Serial.available()>0){
  	//Read the incoming byte
    incomingByte = Serial.read();
    //Start the message when the '<' symbol is received
    if(incomingByte == '<')
    {
     started = true;
     index = 0;
     msg[index] = '\0'; // Throw away any incomplete packet
   }
   //End the message when the '>' symbol is received
   else if(incomingByte == '>')
   {
     ended = true;
     break; // Done reading - exit from while loop!
   }
   //Read the message!
   else
   {
     if(index < 5) // Make sure there is room
     {
       msg[index] = incomingByte; // Add char to array
       index++;
       msg[index] = '\0'; // Add NULL to end
     }
   }
 }
 
 if(started && ended)
 {
   int value = atoi(msg);
   int light = map(value, 0, 1023, 0, 255);
   analogWrite(6, light);
   //Serial.println(value); //Only for debugging
   index = 0;
   msg[index] = '\0';
   started = false;
   ended = false;
 }
}//Lines = 25
//Lines total = 38

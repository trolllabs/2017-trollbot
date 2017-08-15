/*
*26.04.2017*
TrollBot library, made in the course Fuzzy Front End for the TrollBot project at NTNU in 2017.

This library is based on the great work by J. Coliz, his libraries RF24.h and RF24Network.h can be obtained from 
https://github.com/nRF24, and the nRF24L01 chip by Nordic Semiconductor ASA.

The inteded use for this library is as follows: a master node can controll other nodes with simple function calls, 
similar to the already known standard functions. The other nodes runs the same basic sketch, which automagically 
does what the master is requesting.

Besides a few necessary lines of code, the master can controll other nodes by simply giving them a name, e.g. Alf. When 
we want node Alf to set pin 5 as output and set it to high, the master must write Alf.pinMode(5, OUTPUT) and 
Alf.digitaWrite(5, HIGH). Please refer to the example sketches for a simple demonstration.

This code is by no means a finished product. It's still a working prototype.
*/

/*
* The following libraries are needed.
*/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Servo.h>

/*
* A radio and network object is created to access their functions. CE and CS are set to pins 8 and 10.
* Also 4 servo objects are predefined.
*/
RF24 radio(8, 10);
RF24Network network(radio);
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

struct TrollBot {

	/*
	* The following variables are set for each node in the network.
	*/
	uint16_t nodeAddress;		// The logical address for this node in Octal format
	uint8_t channel;			// Channels from 0-124 can be used (2.400-2.524 GHz)
	RF24NetworkHeader header;	// This header is sent with each message (see RF24Network.h)

	
	/* 
	* Constructor for setting the node address, channel and network header. The header contains information about the 
	* node, and is needed whenever messages are sent or received. The default address is set to 90.
	*/
	TrollBot(uint16_t nodeAddress, uint8_t channel = 90): nodeAddress(nodeAddress), channel(channel) {
		header= RF24NetworkHeader(nodeAddress);
	}
	
	/*
	*  This code is used in the setup to start radio communication.
	*/
	void setup(){
		SPI.begin();
		radio.begin();
		network.begin(channel, nodeAddress);
	}

	/*
	* network.update() must be run regularly to receive, send and forward messages.
	* Must be used directy on the loop of the master node.
	*/
	void masterLoop() {
		network.update();
	}

	/*
	* The following functions use the same principle. The array 'data' contains three values. The first value determines
	* which function the receiving node will run. The next values are the input values used in the function.
	* When this data is received by a node, it is handled by a function which is not a member of this struct. It's
	* explained further down.
	*/
	void digitalWrite(int pin, int mode) {
		uint8_t data[3] = { 10,pin,mode };
		network.write(header, &data, sizeof(data));
	}

	void analogWrite(int pin, int val) {
		uint8_t data[3] = { 11,pin,val };
		network.write(header, &data, sizeof(data));
	}

	/*
	* Receiving data is a bit different. After the request is sent, a delay is found necessary before receiving the
	* data. A header is constructed for the incoming data. If the address of the incoming data matches the address
	* which the request was sent to, the value is returend. 
	* NB! Otherwise a default value of 0 is returned if not specified.
	*/
	int digitalRead(int pin, int defaultReturn = 0) {
		uint8_t data[3] = { 12,pin,0 };
		uint16_t receiveData;
		network.write(header, &data, sizeof(data));
		delay(20);
			RF24NetworkHeader fromHeader;
			network.peek(fromHeader);
			if (fromHeader.from_node == nodeAddress) {
				network.read(fromHeader, &receiveData, sizeof(receiveData));
				return receiveData;
			}
			else return defaultReturn;
	}

	int analogRead(int pin, int defaultReturn = 0) {
		uint8_t data[3] = { 13,pin,0 };
		uint16_t receiveData;
		network.write(header, &data, sizeof(data));
		delay(20);
			RF24NetworkHeader fromHeader;
			network.peek(fromHeader);
			if (fromHeader.from_node == nodeAddress) {
				network.read(fromHeader, &receiveData, sizeof(receiveData));
				return receiveData;
			}
			else return defaultReturn;
	}

	void pinMode(int pin, int mode) {
		uint8_t data[3] = { 14,pin,mode };
		network.write(header, &data, sizeof(data));
	}

	void servo_attach(int servoNum, int pin) {
		uint8_t data[3] = { 15,servoNum,pin };
		network.write(header, &data, sizeof(data));
	}

	void servo_write(int servoNum, int angle) {
		uint8_t data[3] = { 16,servoNum,angle };
		network.write(header, &data, sizeof(data));
	}
};


/*
* The loop function running on the child nodes continuously. 
*
* If there is data available in the network for this node, the data is read and stored. The first value determine which 
* functions to run. The next values are then used by the function.
*
*  If there is incoming data intended for another node, network.update() forwards the data. 
*/
void Loop(int sendToThisAddress = 00) {
	network.update();
	if (network.available()) {
		RF24NetworkHeader toHeader(sendToThisAddress);
		RF24NetworkHeader fromHeader;
		uint8_t data[3];
		uint16_t transmitData = 0;
		network.read(fromHeader, &data, sizeof(data));

		switch (data[0]) {
		case 10: digitalWrite(data[1], data[2]); break;
		case 11: analogWrite(data[1], data[2]); break;
		case 12: transmitData = digitalRead(data[1]);
			network.write(toHeader, &transmitData, sizeof(transmitData)); break;
		case 13: transmitData = analogRead(data[1]);
			network.write(toHeader, &transmitData, sizeof(transmitData)); break;
		case 14: pinMode(data[1], data[2]); break;
		case 15: switch (data[1]) {
			case 1: servo1.attach(data[2]); break;
			case 2: servo2.attach(data[2]); break;
			case 3: servo3.attach(data[2]); break;
			case 4: servo4.attach(data[2]); break;
		}break;
		case 16: switch (data[1]) {
			case 1: servo1.write(data[2]); break;
			case 2: servo2.write(data[2]); break;
			case 3: servo3.write(data[2]); break;
			case 4: servo4.write(data[2]); break;
		}break;

		default: network.read(fromHeader, 0, 0); break;
		}
	}
}

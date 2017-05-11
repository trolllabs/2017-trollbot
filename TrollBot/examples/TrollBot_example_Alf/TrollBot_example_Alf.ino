#include"TrollBot.h"

TrollBot Alf(01,100);     // Initiate this node with address 01 and channel 100

void setup() {
  Alf.setup();            // Run the setup functions for this node
}

void loop() {
  Loop();                 // Run the loop function for this node
}

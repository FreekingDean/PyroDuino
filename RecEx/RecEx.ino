#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <RF24_config.h>

/* 
 * This is a simple arduino module set to be a transmitter, currently for
 * firework Efuse Slats && holiday lightshow spectacular.
 * Author: Dean Galvin
 * Creation Date: 12-9-2013
 * Last Updated: 1-22-2014
 * Version 0.01
 */
 
//I attempt to make all constants up
//outside the code below.

//Setup RDF24 pins SPI 50-52 
RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

byte slatState[16] = {0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00};
// The role of the current running sketch
role_e role = role_pong_back;
bool verbose = true;
bool debug = false;
bool info = true;
// the setup routine runs once when you press reset:
void setup() {  
  Serial.begin(57600);
  
  radio.begin();
  radio.setRetries(0, 15);
  radio.setPayloadSize(8);
  radio.startListening();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
}

// the loop routine runs over and over again forever:
void loop() {
  if (radio.available()) {
    byte bufByte[3];
    readTran(bufByte);
  }
  delay(10);
}

void readTran(byte buffer[]) {
  radio.read(buffer, 3);
  radio.stopListening();
  switch (buffer[0]) {
    case 5:
      Serial.println("Ping Pong");
      break;
    case 0:
      setOffFirework(buffer[1], buffer[2]);
      break;
    case 1:
      setLightOn(buffer[1], buffer[2]);
      break;
    case 2:
      setLightOff(buffer[1], buffer[2]);
      break;
  }
  radio.write(buffer, 3);
  radio.startListening();
  printByte(buffer);
}

void setLightOn(int slat, int slot) {
  char toPrint[32];
  sprintf(toPrint, "Lighting On @ Slat:%02d, Slot: %02d", slat, slot);
  printi(toPrint);
  toggleSlot(slat, slot, HIGH);
}

void setLightOff(int slat, int slot) {
  char toPrint[33];
  sprintf(toPrint, "Lighting Off @ Slat:%02d, Slot: %02d", slat, slot);
  printi(toPrint);
  toggleSlot(slat, slot, LOW);
}

void setOffFirework(int slat, int slot) {
  char toPrint[31];
  sprintf(toPrint, "Firing off @ Slat:%02d, Slot: %02d", slat, slot);
  printi(toPrint);
  toggleSlot(slat, slot, HIGH);
  delay(10);
  toggleSlot(slat, slot, LOW);
}

void toggleSlot(int slat, int slot, int tgl) {
  char toPrint[30];
  sprintf(toPrint, "Setting slat:%02d slot:%02d to %02d", slat, slot, tgl);
  printv(toPrint);
  slat = (int) slat/2;
  slat *=2;
  slot > 7 ? slat+=1:slat;
  slot = slot % 8;
  byte toShift = (byte) pow(2, slot);
  toShift > 2 ? toShift++:toShift;
  Serial.println(toShift);
  if(tgl == HIGH)
    slatState[slat] = slatState[slat] | toShift;
  else{
    toShift = 0xFF - toShift;
    slatState[slat] = slatState[slat] & toShift;
  }
  while(slat>=0) {
    //shiftOut(CLK, LATCH, DATA, slatState[slat]);
    char toPrint[5];
    sprintf(toPrint, "0x%02X ", slatState[slat]);
    Serial.print(toPrint);
    slat--;
  }
  Serial.println();
}

void printv(char toOut[]) {
  if(verbose)
    Serial.println(toOut);
}

void printi(char toOut[]) {
  if(info)
    Serial.println(toOut);
}
void printByte(byte bufByte[]) {
  if(debug) {
    for(int i=0; i<3; i++) {
      char buffer[4];
      sprintf(buffer, "0x%02x ",bufByte[i]);
      Serial.print(buffer);
    }
    Serial.println();
  }
}

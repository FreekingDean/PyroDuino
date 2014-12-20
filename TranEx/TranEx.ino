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
//Radio pipe addresses for the 2 nodes to communicate.
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

// The role of the current running sketch
role_e role = role_pong_back;

int pingWait = 1000;
int pingTime = pingWait-1;
long pingSent = 0;
int redPin = 2;
int greenPin = 3;
int bluePin = 4;
bool connected = false;
bool timedOut = false;
 
byte pingout[] = {0x05, 0x00, 0x00};
// the setup routine runs once when you press reset:
void setup() {  
  Serial.begin(57600);
  
  radio.begin();
  radio.setRetries(0, 15);
  radio.setPayloadSize(8);
  radio.startListening();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}
 
// the loop routine runs over and over again forever:
void loop() {
  pingTime++;
  if (pingTime >= pingWait) {
    sendPing();
  }
  if (radio.available()) {
    byte recByte [3];
    getTransmission(recByte);
  }
  if (Serial.available() >= 3) {
    getSerial();
  }
  delay(10);
}

void pingTimeout() {
  Serial.println("PING TIMED OUT!");
  timedOut = true;
  connected = false;
  pingWait=1000;
  pingTime = 0;
  setRGB(0, 255, 255);
}

void pingConnect() {
  connected = true;
  Serial.println("Got connection!");
  timedOut=true;
  pingWait = 10000;
  pingTime = 0;
  setRGB(255, 0, 255);
}

void sendPing() {
  Serial.println("Pinging...");
  pingout[1] = millis() % 256;
  sendTrans(pingout, true);
  pingTime = 0;
  timedOut = false;

}

void getTransmission(byte ret[]) {
  byte recBytes[3];
  radio.read(&recBytes, sizeof(recBytes));
  printBytes(recBytes[0], recBytes[1], recBytes[2]);
  ret[0] = recBytes[0];
  ret[1] = recBytes[1];
  ret[2] = recBytes[2];
}

void getSerial() {
  // read the incoming byte:
  byte trans[3];
  for(int i=0; i < 3; i++) {
    byte incomingByte = Serial.read();
    incomingByte -= 48;
    trans[i] = incomingByte;
  }
  if (trans[0] == 5 && trans[1] == 4 && trans [2] == 2) {
    Serial.println("Synthetic Timeout");
    pingTimeout();
  } else {
    sendTrans(trans, false);
  }
}

void sendTrans(byte transmission[], bool isPing) {
  if(connected || isPing) {
    setRGB(255, 255, 0);
    long timeSend = millis();
    radio.stopListening();
    radio.write(transmission, 3);
    radio.startListening();
    while(!radio.available()) {
      if(millis() > timeSend+1000) {
        pingTimeout();
        return;
      }
    }
    byte recByte [3];
    getTransmission(recByte);
    if(transmission[0] == recByte[0] && transmission[1] == recByte[1] && transmission [2] == recByte[2]) {
      pingConnect();
    } else {
      pingTimeout();
    }
    printBytes(recByte[0], recByte[1], recByte[2]);
    printByteTran(transmission);
  }
}

void setRGB(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void printBytes(byte bOne, byte bTwo, byte bThree) {
  byte printBytes[] = {bOne, bTwo, bThree};
  printByteTran(printBytes);
}

void printByteTran(byte trans[]) {
  for(int i=0; i<3; i++) {
    //char buffer[4];
    //sprintf(buffer, "0x%02x ",trans[i]);
    Serial.print(trans[i]);
  }
  Serial.println();
}

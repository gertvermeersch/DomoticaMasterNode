#include <NRF905.h>
#include <domotica.h>
#include <SPI.h>
#include <NewRemoteTransmitter.h>

/* Main program for the Master node.
 * Receives commands over Serial in the following order:
 * 1st 4 bytes: destination address
 * 2nd 4 bytes: type of message: COMD, STAT, REQT...
 * 3th 4 bytes: parameter: SW0N, SW0F, TOGL, TEMP, REDL, BLUE, GREN
 * 4th 20 bytes: value
 */
 
NewRemoteTransmitter transmitter(12376190, 6, 260, 3);
Domotica controller;
//NRF905 nrf;
char testmsg[MSG_LEN];
long sent = 0;
volatile long received = 0;
char serialBuffer[32];
char response[32];
bool state0 = false;
bool state1 = false;
bool state2 = false;


void receivedMessage() {
 char* ptrbuffer;
 Serial.println("interrupt!");
 ptrbuffer = controller.getMsg();
 for(int i = 0; i<32;i++) {
   Serial.print(ptrbuffer[i],DEC);
 }
 received++; 
}

void setup() {
  Serial.begin(115200); //max baud rate
  //nrf = NRF905();
  controller = Domotica();
  controller.setDebug(true);
  controller.init(0); 
  attachInterrupt(0,receivedMessage,RISING);
  transmitter.sendUnit(0,false);
  transmitter.sendUnit(1,false);
  transmitter.sendUnit(2,false);
}

void loop() {
  while(!Serial.available()); //wait until a message is sent
  int i = 0;
  delay(1);
  Serial.println("receiving");
  while(Serial.available() > 0 && i < 32) {
      serialBuffer[i] = Serial.read();
      //Serial.print(Serial.available(), HEX);
      if(serialBuffer[i] == '\n' || serialBuffer[i] == '\r') {
        serialBuffer[i] = '\0';
        break;
      }
      i++;
  }
  //Serial.println(serialBuffer);
  parseSerialBuffer();
}

void parseSerialBuffer() {
  Serial.println("Parsing message");
  if(serialBuffer[4] == 'R' && serialBuffer[5] == 'E' && serialBuffer[6] == 'Q' && serialBuffer[7] == 'T') {
  //Request is following
    if(serialBuffer[0] == '0' && serialBuffer[1] == '0' && serialBuffer[2] == '0' && serialBuffer[3] == '0') {
    //Request is for the RF433Mhz switches, so local
        if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'S' && serialBuffer[11] == 'T') {
          //switch state requested
          String responseStr = "0000STATSWST";
          responseStr.toCharArray(response,32);
          //now we add "<switchnr><state>"
          response[12] = serialBuffer[12]; //copy the switch number
          if(serialBuffer[12] == '1') {

            response[13] = state0?'1':'0';
            
          }
          else if(serialBuffer[12] == '2') {
            response[13] = state1?'1':'0';
          }
          else if(serialBuffer[12] == '3') {
            response[13] = state2?'1':'0';
          }
          response[14] = '\0';
          
          Serial.println(response);
          
        }
    }
  
  }
    
  if(serialBuffer[4] == 'C' && serialBuffer[5] == 'O' && serialBuffer[6] == 'M' && serialBuffer[7] == 'D') {
  //command is following
    if(serialBuffer[0] == '0' && serialBuffer[1] == '0' && serialBuffer[2] == '0' && serialBuffer[3] == '0') {
    //destination address is RF433 Mhz receivers
    Serial.println("destination address is RF433 Mhz receivers");
    Serial.println("Command is following");
      if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'N') {
      //switch on command
       Serial.println("Switch on command");
        if(serialBuffer[12] == '1') {
          transmitter.sendUnit(0,true);
          state0 = true;
        }
        else if(serialBuffer[12] == '2') {
          transmitter.sendUnit(1,true);
          state1 = true;
        }
        else if(serialBuffer[12] == '3') {
          transmitter.sendUnit(2,true);
          state2 = true;
          }
      }
      else if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'F') {
        //switch off command
        Serial.println("Switch off command");
        if(serialBuffer[12] == '1') {
          transmitter.sendUnit(0,false);
          state0 = false;
        }
        else if(serialBuffer[12] == '2') {
          transmitter.sendUnit(1,false);
          state1 = false;
          }
        else if(serialBuffer[12]== '3') {
          transmitter.sendUnit(2,false);
          state2 = false;
          }
          
      }
    }
  }
}



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
NewRemoteTransmitter transmitter2(16232974, 6, 260, 3);

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
bool state3 = false;
bool state4 = false;
bool state5 = false;
bool stateRelay = false;
boolean green;

/* turn debug on  */
bool debug = false;


void receivedMessage() {
 char* ptrbuffer;
 if(debug)Serial.println("interrupt!");
 ptrbuffer = controller.getMsg();
 for(int i = 0; i<32;i++) {
   Serial.print(ptrbuffer[i]);
 }
 Serial.print('\r');
 received++; 
}

void setup() {
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  Serial.begin(115200); //max baud rate
  pinMode(A0,OUTPUT);
  digitalWrite(A0,LOW);
  //nrf = NRF905();
  controller = Domotica();
  controller.setDebug(debug);
  controller.init(0); 
  attachInterrupt(1,receivedMessage,RISING);
  transmitter.sendUnit(0,false);
  transmitter.sendUnit(1,false);
  transmitter.sendUnit(2,false);
  transmitter2.sendUnit(0,false);
  transmitter2.sendUnit(1,false);
  transmitter2.sendUnit(2,false);
  
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 31249;// 2 second interval = MINIMUM for the temperature sensor // 1 sec interval for reading, can't read in interrupt because of use of delay == bad
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  green = !green;
   
   digitalWrite(A1, green);
   digitalWrite(A2, !green);
}

void loop() {
  while(!Serial.available()); //wait until a message is sent
  int i = 0;
  char temp;
  delay(1);
  //Serial.println("receiving");
  while(i < 32) {
      temp = Serial.read();
      if( (temp >= 'a' && temp <= 'z') or (temp >= '0' && temp <= '9') or (temp >= 'A' && temp <= 'Z') or temp == '\r')  { //alphanumerical char caps only
        serialBuffer[i++] = temp;
        if(temp == '\r')
          i = 32; //stop reading when \r is read
      }
      //Serial.print(i);
      //wait for next character, this is a more solid solution!
      while(Serial.available() == 0 && i < 32);
  }
  while(Serial.read() != -1); //flush buffer
  if(debug)Serial.print("I got: ");
  if(debug)Serial.println(serialBuffer);
  if(debug)Serial.print("The buffer now contains: ");
  for(int i = 0; i<32; i++) {
    if(debug)Serial.print(serialBuffer[i], HEX);
    if(debug)Serial.print(" ");  
  }
  if(debug)Serial.println("");
  parseSerialBuffer();
  delay(50); //allow some time to let the other end parse the message
}

void parseSerialBuffer() {
  //yellow light on
  digitalWrite(A2, true);
  if(debug)Serial.println("Parsing message");
  
    
  if(serialBuffer[0] == '0' && serialBuffer[1] == '0' && serialBuffer[2] == '0' && serialBuffer[3] == '0') {
    //Request is for the RF433Mhz switches, so local
    if(serialBuffer[4] == 'R' && serialBuffer[5] == 'E' && serialBuffer[6] == 'Q' && serialBuffer[7] == 'T') {
    //Request is following
      
          if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'S' && serialBuffer[11] == 'T') {
            //switch state requested
            String responseStr = "0000STATSWST";
            responseStr.toCharArray(response,32);
            //now we add "<switchnr><state>"
            response[12] = serialBuffer[12]; //copy the switch number
            if(serialBuffer[12] == '0') {
  
              response[13] = state0?'1':'0';
              
            }
            else if(serialBuffer[12] == '1') {
              response[13] = state1?'1':'0';
            }
            else if(serialBuffer[12] == '2') {
              response[13] = state2?'1':'0';
            }
            if(serialBuffer[12] == '3') {
  
              response[13] = state3?'1':'0';
              
            }
            else if(serialBuffer[12] == '4') {
              response[13] = state4?'1':'0';
            }
            else if(serialBuffer[12] == '5') {
              response[13] = state5?'1':'0';
            }
            response[14] = '\r';
            
            Serial.println(response);
            
          }
      }
    //command
    if(serialBuffer[4] == 'C' && serialBuffer[5] == 'O' && serialBuffer[6] == 'M' && serialBuffer[7] == 'D') {
  //command is following
     if(debug)Serial.println("destination address is RF433 Mhz receivers");
      if(debug)Serial.println("Command is following");
        if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'N') {
        //switch on command
         if(debug)Serial.println("Switch on command");
          if(serialBuffer[12] == '0') {
            transmitter.sendUnit(0,true);
            state0 = true;
          }
          else if(serialBuffer[12] == '1') {
            transmitter.sendUnit(1,true);
            state1 = true;
          }
          else if(serialBuffer[12] == '2') {
            transmitter.sendUnit(2,true);
            state2 = true;
          }
          else if(serialBuffer[12] == '3') {
            transmitter2.sendUnit(0,true);
            state3 = true;
          }
          else if(serialBuffer[12] == '4') {
            transmitter2.sendUnit(1,true);
            state4 = true;
          }
          else if(serialBuffer[12] == '5') {
            transmitter2.sendUnit(2,true);
            state5 = true;
          }
        }
        else if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'F') {
          //switch off command
          if(debug)Serial.println("Switch off command");
          if(serialBuffer[12] == '0') {
            transmitter.sendUnit(0,false);
            state0 = false;
          }
          else if(serialBuffer[12] == '1') {
            transmitter.sendUnit(1,false);
            state1 = false;
          }
          else if(serialBuffer[12]== '2') {
            transmitter.sendUnit(2,false);
            state2 = false;
          }
          if(serialBuffer[12] == '3') {
            transmitter2.sendUnit(0,false);
            state3 = false;
          }
          else if(serialBuffer[12] == '4') {
            transmitter2.sendUnit(1,false);
            state4 = false;
          }
          else if(serialBuffer[12]== '5') {
            transmitter2.sendUnit(2,false);
            state5 = false;
          }
            
        }
  
    }
    }
    
    else if(serialBuffer[0] == 0x66 && serialBuffer[1] == 0x66 && serialBuffer[2] == 0x66 && serialBuffer[3] == 0x66) {
    //Request is for the local relay
        if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'S' && serialBuffer[11] == 'T') {
          //switch state requested
          String responseStr = "ffffSTATSWST";
          responseStr.toCharArray(response,32);
          //now we add "<switchnr><state>"
          response[12] = serialBuffer[12]; //copy the switch number
          if(serialBuffer[12] == '1') {
            response[13] = state0?'1':'0'; 
          }
          response[14] = '\r';
          
          Serial.println(response);
          
        }
        if(serialBuffer[4] == 'C' && serialBuffer[5] == 'O' && serialBuffer[6] == 'M' && serialBuffer[7] == 'D') {
  //command is following
   
      //command is for this controller itself
      if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'N') {
      //switch on command
       if(debug)Serial.println("Switch on command");
        if(serialBuffer[12] == '1') {
          digitalWrite(A0,HIGH);
          stateRelay = true;
        }
       
      }
      else if(serialBuffer[8] == 'S' && serialBuffer[9] == 'W' && serialBuffer[10] == 'O' && serialBuffer[11] == 'F') {
        //switch off command
        if(debug)Serial.println("Switch off command");
        if(serialBuffer[12] == '1') {
          digitalWrite(A0,LOW);
          stateRelay = false;
        } 
      }
     }
 
  }
  
  else {
    //the message has to be forwarded using the NRF905 chip
    controller.sendToAddress(serialBuffer, serialBuffer+4);
  }
    
  //yellow light off
  digitalWrite(A2, false);
  
    
  
}



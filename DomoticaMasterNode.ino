#include <NRF905.h>
#include <domotica.h>
#include <SPI.h>

Domotica controller;
//NRF905 nrf;
char testmsg[MSG_LEN] = {"switch10000000000000000000"};
long sent = 0;
long received = 0;

void setup() {
  Serial.begin(9600);
  //nrf = NRF905();
  controller = Domotica();
  controller.setDebug(true);
  controller.init(0);  
  
  
}

void loop() {
  bool res = controller.sendToNode(2,testmsg);
  sent++;
  //delay(10);
  if(res) received++;
  Serial.print("Sent:\t");
  Serial.print(sent);
  Serial.print("\tReceived:\t");
  Serial.print(received);
  float perc = (received * 100) / sent;
  Serial.print("\t%:\t");
  Serial.println(perc);
  delay(2000);
}

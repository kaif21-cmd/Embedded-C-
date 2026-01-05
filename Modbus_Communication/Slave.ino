 

#include <ModbusRTU.h> // modbus library for r/w

ModbusRTU mb; // handeling the communication

uint16_t resistor1 = 100; // unsigned me 16bit - data store hoga 
uint16_t resistor2 = 220; // data store kar rhe he 
uint16_t resistor3 = 330; // data store 

void setup() {
  Serial.begin(115200); // serial ko set

  // UART2 for Modbus
  Serial2.begin(9600, SERIAL_8N1, 32, 33); // communcation 96000 8 serial data bit rx,tx 

  mb.begin(&Serial2);
  mb.slave(1);  //slave id 1 dedi 1-255 hoti h 

  // Holding Registers me value add krenge 
  mb.addHreg(0, resistor1);
  mb.addHreg(1, resistor2);
  mb.addHreg(2, resistor3);

  Serial.println("Modbus Slave Ready");
}

void loop() {
  mb.task(); // ye cmd ka rek sunega or master ki cmd ka reply kre ga 
}




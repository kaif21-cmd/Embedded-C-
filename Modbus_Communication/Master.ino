

#include <ModbusRTU.h> // read write 

ModbusRTU mb;

// 3 registers ka data
uint16_t resValue[3]; //array me save 

// Call back execute 
bool cbRead(Modbus::ResultCode event, uint16_t transactionId, void* data) {

  if (event == Modbus::EX_SUCCESS) {
    Serial.println("----- DATA RECEIVED -----");
    Serial.print("Register 0: "); Serial.println(resValue[0]);
    Serial.print("Register 1: "); Serial.println(resValue[1]);
    Serial.print("Register 2: "); Serial.println(resValue[2]);
    Serial.println("-------------------------");
  } else {
    Serial.println("Read Failed");
  }

  return true;
}

void setup() {
  Serial.begin(115200);

  // UART2  RX=16, TX=17
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  mb.begin(&Serial2);
  mb.master();

  Serial.println("Modbus Master Ready");
  Serial.println("Send 0 to read all registers");
}

void loop() {
  mb.task();   // ❗ VERY IMPORTANT

  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '0') {
      // Slave ID = 1
      // Start reg = 0
      // Total end  = 3
      mb.readHreg(1, 0, resValue, 3, cbRead); // slave id one se 0,1,3 sab registor ki value dega  0- 3
    }
  }
}

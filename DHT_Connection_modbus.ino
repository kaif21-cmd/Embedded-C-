#Slave code 

#include <ModbusRTU.h>
#include <DHT.h>

// DHT

#define DHTPIN 33 // pin 33 declare 
#define DHTTYPE DHT11 // dht type 

DHT dht(DHTPIN, DHTTYPE);
ModbusRTU mb;

// total parameters
const int no_para = 4;   // PM2.5, PM10, TEMP, HUM
float para_val[no_para]; // Ye array actual sensor values rakhega float 

// each float = 2 registers
uint16_t reg[no_para][2]; // each flot 2 registers  1 float = 2 registers

// FLOAT to HEX 
void floatToHex(float value, uint16_t *reg) {
  union {  // union ek hi memory ko alag alahg datatype se dekhna
    float f;   //32 bit 
    uint16_t h[2]; // 32 bit arry fro 2 elem 16 ,16 bit ke part me 
  } u; // union  

  u.f = value; // CPU converts float to binary iunn IEEE format 
  reg[0] = u.h[0];   // first part union ki wahi memory 16 bit ke first half ko read kregi and 1 register me save krdegi
  reg[1] = u.h[1];  // second part union ki wahi memory 16 bit ke seconf half ko read kregi and 2 register me save kredegi 
}

void setup() {
  Serial.begin(115200);

  // modbus uart
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  dht.begin(); // dht begin 

  mb.begin(&Serial2); // serial define 
  mb.slave(1);

  // total 8 holding register 
  for (int i = 0; i < 8; i++) {
    mb.addHreg(i, 0);
  }

  Serial.println(" Modbus Slave  DHT11 Ready");
}

void loop() {
  mb.task();   //MUST for Modbus

  // -------- SENSOR / DATA READ --------
  para_val[0] = random(10, 80);    // PM2.5 dummy
  para_val[1] = random(20, 150);   // PM10 dummy

  para_val[2] = dht.readTemperature(); // temp
  para_val[3] = dht.readHumidity();    // hum

  // DHT condition if fail -
  if (isnan(para_val[2]) || isnan(para_val[3])) {
    para_val[2] = 0;
    para_val[3] = 0;
    Serial.println(" DHT Read Failed");
  }

  // updating the registers values 

  for (int i = 0; i < no_para; i++) { // i=0,1,2,3= pm.2.5,pm10,temp,hum
    floatToHex(para_val[i], reg[i]); // hreg holding register add me value dalni he  
    mb.Hreg(i * 2,     reg[i][0]); // every element have two registers // for low part ex temp i=2 2*2=4 to temp ki low value register 4 me dalni he 
    mb.Hreg(i * 2 + 1, reg[i][1]);// for high part  temp ki high value // register 5 me 
  }

  // 

  Serial.print("PM2.5: "); Serial.print(para_val[0]);
  Serial.print("  PM10: "); Serial.print(para_val[1]);
  Serial.print("  TEMP: "); Serial.print(para_val[2]);
  Serial.print("  HUM: ");  Serial.println(para_val[3]);

  delay(2000); 
}

=====================================

# Master Code 
  #include <ModbusMaster.h>

ModbusMaster node;


// converting hex to float 

float hexToFloat(uint16_t low, uint16_t high) {
  union {
    uint16_t h[2];
    float f;
  } u;
  u.h[0] = low;
  u.h[1] = high; 
  return u.f;
}

void setup() {
  Serial.begin(115200);

 
  Serial1.begin(9600, SERIAL_8N1, 32, 33); //  uart communication 

  node.begin(1, Serial1);   // Slave ID = 1
  Serial.println("Modbus Master Ready");
}

void loop() {
  uint8_t result = node.readHoldingRegisters(0, 8);  // 0-8 register data 

  if (result == node.ku8MBSuccess) {
    Serial.println("---- DATA RECEIVED ----");

    Serial.print("PM2.5: ");
    Serial.println(hexToFloat(
      node.getResponseBuffer(0), // pehel o register ki value 
      node.getResponseBuffer(1))); // then 1 regiter ki value in dono ko merge krke float same condition applied for all..
  
    Serial.print("PM10 : ");
    Serial.println(hexToFloat(
      node.getResponseBuffer(2),
      node.getResponseBuffer(3)));

    Serial.print("TEMP : ");
    Serial.println(hexToFloat(
      node.getResponseBuffer(4),
      node.getResponseBuffer(5)));

    Serial.print("HUM  : ");
    Serial.println(hexToFloat(
      node.getResponseBuffer(6),
      node.getResponseBuffer(7)));

    Serial.println("-----------------------");
  } else {
    Serial.print("Read Failed: ");
    Serial.println(result);
  }

  delay(1000);
}

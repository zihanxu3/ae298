
/*-----------------------Bretl Research Group------------------------*/
/*
 ADXL355-PMDZ Accelerometer Sensor Display


 Modified 25 Sept 2019
 by Zihan Xu 
 */

#include <SPI.h>

// Memory register addresses:
const int XDATA3 = 0x08;
const int XDATA2 = 0x09;
const int XDATA1 = 0x0A;
const int YDATA3 = 0x0B;
const int YDATA2 = 0x0C;
const int YDATA1 = 0x0D;
const int ZDATA3 = 0x0E;
const int ZDATA2 = 0x0F;
const int ZDATA1 = 0x10;
const int RANGE = 0x2C;
const int POWER_CTL = 0x2D;

// Device values
const int RANGE_2G = 0x01;
const int RANGE_4G = 0x02;
const int RANGE_8G = 0x03;
const int MEASURE_MODE = 0x00; // Only accelerometer

// Operations
const int READ_BYTE = 0x01;
const int WRITE_BYTE = 0x00;

// Pins used for the connection with the sensor
const int CHIP_SELECT_PIN = 7;

// setup a count, iterating the loop for 65000 times
int count = 0;

void setup() {
  SPI.begin();
  //SPI.beginTransaction(SPISettings(500, MSBFIRST, SPI_MODE0));
  Serial.begin(9600);
//  //possible configuration: set up the clock 
  
  
  // Initalize the  data ready and chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_SELECT_PIN, HIGH);

  //Configure ADXL355:
  writeRegister(RANGE, RANGE_2G); // 2G
  writeRegister(POWER_CTL, MEASURE_MODE); // Enable measure mode

  // Give the sensor time to set up:
  delay(100);
}

void loop() {

  int axisAddresses[] = {XDATA1, XDATA2, XDATA3, YDATA1, YDATA2, YDATA3, ZDATA1, ZDATA2, ZDATA3};
  int axisMeasures[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  int dataSize = 9;

  // Read accelerometer data
  readMultipleData(axisAddresses, dataSize, axisMeasures);

  // Split data
//  unsigned long tempV = 0;
//  unsigned long value = 0;
//  value = axisMeasures[2]; 
//  value <<= 12;
//  tempV = axisMeasures[1];
//  tempV <<= 4;
//  value |= tempV;
//  tempV = axisMeasures[0];
//  tempV >>= 4;
//  value |= tempV;
//
//  if (axisMeasures[2] & 0x80) {
//    value = value | 0xFFF00000;
//  }
  int xdata = get_data_helper(axisMeasures[0], axisMeasures[1], axisMeasures[2]);
  int ydata = get_data_helper(axisMeasures[3], axisMeasures[4], axisMeasures[5]);
  int zdata = get_data_helper(axisMeasures[6], axisMeasures[7], axisMeasures[8]);
//  int xdata = (axisMeasures[0] >> 4) + (axisMeasures[1] << 4) + (axisMeasures[2] << 12);
//  int ydata = (axisMeasures[3] >> 4) + (axisMeasures[4] << 4) + (axisMeasures[5] << 12);
//  int zdata = (axisMeasures[6] >> 4) + (axisMeasures[7] << 4) + (axisMeasures[8] << 12);

  
 
  // Apply two's complement
//  if (xdata >= 0x80000) {
//    xdata = ~xdata + 1;
//  }

//  if (ydata >= 0x80000) {
//    ydata = ~ydata + 1;
//  }
//  if (zdata >= 0x80000) {
//    zdata = ~zdata + 1;
//  }

  
  // Print axis
  Serial.print("X=");
  Serial.print(xdata * 3.9E-6);
  Serial.print("\t");
  
  Serial.print("Y=");
  Serial.print(ydata * 3.9E-6);
  Serial.print("\t");

  Serial.print("Z=");
  Serial.print(zdata * 3.9E-6);
  Serial.print("\n");

  // Next data in 100 milliseconds
  delay(100);
//  ++count;
//
//  if (count % 10000 == 0) {
//    exit(0);
//  }
}

/* 
 * Write registry in specific device address
 */
void writeRegister(byte thisRegister, byte thisValue) {
  byte dataToSend = (thisRegister << 1) | WRITE_BYTE;
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  SPI.transfer(thisValue);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}

/* 
 * Read registry in specific device address
 */
unsigned int readRegistry(byte thisRegister) {
  unsigned int result = 0;
  byte dataToSend = (thisRegister << 1) | READ_BYTE;

  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  return result;
}

unsigned long get_data_helper(int data_1, int data_2, int data_3) {
  unsigned long tempV = 0;
  unsigned long value = 0;
  value = data_3; 
  value <<= 12;
  tempV = data_2;
  tempV <<= 4;
  value |= tempV;
  tempV = data_1;
  tempV >>= 4;
  value |= tempV;

  if (data_3 & 0x80) {
    value = value | 0xFFF00000;
  }
  return value;
}
/* 
 * Read multiple registries
 */
void readMultipleData(int *addresses, int dataSize, int *readedData) {
  digitalWrite(CHIP_SELECT_PIN, LOW);
  for(int i = 0; i < dataSize; i = i + 1) {
    byte dataToSend = (addresses[i] << 1) | READ_BYTE;
    SPI.transfer(dataToSend); //send data address 
    readedData[i] = SPI.transfer(0x00);
  }
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}


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
const int OFFSET_X_H = 0x1E;
const int OFFSET_X_L = 0x1F;
const int OFFSET_Y_H = 0x20;
const int OFFSET_Y_L = 0x21;
const int OFFSET_Z_H = 0x22;
const int OFFSET_Z_L = 0x23;

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
  Serial.begin(9600);
 
  //possible configuration: set up the clock 
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  
  //set up the pins 
  SPI.begin();

  // Initalize the  data ready and chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

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
  int offSets[] = {0, 0, 0};

  // Read accelerometer data
//  readMultipleData(axisAddresses, dataSize, axisMeasures);
  getOffsets(offSets);

  // Split data
  // Modified to read data one register at a time instead of a for loop
  int xdata = (readRegistry(XDATA1) >> 4) + (readRegistry(XDATA2) << 4) + (readRegistry(XDATA3) << 12);
  int ydata = (readRegistry(YDATA1) >> 4) + (readRegistry(YDATA2) << 4) + (readRegistry(YDATA3) << 12);
  int zdata = (readRegistry(ZDATA1) >> 4) + (readRegistry(ZDATA2) << 4) + (readRegistry(ZDATA3) << 12);
  
  // Apply two's complement
  if (xdata >= 0x80000) {
    xdata = ~xdata + 1;
  }
  if (ydata >= 0x80000) {
    ydata = ~ydata + 1;
  }
  if (zdata >= 0x80000) {
    zdata = ~zdata + 1;
  }

  // Print axis
  Serial.print("X=");
  Serial.print(xdata);
  Serial.print("\t");
  
  Serial.print("Y=");
  Serial.print(ydata);
  Serial.print("\t");

  Serial.print("Z=");
  Serial.print(zdata);
  Serial.print("\n");

  // Next data in 100 milliseconds
  delay(100);
  ++count;

  if (count == 65000) {
    return;
  }
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
  //address to send to the SPI bus to obtain things for that address
  byte dataToSend = (thisRegister << 1) | READ_BYTE;
 
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  return result;
}

void getOffsets(int* offSets) {
    int x_arr[] = {readRegistry(OFFSET_X_H), readRegistry(OFFSET_X_L)};
    int y_arr[] = {readRegistry(OFFSET_Y_H), readRegistry(OFFSET_Y_L)};
    int z_arr[] = {readRegistry(OFFSET_Z_H), readRegistry(OFFSET_Z_L)};

    
    int x_data = x_arr[1] + (x_arr[0] << 8);
    int y_data = y_arr[1] + (y_arr[0] << 8);
    int z_data = z_arr[1] + (z_arr[0] << 8);
    
    offSets[0] = (x_data << 4);
    offSets[1] = (y_data << 4);
    offSets[2] = (z_data << 4);
//    # Apply tow complement
//    x_data = twos_comp(x_data, 20)
//    y_data = twos_comp(y_data, 20)
//    z_data = twos_comp(z_data, 20)
//    # Return values
}

/* 
 * Read multiple registries
 */
//void readMultipleData(int *addresses, int dataSize, int *readedData) {
//  digitalWrite(CHIP_SELECT_PIN, LOW);
//  for(int i = 0; i < dataSize; i = i + 1) {
//    byte dataToSend = (addresses[i] << 1) | READ_BYTE;
//    SPI.transfer(dataToSend); //send data address and 
//    readedData[i] = SPI.transfer(0x00);
//  }
//  digitalWrite(CHIP_SELECT_PIN, HIGH);
//}

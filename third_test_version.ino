
#include <SPI.h>

const int FIFO = 0x11;

const int POWER = 0x2D;//1 for standby, 0 for go
const int RANGE = 0x2C;// for range
const int SYNC = 0x2B;
const int STATUS = 0x04; 

const int FILTER = 0x28;

bool isTesting = false;

const int chipSelectPin = 7;

 void setup()  
 {  
  Serial.begin(9600);  
    SPI.begin();
    pinMode(chipSelectPin, OUTPUT);
    writeRegister( POWER, 0x01); // standby
    writeRegister( RANGE, 0x01);// 2g

    writeRegister( SYNC, 0x00); //internal clock
    writeRegister( FILTER, 0x05); // 125 Hz

    delay(100);
    
    writeRegister( POWER, 0x00); //measurement mode
    
    delay(100);

    isTesting = true;
    testing();
 }  
 void loop() { } 

 void testing(){

    long start = millis(); //track how long data is collected

    long count= 0 ;  // keeps count of asserted DATA_RDY

    while (isTesting){

        if (readRegister(STATUS) & 1 == 1){ //DATA_RDY bit
            count++;
            readFIFO(); // processing function
        }

        if (millis() - start >= 30000){
             isTesting = false;  
            long duration = millis() - start;
            Serial.println("test ended");
            Serial.println(count);
            Serial.println(duration);
        }
    }    
 }

void readFIFO (){
    //fifo stores 3 bytes of data for each axis, the 20 most significant bits are the values in two's complement
    byte dataByte [9];//
    int dataInt [3];

    digitalWrite(chipSelectPin, LOW);
    SPI.transfer( (FIFO<<1) | 1);
    for (int i = 0 ; i < 9; i++){
        dataByte[i] = SPI.transfer(0x00);
    }
    digitalWrite(chipSelectPin, HIGH);


     // the 16 most significant bits are kept in an integer 
     for (int z = 0 ; z < 3; z++){
        dataInt[z] = ((dataByte[z*3]<<8) | (dataByte[z*3 +1]));         
     }
     //here is the problem
     Serial.print (dataInt[0]);
     Serial.print ("x");
     Serial.print (dataInt[1]);
     Serial.print ("y");
     Serial.println (dataInt[2]);
     Serial.print ("z");
}

 byte readRegister (byte thisRegister){
    byte inByte = 0 ;
    digitalWrite(chipSelectPin, LOW);
    SPI.transfer((thisRegister << 1) | 1);
    inByte = SPI.transfer(0x00);
    digitalWrite(chipSelectPin, HIGH);
    return inByte;
} 
void writeRegister (byte thisRegister, byte value){
    digitalWrite(chipSelectPin, LOW);
    SPI.transfer(thisRegister << 1);
    SPI.transfer(value);
    digitalWrite(chipSelectPin, HIGH);
}

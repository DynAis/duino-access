#include "hzDevice.h"
#include "Arduino.h"
// #include "hzUart.h"


void debug(void){
  analogWrite(5, 200);
  delay(300);
  analogWrite(5,0);
  delay(100);
    analogWrite(5, 200);
  delay(70);
  analogWrite(5,0);
  delay(30);
  analogWrite(5, 200);
  delay(70);
  analogWrite(5,0);
}

void waitting(void){
  analogWrite(5, 200);
  delay(300);
  analogWrite(5,0);
}

void success(void){
  analogWrite(5, 200);
  delay(100);
  analogWrite(5,0);
  delay(50);
  analogWrite(5, 200);
  delay(100);
  analogWrite(5,0);
}

UINT32 getEndTime(UINT32 time){
    return getCurTime() + time;
}

UINT32 getCurTime(void){
    return millis();
}

#define UART_RX_TIMEOUT     10
UINT8 uartReceive(UINT8 *data, UINT32 *len){
    UINT32 time;

    if (NULL == data || NULL == len)
        return Error;
    time = getEndTime(UART_RX_TIMEOUT);
    while(!(Serial1.available() > 0)) {
        if (time < getCurTime()) {
            return ErrorTimeout;
        }
    }
    *len = Serial1.available();
    Serial1.readBytes(data, *len);
    // Serial.print("RX reciving: ");    
    // for (UINT32 i = 0; i < *len; i++){
    //     Serial.print(data[i], HEX);
    // }
    // Serial.println(" ");
    return Ok;
}

void uartWrite(const UINT8 *data, UINT32 len){
    // UINT32 i;
    // Serial.print("len=");Serial.print(len);
    // Serial.print(", TX writing: ");
    // for (i = 0; i < len; i++){
    //     Serial.print(data[i], HEX);
    // }
    // Serial.println(" ");
    Serial1.write(data, len);
}

// read data from your fingerprint module device
UINT8 read(UINT8 *data, UINT32 *len){
    return uartReceive(data, len);
}

//write data to your fingerprint module device
void write(const UINT8 *data, UINT32 len){
    uartWrite(data, len);
}


void genRandom(UINT8 *ramdom,UINT8 len)
{
   //Need to implement 
    randomSeed(millis());
    for(int i=0; i<len ;i++){
        ramdom[i] = (UINT8)random(0,255);
    }
}

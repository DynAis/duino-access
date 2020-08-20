#include "hzDevice.h"
#include "hzUart.h"

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

}

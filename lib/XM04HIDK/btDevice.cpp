#include <Arduino.h>
#include "btDevice.h"
#include "btPinDef.h"
#include <SoftwareSerial.h>

SoftwareSerial btSerial(SOFT_RX, SOFT_TX);

void btInit(void){
  btSerial.begin(9600);
}

UINT8 char2key(char a){
  if(a>='A' && a<='Z'){
    return (UINT8)(a-'A'+4);
  }
  else if(a>='1' && a<='9'){
    return (UINT8)(a-'1'+30);
  }
  else if(a=='0'){
    return 0x27;
  }
  else
  {
    return 0x04;
  }
  
}

void btSendKey(UINT8 state, UINT8 key1, UINT8 key2, UINT8 key3, UINT8 key4, UINT8 key5, UINT8 key6){
  BT_CMD_PACK cmdPack;
  cmdPack.header = 0x01A1000C;
  cmdPack.state = state;
  cmdPack.keystart = 0x00;
  cmdPack.key1 = key1;
  cmdPack.key2 = key2;
  cmdPack.key3 = key3;
  cmdPack.key4 = key4;
  cmdPack.key5 = key5;
  cmdPack.key6 = key6;

  btSerial.write((UINT8 *)&cmdPack, sizeof(BT_CMD_PACK));
}

void btSendPassworld(const char* text){
  UINT8 key;
  for(UINT8 i=0; i<32; i++){
    key = char2key(text[i]);
    // Serial.println(key,HEX);
    if(key>=0x1E)
      btSendKey(0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00);
    else
      btSendKey(0x02, key, 0x00, 0x00, 0x00, 0x00, 0x00);
    btSendKey(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  }
    btSendKey(0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00);
    btSendKey(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}
#include "auth.h"
#include "string.h"

#define     KEY_LEN         16
#define     PKG_SIZE   			10



static void getXorRandom(const uint8_t *random, const uint8_t *data  ,uint8_t * newRandom)
{
  memcpy(newRandom,random,KEY_LEN);
  for(int i=0; i<PKG_SIZE; i++){
    newRandom[i] = random[i] ^ data[i];
  }

}
//signature :32 byte signature 
//rspPkg : The first 10 bytes of response package
//signatureKey:16 byte key
int doSignatureMatch(uint8_t *signature, const uint8_t *rspPkg, uint8_t *signatureKey,uint8_t *calcHash){

    uint8_t xorRandom[KEY_LEN];
    getXorRandom(signature,rspPkg,xorRandom);

    md5_hash(signatureKey, xorRandom, KEY_LEN, calcHash);

    int retval= memcmp(calcHash, signature + KEY_LEN, KEY_LEN);

    return retval;
}

//random : 16 byte random number
//cmdPkg : The first 10 byte of cmd package
//signatureKey : 16 byte key
//signature : 32 byte signature
void genSignature(uint8_t *random, const uint8_t *cmdPkg,uint8_t *signatureKey,uint8_t *signature){

    if(signature!=NULL){
        uint8_t xorRandom[KEY_LEN];
        getXorRandom(random,cmdPkg,xorRandom);

        uint8_t signatureHash[KEY_LEN];
        md5_hash(signatureKey, xorRandom,KEY_LEN, signatureHash);
        memcpy(signature,random,KEY_LEN);
        memcpy(&signature[KEY_LEN], signatureHash, KEY_LEN);
    }
}

//signKey : 32 byte signature Key including host key and device's key
//random : 16 byte random number
void updateSignKey(UINT8 *signKey, UINT8 *random)
{
    uint8_t i;

    for (i = 0; i < KEY_LEN; i++) {
        signKey[i] ^= random[i];
    }
    signKey += KEY_LEN;
    for (i = 0; i < KEY_LEN; i++) {
        signKey[i] ^= random[i];
    }
}



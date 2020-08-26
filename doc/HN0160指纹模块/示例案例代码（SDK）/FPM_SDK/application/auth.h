#ifndef _AUTH_H_
#define _AUTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "md5.h"
#include "hzTypes.h"

int doSignatureMatch( uint8_t *signature, const uint8_t *userData,  uint8_t *signatureKey, uint8_t *calcHash );
void genSignature(uint8_t *random, const uint8_t *userData, uint8_t *signatureKey, uint8_t *signature);
void updateSignKey(UINT8 *signKey, UINT8 *random);

#ifdef __cplusplus
}
#endif

#endif // _AUTH_H


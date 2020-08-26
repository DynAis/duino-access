/* 
 * MD5 hash in C
 */

#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>

#ifndef UINT32_C
#define UINT32_C(c) c ## UL
#endif

void md5_hash(uint8_t md5_pass[16], const uint8_t *input, uint32_t len, uint8_t output[16]);

#endif //_MD5_H_

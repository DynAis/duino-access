#ifndef _HZ_MISC_H_
#define _HZ_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hzTypes.h"


UINT16 calChksum16bit(UINT8 *buf, UINT32 len);
UINT16 cal_16bit_crc(UINT8* buf, UINT32 len);
UINT8 calXor8bit(UINT8 *buf, UINT32 len);
UINT32 getEndTime(UINT32 time);
UINT32 getCurTime(void);
void hzWdtEnable(void);
void hzWdtFeed(void);
void hzSleep(void);

#ifdef __cplusplus
}
#endif

#endif /* _HZ_MISC_H_ */

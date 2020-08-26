#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include "hzTypes.h"

#define FP_GROUP_SIZE                       6
#define MINI_VALID_PRESS                    FP_GROUP_SIZE
#define MAX_PRESS_CNT                       10

#define WAKE_UP_TIME                        200

#define FP_STATE_DEFAULT                    0
#define FP_STATE_START                      1
#define FP_STATE_WAIT_FINGER_OFF2ENROLL     2
#define FP_STATE_WAIT_FINGER_OFF2OK         3
#define FP_STATE_WAIT_FINGER_OFF2ERR        4
#define FP_STATE_ENROLL_PROC                5
#define FP_STATE_END_OK                     6
#define FP_STATE_END_ERR                    7


UINT8 fpWakeup(void);
UINT8 fpInit(void);
void fpEnroll(void);
UINT8 fpIdentify(void);
UINT8 fpUpdate(void);
UINT8 fpDelete(UINT16 index);
UINT8 fpDeleteAll(void);
UINT8 fpSleep(void);

#endif // #ifndef _FINGERPRINT_H_


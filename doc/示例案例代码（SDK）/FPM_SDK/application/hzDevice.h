#ifndef _HZ_DEVICE_H_
#define _HZ_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "hzTypes.h"
// communication UART buffer size definition
// minimal value should be 10 if communicating without signature
// minimal value should be 44 if communicating with signature
// 64 is recommanded to improve the robunstness
#define	COMM_BUFFER_SIZE			    	64

// hardware UART FIFO size definition according to the MCU
#define UART_FIFO_SIZE						1

// system timer type definition.
#define INCREASE_TIMER						1
#define DECREASE_TIMER						-1
#define TIMER_TYPE							INCREASE_TIMER

UINT32 getEndTime(UINT32 time);
UINT32 getCurTime(void);
UINT8 read(UINT8 *data, UINT32 *len);
void write(const UINT8 *data, UINT32 len);
void genRandom(UINT8 *ramdom,UINT8 len);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef _HZ_DEVICE_H_

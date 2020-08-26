#ifndef _HZ_UART_H
#define _HZ_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hzTypes.h"

UINT8 uartInit(UINT32 baudrate);

UINT8 uartShutdown(void);

void uartWrite(const UINT8 *data, UINT32 len);

UINT8 uartReceive(UINT8 *data, UINT32 *len);

#ifdef __cplusplus
}
#endif

#endif /* _HZ_UART_H */

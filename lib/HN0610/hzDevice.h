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

/** generic error codes */
typedef enum en_result
{
    Ok                          = 0u,  ///< No error
    Error                       = 1u,  ///< Non-specific error code
    ErrorAddressAlignment       = 2u,  ///< Address alignment does not match
    ErrorAccessRights           = 3u,  ///< Wrong mode (e.g. user/system) mode is set
    ErrorInvalidParameter       = 4u,  ///< Provided parameter is not valid
    ErrorOperationInProgress    = 5u,  ///< A conflicting or requested operation is still in progress
    ErrorInvalidMode            = 6u,  ///< Operation not allowed in current mode
    ErrorUninitialized          = 7u,  ///< Module (or part of it) was not initialized properly
    ErrorBufferFull             = 8u,  ///< Circular buffer can not be written because the buffer is full
    ErrorTimeout                = 9u,  ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
    ErrorNotReady               = 10u, ///< A requested final state is not reached
    OperationInProgress         = 11u  ///< Indicator for operation in progres
} en_result_t;

void debug(void);
void waitting(void);
void success(void);

UINT32 getEndTime(UINT32 time);
UINT32 getCurTime(void);
UINT8 uartReceive(UINT8 *data, UINT32 *len);
void uartWrite(const UINT8 *data, UINT32 len);
UINT8 read(UINT8 *data, UINT32 *len);
void write(const UINT8 *data, UINT32 len);
void genRandom(UINT8 *ramdom,UINT8 len);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef _HZ_DEVICE_H_

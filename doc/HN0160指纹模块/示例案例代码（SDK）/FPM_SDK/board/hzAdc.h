#ifndef _HZ_ADC_H
#define _HZ_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hzTypes.h"
#include "hzGpio.h"

#define PIN_ADC_CH1             (PORT2 | PIN06)

void adcInit(void);
void adcShutdown(void);
UINT16 adcGetVal(void);

#ifdef __cplusplus
}
#endif

#endif /* _HZ_ADC_H */

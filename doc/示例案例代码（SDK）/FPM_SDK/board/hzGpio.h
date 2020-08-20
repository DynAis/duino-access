#ifndef _HZ_GPIO_H_
#define _HZ_GPIO_H_

#include "hzTypes.h"
#include "gpio.h"

#define PORT0           0x00000000UL
#define PORT1           0x00010000UL
#define PORT2           0x00020000UL
#define PORT3           0x00030000UL
#define PORT4           0x00040000UL
#define PORT5           0x00050000UL
#define PORT6           0x00060000UL
#define PORT7           0x00070000UL

#define PIN00           0x00000000UL
#define PIN01           0x00000001UL
#define PIN02           0x00000002UL
#define PIN03           0x00000003UL
#define PIN04           0x00000004UL
#define PIN05           0x00000005UL
#define PIN06           0x00000006UL
#define PIN07           0x00000007UL
#define PIN08           0x00000008UL
#define PIN09           0x00000009UL
#define PIN10           0x0000000AUL
#define PIN11           0x0000000BUL
#define PIN12           0x0000000CUL
#define PIN13           0x0000000DUL
#define PIN14           0x0000000EUL
#define PIN15           0x0000000FUL


#define PIN_FP_INT      (PORT3 | PIN05)
#define PIN_FP_RST      (PORT3 | PIN06)
#define PIN_CHRG_DET    (PORT2 | PIN05)
#define PIN_LED_R       (PORT1 | PIN05)
#define PIN_LED_G       (PORT2 | PIN03)
#define PIN_LED_B       (PORT2 | PIN04)
#define PIN_MT_IN1      (PORT3 | PIN03)
#define PIN_MT_IN2      (PORT3 | PIN04)
#define PIN_MT_DET      (PORT3 | PIN02)

#define PIN_NONE        0xFFFFFFFFUL


/**
 * Enumeration type for the dirction of GPIO pad on a given pin.
 */
typedef enum {
    OUT,      /**< OUTPUT */
    IN,       /**< INPUT */
}GPIO_DIR;

/**
 * Enumeration type for the type of GPIO pad on a given pin.
 */
typedef enum {
    PULL_NONE,     /**< No pull-up or pull-down */
    PULL_UP,       /**< Set pad to weak pull-up */
    PULL_DOWN,     /**< Set pad to weak pull-down */
}GPIO_PULL;

void gpioCfgPin(UINT32 pin, GPIO_DIR dir, GPIO_PULL pull);
void gpioSetPin(UINT32 pin);
void gpioClrPin(UINT32 pin);
void gpioTglPin(UINT32 pin);
UINT32 gpioGetPin(UINT32 pin);

#endif

#include "hzGpio.h"

void gpioCfgPin(UINT32 pin, GPIO_DIR dir, GPIO_PULL pull)
{
    Gpio_InitIOExt((UINT8)(pin >> 16), (UINT8)pin, (en_gpio_dir_t)dir, (pull == PULL_UP), (pull == PULL_DOWN), FALSE, FALSE);
}

void gpioSetPin(UINT32 pin)
{
    UINT8 port, offset;
    UINT32 addr;

    port = (UINT8)(pin >> 16);
    offset = (UINT8)(pin & 0x000000ff);
    addr = (UINT32)&M0P_GPIO->P0OUT + port * GPIO_GPSZ;
    *((volatile UINT32 *)(addr)) |= ((1UL)<<(offset));
}

void gpioClrPin(UINT32 pin)
{
    UINT8 port, offset;
    UINT32 addr;

    port = (UINT8)(pin >> 16);
    offset = (UINT8)(pin & 0x000000ff);
    addr = (UINT32)&M0P_GPIO->P0OUT + port * GPIO_GPSZ;
    *((volatile UINT32 *)(addr)) &= (~(1UL<<(offset)));
}

void gpioTglPin(UINT32 pin)
{
    UINT8 port, offset;
    UINT32 addr;

    port = (UINT8)(pin >> 16);
    offset = (UINT8)(pin & 0x000000ff);
    addr = (UINT32)&M0P_GPIO->P0OUT + port * GPIO_GPSZ;
    *((volatile UINT32 *)(addr)) ^= ((1UL)<<(offset));
}

UINT32 gpioGetPin(UINT32 pin)
{
    return (UINT32)Gpio_GetIO((UINT8)(pin >> 16), (UINT8)pin);
}


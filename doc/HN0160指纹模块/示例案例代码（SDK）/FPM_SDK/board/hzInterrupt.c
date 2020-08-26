/*
* interrupt implement of the application.
*/

#include "hzTypes.h"
#include "hzGpio.h"

UINT32 tickMs = 0;
extern UINT32 mtrForTime;
extern UINT32 mtrRevTime;
extern UINT32 ledTglPin;


/*!
    \brief      SysTick interrupt handler that excute every ms. 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    tickMs++;
    // LED toggle process here
    if(PIN_NONE != ledTglPin) {
        if(!(tickMs % 100))
           gpioTglPin(ledTglPin);
    }
    // motor drive process here
    if(mtrForTime) { 
        if(--mtrForTime == 0) {
            gpioClrPin(PIN_MT_IN1);
        }
    }
    if(mtrRevTime) { 
        if(--mtrRevTime == 0) {
            gpioClrPin(PIN_MT_IN2);
        }
    }  
}

//void Gpio_IRQHandler(uint8_t port)
//{
//    *((uint32_t *)((uint32_t)&M0P_GPIO->P0ICLR + port * GPIO_GPSZ)) = 0;
//}


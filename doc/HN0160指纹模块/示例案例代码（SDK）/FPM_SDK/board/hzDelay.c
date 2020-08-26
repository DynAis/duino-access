#include "hzDelay.h"
#include "hzMisc.h"

extern UINT32 SystemCoreClock;

void delayUs(UINT32 us)
{
    UINT32 tick = 0, tickMark, tickExpect;

    tickMark = SysTick->VAL;
    tickExpect = us * SystemCoreClock / 1000000;
    do{
        if (tickMark < SysTick->VAL)
            tick = SysTick->LOAD - SysTick->VAL + tickMark;
        else
            tick = tickMark - SysTick->VAL;
    }while(tick < tickExpect);
}

void delayMs(UINT32 ms)
{
    UINT32 time;

    time = getCurTime();
	while (getCurTime() - time < ms);
}


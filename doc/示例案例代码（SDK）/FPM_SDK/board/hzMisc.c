#include "hzMisc.h"
#include "lpm.h"
#include "hzGpio.h"
#include "wdt.h"

extern UINT32 tickMs;

//// 16bit checksum calculation
//UINT16 calChksum16bit(UINT8 *buf, UINT32 len)
//{
//    UINT16 w_nCheckSum = 0;
//
//    while (len)
//    {
//        w_nCheckSum += *buf;
//        buf++;
//        len--;
//    }
//
//    return w_nCheckSum;
//}


// CRC CACULATION by CRC16_CCITT��x16+x12+x5+1��0x1021��
UINT16 cal_16bit_crc(UINT8* buf, UINT32 len)
{
	UINT8 i;
    UINT16 crc=0;
	while(len--!=0) 
	{
		for(i=0x80; i!=0; i/=2)
		{
			if((crc&0x8000)!=0) 
			{crc*=2; crc^=0x1021;}
			else
			{crc*=2;}

			if((*buf&i)!=0) crc^=0x1021;		
		}
		buf++;
	
	}
//  hzPrintf("crc: 0x%x\r\n", crc);
	return crc;
}

//// xor calculation.
//UINT8 calXor8bit(UINT8 *buf, UINT32 len)
//{
//    UINT8 xorCal = 0;
//
//    while (len)
//    {
//        xorCal ^= *buf;
//        buf++;
//        len--;
//    }
//
//    return xorCal;
//}

/*!
    \brief      gets the ending timer tick
    \param[in]  time: time to count with the MS unit
    \param[out] none
    \retval     ending timer ticks
*/
UINT32 getEndTime(UINT32 time)
{
    return (tickMs + time);
}

/*!
    \brief      gets the current timer tick
    \param[in]  none
    \param[out] none
    \retval     current timer ticks
*/
UINT32 getCurTime(void)
{
    return tickMs;
}

/*!
    \brief      enter sleep mode
    \param[in]  none
    \param[out] none
    \retval     current timer ticks
*/
void hzSleep(void)
{
    stc_lpm_config_t lpmCfg;

    lpmCfg.enSEVONPEND = (en_lpm_sevonpend_t)SlpExtDisable;
    lpmCfg.enSLEEPDEEP = SlpDpEnable;
    lpmCfg.enSLEEPONEXIT = SlpExtDisable;

    // enble the FP INT interrupt to wakeup
    Gpio_ClearIrq((UINT8)(PIN_FP_INT >> 16), (UINT8)PIN_FP_INT);
    Gpio_EnableIrq((UINT8)(PIN_FP_INT >> 16), (UINT8)PIN_FP_INT, GpioIrqRising);
    EnableNvic(PORT3_IRQn, DDL_IRQ_LEVEL_DEFAULT, TRUE);
    // config sleep mode and sleep
    Lpm_Config(&lpmCfg);
    Lpm_GotoLpmMode();
    // disable the FP INT interrupt after wakeup
    Gpio_ClearIrq((UINT8)(PIN_FP_INT >> 16), (UINT8)PIN_FP_INT);
    Gpio_DisableIrq((UINT8)(PIN_FP_INT >> 16), (UINT8)PIN_FP_INT, GpioIrqRising);
    EnableNvic(PORT3_IRQn, DDL_IRQ_LEVEL_DEFAULT, FALSE);
    // reset the time tick to zero to avoid the wrap around issue
    NVIC_DisableIRQ(SysTick_IRQn);
    tickMs = 0;
    NVIC_EnableIRQ(SysTick_IRQn);
}

/*!
    \brief      start the watch dog timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void hzWdtEnable(void)
{
    stc_wdt_config_t  stcWdt_Config;
    
    DDL_ZERO_STRUCT(stcWdt_Config);
    
    stcWdt_Config.u8LoadValue = 0x0c;//6.55s
    stcWdt_Config.enResetEnable = WRESET_EN;
    
    Clk_SetPeripheralGate(ClkPeripheralWdt,TRUE);
    Wdt_Init(&stcWdt_Config);
    
    Wdt_Start();
}

/*!
    \brief      feed the watch dog timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void hzWdtFeed(void)
{
    Wdt_Feed();
}


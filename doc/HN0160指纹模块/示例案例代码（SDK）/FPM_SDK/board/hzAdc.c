#include "hzAdc.h"
#include "hzDelay.h"
#include "adc.h"

void adcInit(void)
{
    stc_adc_cfg_t      stcAdcCfg;
    stc_adc_norm_cfg_t stcAdcNormCfg;
    
    DDL_ZERO_STRUCT(stcAdcCfg);
    DDL_ZERO_STRUCT(stcAdcNormCfg);
    
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralAdcBgr, TRUE);
 
    Gpio_SetAnalog(PIN_ADC_CH1 >> 16, (uint8_t)PIN_ADC_CH1, TRUE);
    
    Adc_Enable();
    M0P_BGR->CR_f.BGR_EN = 0x1u;//BGR必须使能
    M0P_BGR->CR_f.TS_EN = 0x0u;

    delayUs(100);
    
    stcAdcCfg.enAdcOpMode = AdcNormalMode;          //单次采样模式
    stcAdcCfg.enAdcClkSel = AdcClkSysTDiv1;         //PCLK
    stcAdcCfg.enAdcSampTimeSel = AdcSampTime4Clk;   //4个采样时钟
    stcAdcCfg.enAdcRefVolSel = RefVolSelInBgr2p5;   //参考电压:内部2.5V(avdd>3V,SPS<=200kHz)  SPS速率 = ADC时钟 / (采样时钟 + 16CLK) 
    stcAdcCfg.bAdcInBufEn = FALSE;                  //电压跟随器如果使能，SPS采样速率 <=200K
    stcAdcCfg.u32AdcRegHighThd = 0u;                //比较阈值上门限
    stcAdcCfg.u32AdcRegLowThd = 0u;                 //比较阈值下门限
    stcAdcCfg.enAdcTrig0Sel = AdcTrigDisable;       //ADC转换自动触发设置
    stcAdcCfg.enAdcTrig1Sel = AdcTrigDisable;
    Adc_Init(&stcAdcCfg);
    
    stcAdcNormCfg.enAdcNormModeCh = AdcExInputCH1;  //通道1 P26
    stcAdcNormCfg.bAdcResultAccEn = FALSE;
    Adc_ConfigNormMode(&stcAdcCfg, &stcAdcNormCfg);
}

void adcShutdown(void)
{
    Adc_Disable();
    M0P_BGR->CR_f.BGR_EN = 0x0u;
}

UINT16 adcGetVal(void)
{
    UINT16 adcVal;

    Adc_Start();       
    while(FALSE != Adc_PollBusyState());   
    Adc_GetResult(&adcVal);
    Adc_ClrAccResult();
    return adcVal;
}


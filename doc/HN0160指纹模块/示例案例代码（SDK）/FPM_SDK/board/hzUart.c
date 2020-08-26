#include "hzMisc.h"
#include "hzUart.h"
#include "uart.h"
#include "bt.h"


UINT8 uartInit(UINT32 baudrate)
{
    uint16_t timer;
    uint32_t pclk;
//    stc_clk_config_t stcCfg;
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
//  stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
//    en_uart_mmdorck_t enTb8;

    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcUartIrqCb);
//    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);
    //时钟部分配置
//  stcCfg.enClkSrc = ClkXTH;
//  stcCfg.enHClkDiv = ClkDiv1;
//  stcCfg.enPClkDiv = ClkDiv1;
//  Clk_Init(&stcCfg);
    
    Gpio_InitIOExt(0,1,GpioDirOut,TRUE,FALSE,FALSE,FALSE); 
    Gpio_InitIOExt(0,2,GpioDirOut,TRUE,FALSE,FALSE,FALSE);
    
    //通道端口配置
    Gpio_SetFunc_UART0_RXD_P01();
    Gpio_SetFunc_UART0_TXD_P02();

    //外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);
    Clk_SetPeripheralGate(ClkPeripheralUart0,TRUE);



    stcUartIrqCb.pfnRxIrqCb = NULL;
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = NULL;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = FALSE;
#if 1//多主机模式测试
    stcConfig.enRunMode = UartMode1;
//  stcMulti.enMulti_mode = UartMulti;
//  enTb8 = Addr;
//  Uart_SetMMDOrCk(UARTCH1,enTb8);
//  stcConfig.pstcMultiMode = &stcMulti;
    
    stcBaud.bDbaud = 0u;
    stcBaud.u32Baud = baudrate;
    stcBaud.u8Mode = UartMode1; //计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();
    timer=Uart_SetBaudRate(UARTCH0,pclk,&stcBaud);
  
    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    Bt_Init(TIM0, &stcBtConfig);//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM0,timer);
    Bt_Cnt16Set(TIM0,timer);
    Bt_Run(TIM0);
#endif
    Uart_Init(UARTCH0, &stcConfig);
    Uart_ClrStatus(UARTCH0,UartTxEmpty);//清所有中断请求
    Uart_ClrStatus(UARTCH0,UartRxFull);
    Uart_EnableFunc(UARTCH0,UartRx);
    return Ok;
}

UINT8 uartShutdown(void)
{
    return Bt_Stop(TIM0);
}

void uartWrite(const UINT8 *data, UINT32 len)
{
    UINT32 i;

    for (i = 0; i < len; i++)
        Uart_SendData(UARTCH0, data[i]);
}

#define UART_RX_TIMEOUT     10
UINT8 uartReceive(UINT8 *data, UINT32 *len)
{
    UINT32 time;

    if (NULL == data || NULL == len)
        return Error;
    time = getEndTime(UART_RX_TIMEOUT);
    while(0 == Uart_GetStatus(UARTCH0,UartRxFull)) {
        if (time < getCurTime()) {
            return ErrorTimeout;
        }
    }
    Uart_ClrStatus(UARTCH0,UartRxFull);
    Uart_ClrStatus(UARTCH0,UartRFRAMEError);
    *data = Uart_ReceiveData(UARTCH0);
    *len = 1u;
    return Ok;
}


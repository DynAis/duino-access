/*
* main of the application.
*/

#include "fp.h"
#include "fpmComm.h"
#include "hzGpio.h"
#include "hzMisc.h"
#include "gpio.h"
#include "hzUart.h"
#include "hzDelay.h"

#define LED_RED_ON          gpioClrPin(PIN_LED_R)
#define LED_RED_OFF         gpioSetPin(PIN_LED_R)
#define LED_RED_TOGGLE      gpioTglPin(PIN_LED_R)
#define LED_GREEN_ON        gpioClrPin(PIN_LED_G)
#define LED_GREEN_OFF       gpioSetPin(PIN_LED_G)
#define LED_GREEN_TOGGLE    gpioTglPin(PIN_LED_G)
#define LED_BLUE_ON         gpioClrPin(PIN_LED_B)
#define LED_BLUE_OFF        gpioSetPin(PIN_LED_B)
#define LED_BLUE_TOGGLE     gpioTglPin(PIN_LED_B)


extern UINT8 fpState;

UINT32 mtrForTime = 0;
UINT32 mtrRevTime = 0;
UINT32 ledTglPin = PIN_NONE;

static void okIndication(void)
{


}

static void errIndication(void)
{
    LED_GREEN_OFF; LED_BLUE_OFF;
    LED_RED_ON; delayMs(1000); LED_RED_OFF; delayMs(100);
}

static void fingerPressIndication(void)
{
    LED_RED_OFF; LED_BLUE_OFF;
    delayMs(100); LED_GREEN_ON; delayMs(100); LED_GREEN_OFF; delayMs(100);
}

static void fpmErrIndication(void)
{
    LED_RED_OFF; LED_GREEN_OFF; LED_BLUE_OFF;
    LED_RED_ON; delayMs(100); LED_RED_OFF; delayMs(900);
}

static void initialize(void)
{
    gpioCfgPin(PIN_LED_R, OUT, PULL_UP);
    gpioSetPin(PIN_LED_R);
    gpioCfgPin(PIN_LED_G, OUT, PULL_UP);
    gpioSetPin(PIN_LED_G);
    gpioCfgPin(PIN_LED_B, OUT, PULL_UP);
    gpioSetPin(PIN_LED_B);
    gpioCfgPin(PIN_FP_RST, OUT, PULL_UP);
    gpioSetPin(PIN_FP_RST);
    gpioCfgPin(PIN_FP_INT, IN, PULL_DOWN);
    uartInit(57600);
    while(fpInit()) {
        fpmErrIndication();
    }
}

int32_t main(void)
{
    UINT8 retval, prevFpState;

    initialize();

    // wake up fpm
    while(fpWakeup()) {
        fpmErrIndication();
    }

    // enroll finger
    fpState = FP_STATE_START;
    while(1) {
        prevFpState = fpState;
        fpEnroll();
        // excute the INDICATION according to the fpState
        switch(fpState) {
        case FP_STATE_WAIT_FINGER_OFF2ENROLL:
        case FP_STATE_WAIT_FINGER_OFF2OK:
        case FP_STATE_WAIT_FINGER_OFF2ERR:
            if(FP_STATE_ENROLL_PROC == prevFpState) {
                fingerPressIndication();
            }
            break;
        case FP_STATE_END_OK:
            okIndication();
            break;
        case FP_STATE_END_ERR:
            errIndication();
            break;
        }
        // one finger enrollment is finished
        if(FP_STATE_DEFAULT == fpState) {
            break;
        }
        LED_BLUE_TOGGLE;
    }

    LED_BLUE_OFF;

    // identify finger
    while(1) {
        retval = fpIdentify();
        if(HZERR_SUCCESS == retval) {
            okIndication();
        }
        else if(HZERR_NO_FINGER != retval) {
            errIndication();
        }
        LED_BLUE_TOGGLE;
        LED_GREEN_TOGGLE;
        LED_RED_TOGGLE;
    }

    // example finish
    //while(1);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/



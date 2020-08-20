#include "fpmComm.h"
#include "fp.h"
#include "hzGpio.h"
#include "hzMisc.h"
#include "hzUart.h"
#include "hzDelay.h"

UINT16 fpIndex;
UINT8 fpState = FP_STATE_DEFAULT;
UINT8 fpPressCnt = 0;
UINT8 fpValidPressCnt = 0;

/*!
    \brief      wake up the fpm device
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpWakeup(void)
{
    UINT8 retval, dat;
    UINT32 len, endTime;

    // clear the reset pin at least 10us to wakeup
    gpioClrPin(PIN_FP_RST);
    delayUs(100);
    gpioSetPin(PIN_FP_RST);

    // wait the 0x55 from the device to finish the wakeup process
    endTime = getEndTime(WAKE_UP_TIME);
    len = 1;
    for (;;) {
#if TIMER_TYPE == DECREASE_TIMER
        if(getCurTime() < endTime) {
            return HZERR_RSP_TIMEOUT;
        }
#elif TIMER_TYPE == INCREASE_TIMER
        if(getCurTime() > endTime) {
            return HZERR_RSP_TIMEOUT;
        }
#else
#error "incorrect timer type!"
#endif
        retval = uartReceive(&dat, &len);
        if (!retval && (dat == 0x55))
            return HZERR_SUCCESS;
    }
}

/*!
    \brief      initialize the module and config it to sleep mode
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpInit(void)
{
    UINT8 retval;
    UINT32 param;

    retval = fpWakeup();
    if (retval)
        return retval;
    // config the FPID_BINDING_SPAN as needed
    retval = fpmGetParam(&param, NULL);
    if (retval)
        return retval;
    if (FP_GROUP_SIZE != ((param >> 9) & 0x0F)) {
        param &= (~(0x0F << 9));
        param |= (FP_GROUP_SIZE << 9);
        retval = fpmSetParam(param, NULL);
        if (retval)
            return retval;
    }
    // set the module to sleep mode with low power finger detection
    // enable after initialize
    retval = fpSleep();
    return retval;
}

/*!
    \brief      main fp handle for fingerprint enroll. Loop this function
    \           to enroll a finger. 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void fpEnroll(void)
{
    UINT8 retval;

    switch (fpState) {
        case FP_STATE_START:
            // get an empty index from fpm first while enrolling a finger
            retval = fpmGetEmptyIndex(&fpIndex, NULL);
            if (!retval)
            {
                fpPressCnt = 0;
                fpValidPressCnt = 0;
                fpState = FP_STATE_ENROLL_PROC;
            }
            else {
                fpState = FP_STATE_END_ERR;
            }
            break;
        case FP_STATE_WAIT_FINGER_OFF2ENROLL:
        case FP_STATE_WAIT_FINGER_OFF2OK:
        case FP_STATE_WAIT_FINGER_OFF2ERR:
            // wait the finger toke off here
            retval = fpmDetectFinger(NULL);
            if (HZERR_NO_FINGER == retval) {
                fpState = (fpState == FP_STATE_WAIT_FINGER_OFF2ENROLL) ? FP_STATE_ENROLL_PROC : \
                          (fpState == FP_STATE_WAIT_FINGER_OFF2OK) ? FP_STATE_END_OK : FP_STATE_END_ERR;
            }
            break;
        case FP_STATE_ENROLL_PROC:
            // wait the finger press on and excute the enroll process
            retval = fpmDetectFinger(NULL);
            if (HZERR_SUCCESS == retval) {
                retval = fpmEnrollFinger(fpIndex, MINI_VALID_PRESS, fpValidPressCnt, NULL);
                if (HZERR_CONTINUE == retval) {
                    // a valid finger press here. switch to FP_STATE_WAIT_FINGER_OFF2ENROLL
                    // state to wait finger off and do next finger press again.
                    fpValidPressCnt++;
                    fpPressCnt++;
                    fpState = FP_STATE_WAIT_FINGER_OFF2ENROLL;
                }
                else if (HZERR_SUCCESS == retval) {
                    // enroll success and wait the finger off to finish this enrollment
                    fpState = FP_STATE_WAIT_FINGER_OFF2OK;                   
                }
                else {
                    // force to finish the enrollment if the finger press exceed MAX_PRESS_CNT
                    if (++fpPressCnt >= MAX_PRESS_CNT) {
                        fpValidPressCnt = MINI_VALID_PRESS;
                        retval = fpmEnrollFinger(fpIndex, MINI_VALID_PRESS, fpValidPressCnt, NULL);
                        if (retval) 
                            fpState = FP_STATE_WAIT_FINGER_OFF2ERR;
                        else
                            fpState = FP_STATE_WAIT_FINGER_OFF2OK;
                    }
                    else {
                        fpState = FP_STATE_WAIT_FINGER_OFF2ENROLL;
                    }
                }
            }
            break;
        case FP_STATE_END_OK:
        case FP_STATE_END_ERR:
            fpState = FP_STATE_DEFAULT;
            break;
        default: break;
    }
}

/*!
    \brief      main fp handle for fingerprint identify
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpIdentify(void)
{
    UINT8 retval; //update;

    retval = fpmDetectFinger(NULL);
    if (retval)
        return retval;
    retval = fpmIdentifyFinger(&fpIndex, NULL);
    return retval;
}

/*!
    \brief      fingerprint update after identified successfully
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpUpdate(void)
{
    UINT8 retval, update;

    retval = fpmUpdateFinger(&update, NULL);
    return retval;
}

/*!
    \brief      delete one enrolled fingerprint
    \param[in]  index: fingerprint index to delete
    \param[out] none
    \retval     error code
*/
UINT8 fpDelete(UINT16 index)
{
    UINT8 retval;

    retval = fpmDeleteFinger(index, index, NULL);
    return retval;
}

/*!
    \brief      delete all enrolled fingerprint
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpDeleteAll(void)
{
    UINT8 retval;

    retval = fpmDeleteFinger(0, 1000, NULL);
    return retval;
}

/*!
    \brief      config the fpm to sleep with finger detect enable
    \param[in]  none
    \param[out] none
    \retval     error code
*/
UINT8 fpSleep(void)
{
    UINT8 retval;

    retval = fpmSetSleepMode(0x01, NULL);
    return retval;
}


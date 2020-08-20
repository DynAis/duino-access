#include "fpmComm.h"
#include "auth.h"
#include "Arduino.h"
#include <string.h>

// 16bit checksum calculation
UINT16 calChksum16bit(UINT8 *buf, UINT32 len)
{
    UINT16 checksum = 0;

    while (len)
    {
        checksum += *buf;
        buf++;
        len--;
    }

    return checksum;
}

// xor calculation.
UINT8 calXor8bit(UINT8 *buf, UINT32 len)
{
    UINT8 xorCal = 0;

    while (len)
    {
        xorCal ^= *buf;
        buf++;
        len--;
    }

    return xorCal;
}

/*!
    \brief      sends a command packet to the device
    \param[in]  cmdPkt: command packet
                *bdata: extention block data
                *signature: host signature send to the device
    \param[out] none
    \retval     void
*/
static void fpmSendCmd(ST_CMD_HOZO cmdPkt, UINT8 *bdata, UINT8 *signature)
{
    UINT16 chksum;
    UINT32 i;

    write((UINT8 *)&cmdPkt, sizeof(ST_CMD_HOZO));
    if (((NULL != cmdPkt.exlen) && (NULL != bdata)) || (NULL != signature)) {
        chksum = 0;
        if (NULL != signature) {
            for (i = 0; i < SIGNATURE_SIZE; i++) {
                chksum += signature[i];
            }
            cmdPkt.exlen -= SIGNATURE_SIZE;
        }
        if (NULL != bdata) {
            for (i = 0; i < cmdPkt.exlen; i++) {
                chksum += bdata[i];
            }
            write(bdata, cmdPkt.exlen);
        }
        if (NULL != signature) {
            write(signature, SIGNATURE_SIZE);
        }
        write((UINT8 *)&chksum, sizeof(chksum));
    }
}

/*!
    \brief      receives a respond from the device
    \param[in]  timeout: time out in ms unit
    \param[out] *rspPkt: respond packet
                *bdata: extention block data
                *signature: device signature read from device
    \retval     error code
*/
static UINT8 fpmGetRsp(ST_RSP_HOZO *rspPkt, UINT8 *bdata, UINT8 *signature, UINT32 timeout)
{
    UINT8 retval;
    UINT16 chksum, tail = 0, head = 0;
    UINT32 len, rspEndTime;
    ST_RSP_HOZO *rspPacket;
    UINT8 rspBuf[COMM_BUFFER_SIZE];

    rspEndTime = getEndTime(timeout);
    while(1) {
#if TIMER_TYPE == DECREASE_TIMER
        if(getCurTime() < rspEndTime) {
            return HZERR_RSP_TIMEOUT;
        }
#elif TIMER_TYPE == INCREASE_TIMER
        if(getCurTime() > rspEndTime) {
            return HZERR_RSP_TIMEOUT;
        }
#else
#error "incorrect timer type!"
#endif
        if ((COMM_BUFFER_SIZE - head) >= UART_FIFO_SIZE)
        {
            retval = read(&rspBuf[head], &len);
            // //0-5-2-1
            // Serial.print("in 0-5-2-1, retval=");Serial.print(retval);
            // Serial.print(", head=");Serial.print(head);
            // Serial.print(", rspBuf=");Serial.println(rspBuf[head]);

            if (0 == retval) {
                head += len;
            }
        }
        if ((head - tail) >= sizeof(ST_RSP_HOZO)) {
            if (HZ_RSP_HEADER == rspBuf[tail]) {
                rspPacket = (ST_RSP_HOZO *)&rspBuf[tail];
                if (rspPacket->xorchk == calXor8bit((UINT8 *)rspPacket, sizeof(ST_RSP_HOZO) - 1)) {
                    if (0 != rspPacket->exlen) {
                        if ((head - tail) >= (sizeof(ST_RSP_HOZO) + rspPacket->exlen + 2)) {
                            chksum = *(UINT16 *)&rspBuf[tail + sizeof(ST_RSP_HOZO) + rspPacket->exlen];
                            if (chksum == calChksum16bit(&rspBuf[tail + sizeof(ST_RSP_HOZO)], rspPacket->exlen)) {
                                if (NULL != signature) {
                                    memcpy(signature, \
                                           &rspBuf[tail + sizeof(ST_RSP_HOZO) + rspPacket->exlen - SIGNATURE_SIZE], \
                                           SIGNATURE_SIZE);
                                }
                                if (NULL != bdata) {
                                    memcpy(bdata, &rspBuf[tail + sizeof(ST_RSP_HOZO)], \
                                           (NULL == signature) ? rspPacket->exlen : (rspPacket->exlen - SIGNATURE_SIZE));
                                }
                            }
                            else {
                                return HZERR_RSP_ERROR;
                            }
                        }
                        else {
                            continue;
                        }
                    }
                    *rspPkt = *rspPacket;
                    return HZERR_SUCCESS;
                }
                else {
                    return HZERR_RSP_ERROR;
                }
            }
            else {
                tail++;
            }
        }
    }
}

/*!
    \brief      excute a single fpm command.
    \param[in]  cmd: command
                funcode: function code
                cmdData: command data
                timeout: time out for respond in ms unit
    \param[out] *rspData: respond data
    \param[i/o] *data: extention block data to transmission
                *length: extention block data length to transmission
                *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmCommand(UINT8 cmd, UINT8 funcode, UINT32 cmdData, UINT32 *rspData, \
                        UINT8 *data, UINT32 *length,\
                        UINT32 timeout, UINT8 *rollingKey)
{
    UINT8 retval, i = 0;
    ST_CMD_HOZO cmdPkt;
    ST_RSP_HOZO rspPkt;

    // //0-5-1
    // Serial.print("in 0-5-1, retval=");Serial.println(retval);

    cmdPkt.header = HZ_CMD_HEADER;
    cmdPkt.command = cmd;
    cmdPkt.funcode = funcode;
    cmdPkt.cmddata = cmdData;
    cmdPkt.exlen = (NULL == length) ? 0 : (*length);
    cmdPkt.xorchk = calXor8bit((UINT8 *)&cmdPkt, sizeof(ST_CMD_HOZO) - 1);
    UINT8 *signature = NULL;
    if(rollingKey!=NULL){
       cmdPkt.exlen +=  SIGNATURE_SIZE;
       cmdPkt.xorchk = calXor8bit((UINT8 *)&cmdPkt, sizeof(ST_CMD_HOZO) - 1);
       UINT8 signatureBuf[SIGNATURE_SIZE];
       signature=signatureBuf;
       UINT8  ramdom[SIGNATURE_SIZE/2];
       genRandom(ramdom,SIGNATURE_SIZE/2);
       genSignature(ramdom,(UINT8*)&cmdPkt,rollingKey,signature);
    }
    do {
        // //0-5-2
        // Serial.print("in 0-5-2, retval=");Serial.println(retval);
        fpmSendCmd(cmdPkt, data, signature);
        retval = fpmGetRsp(&rspPkt, data, signature, timeout);
        if (HZERR_SUCCESS == retval) {
            if (cmdPkt.command == rspPkt.command) {
                retval = rspPkt.rspcode;
                if (NULL != rspData) {
                    *rspData = rspPkt.rspdata;
                }
                if (NULL != length) {
                    *length = (NULL == signature) ? rspPkt.exlen : (rspPkt.exlen - SIGNATURE_SIZE);
                }
                if(rollingKey != NULL && signature!=NULL) {
                    UINT8 hash[SIGNATURE_SIZE/2];
                    int ret = doSignatureMatch(signature, (UINT8*)&rspPkt,rollingKey + SIGNATURE_SIZE / 2, hash);
                    if(ret==0){
                        updateSignKey(rollingKey,signature);
                    }else{
                       retval = HZERR_ILLEGAL_DEVICE;
                    }
                }
            }
            else {
                retval = HZERR_RSP_INVALID_CMD;
            }
            break;
        }
    } while (i++ < CMD_REPEAT_CYCLE);
    return retval;
}

/*!
    \brief      gets the devive's information.
    \param[in]  none
    \param[out] *deviceInfo: the device's information
    \retval     error code
*/
UINT8 fpmGetDeviceInfo(ST_DEV_INFO_HOZO *deviceInfo)
{
    UINT8 retval;

    retval = fpmCommand(HZ_GET_DEVICE_INFO, 0, 0, NULL, (UINT8 *)deviceInfo, NULL, TIMEOUT_VERY_SHORT, NULL);
    return retval;
}

/*!
    \brief      gets the devive's signature.
    \param[in]  none
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmGetSignature(UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_GET_SIGNATURE, 0, 0, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    return retval;
}

/*!
    \brief      sets the devive's signature.
    \param[in]  *newKey: The Key need to be written
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmSetSignature(UINT8 *newKey, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 len = SIGNATURE_SIZE;

    retval = fpmCommand(HZ_SET_SIGNATURE, 0, 0, NULL, newKey, &len, TIMEOUT_SHORT, rollingKey);
    return retval;
}

/*!
    \brief      gets the devive's parameter.
    \param[in]  none
    \param[out] *param: parameter read from the device
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmGetParam(UINT32 *param, UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_GET_PARAM, 0, 0, param, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    return retval;
}

/*!
    \brief      sets the devive's parameter.
    \param[in]  param: the device's parameter need to be written
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmSetParam(UINT32 param, UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_SET_PARAM, 0, param, NULL, NULL, NULL, TIMEOUT_LONG, rollingKey);
    return retval;
}

/*!
    \brief      gets the minimum registrable fingerprint index number.
    \param[in]  none
    \param[out] *index: index number
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmGetEmptyIndex(UINT16 *index, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 rspData;

    retval = fpmCommand(HZ_GET_EMPTY_INDEX, 0, 0, &rspData, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    *index = (UINT16)rspData;
    return retval;
}

/*!
    \brief      gets the index number status of the fingerprint database.
    \param[in]  index: the index number whose status want to get
    \param[out] *status: index number's status
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmGetIndexStatus(UINT16 index, UINT8 *status, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 rspData;

    retval = fpmCommand(HZ_GET_INDEX_STATUS, 0, (UINT32)index, &rspData, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    *status = (UINT8)rspData;
    return retval;
}

/*!
    \brief      Sets sleep mode to make device enter a low-power sleep mode.
    \param[in]  sleepMode: the sleep mode want to set
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmSetSleepMode(UINT32 sleepMode, UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_SET_SLEEP_MODE, 0, sleepMode, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
     return retval;
}

/*!
    \brief      Checks for finger pressing on sensor surface. If a finger is
                pressed on the surface of the sensor, the image is cached in
                the ImageBuffer for the next step usage.
    \param[in]  none
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmDetectFinger(UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_DETECT_FINGER, 0, 0, NULL, NULL, NULL, TIMEOUT_NORMAL, rollingKey);
    return retval;
}

/*!
    \brief      Enrolls a fingerprint to the specified index number of fingerprint database.
    \param[in]  index: the index number for the enrollment
                miniValidPress: the miniment valid fingerprint press to finish the enrollment
                currentValidPress: the valid press count currently, set this param as
                                   (miniValidPress + 1) could force the finish the enrollment
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmEnrollFinger(UINT16 index, UINT8 miniValidPress, UINT8 currentValidPress, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 cmdData;

    cmdData = ((UINT32)currentValidPress << 24) | \
              ((UINT32)miniValidPress << 16) | (UINT32)index;
    retval = fpmCommand(HZ_ENROLL_FINGER, 0, cmdData, NULL, NULL, NULL, TIMEOUT_LONG, rollingKey);
    return retval;
}

/*!
    \brief      Performs 1:1 verification between the fingerprint in ImageBuffer and the
                fingerprint with the specified index number.
    \param[in]  index: the index number for the veirification
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmVerifyFinger(UINT16 index, UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_VERIFY_FINGER, 0, (UINT32)index, NULL, NULL, NULL, TIMEOUT_NORMAL, rollingKey);
    return retval;
}

/*!
    \brief      Performs 1:N identification between the fingerprint in ImageBuffer and
    \           all fingerprints enrolled in the fingerprint database.
    \param[in]  none
    \param[out] *index: the index number matched
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmIdentifyFinger(UINT16 *index, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 rspData;

    retval = fpmCommand(HZ_IDENTIFY_FINGER, 0, 0, &rspData, NULL, NULL, TIMEOUT_NORMAL, rollingKey);
    *index = (UINT16)rspData;
    return retval;
}

/*!
    \brief      Deletes the fingerprints with specified index numbers.
    \param[in]  startIndex: the start index of the delete section
                terminalIndex: the terminal index of the delete section
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmDeleteFinger(UINT16 startIndex, UINT16 terminalIndex, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 cmdData;

    cmdData = ((UINT32)terminalIndex << 16) | \
              ((UINT32)startIndex & 0x0000FFFF);
    retval = fpmCommand(HZ_DELETE_FINGER, 0, cmdData, NULL, NULL, NULL, TIMEOUT_LONG, rollingKey);
    return retval;
}

/*!
    \brief      Updates the verified or identified fingerprint data to
                improve the enrollment information, so as to improve the
                recognition rate.
    \param[in]  none
    \param[out] *update: fingerprint update result
        \arg    0: no data updated
        \arg    1: data updated
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmUpdateFinger(UINT8 *update, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 rspData;

    retval = fpmCommand(HZ_UPDATE_FINGER, 0, 0, &rspData, NULL, NULL, TIMEOUT_LONG, rollingKey);
    *update = (UINT8)rspData;
    return retval;
}

/*!
    \brief      Extracts the fingerprint data in the image buffer to the
                fingerprint data buffer with the specified number.
    \param[in]  bufferIndex: the buffer index to save the fingerprint data
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmExtractFinger(UINT8 bufferIndex, UINT8 *rollingKey)
{
    UINT8 retval;

    retval = fpmCommand(HZ_EXTRACT_FINGER_DATA, 0, (UINT32)bufferIndex, NULL, NULL, NULL, TIMEOUT_NORMAL, rollingKey);
    return retval;
}

/*!
    \brief      Reads the fingerprint image data in the image buffer.
    \param[in]  none
    \param[out] *imageData: the image data has been read
                *width: width of the image
                *height: height of the image
                *dpi: the resolution of the image
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmReadImageBuffer(UINT8 *imageData, UINT16 *width, UINT16 *height, UINT16 *dpi, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    retval = fpmCommand(HZ_READ_IMAGE_BUFFER, 0, 0, &temp, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    *width = (UINT16)(temp & 0x000003FF);
    *height = (UINT16)((temp >> 10) & 0x000003FF);
    *dpi = (UINT16)((temp >> 20) & 0x000003FF);
    blockNum = ((*width)*(*height) + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        retval = fpmCommand(HZ_READ_IMAGE_BUFFER, 1, temp, NULL, &imageData[i*blockSize], NULL, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

/*!
    \brief      Writes the fingerprint image data to the image buffer.
    \param[in]  *imageData: the image data to be written
                width: width of the image
                height: height of the image
                dpi: the resolution of the image
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmWriteImageBuffer(UINT8 *imageData, UINT16 width, UINT16 height, UINT16 dpi, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum, size;

    blockSize = (rollingKey==NULL) ? blockSize:blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    temp = ((UINT32)dpi << 20) | ((UINT32)height << 10) | (UINT32)width;
    retval = fpmCommand(HZ_WRITE_IMAGE_BUFFER, 0, temp, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    blockNum = (width*height + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        if (i == (blockNum - 1)) {
            size = (width * height) % blockSize;
        }
        else {
            size = blockSize;
        }
        retval = fpmCommand(HZ_WRITE_IMAGE_BUFFER, 1, temp, NULL, &imageData[i*blockSize], &size, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

/*!
    \brief      Reads fingerprint data with specified index number.
    \param[in]  index: the index number to be read
                format: the fingerprint data format
    \param[out] *fingerData: the fingerprint data that has been read
                *length: length of the fingerprint data
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmReadFingerData(UINT16 index, UINT8 format, UINT8 *fingerData, UINT16 *length, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    temp = ((UINT32)format << 13) | (UINT32)index;
    retval = fpmCommand(HZ_READ_FINGER_DATA, 0, temp, &temp, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    *length = temp;
    blockNum = (temp + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        retval = fpmCommand(HZ_READ_FINGER_DATA, 1, temp, NULL, &fingerData[i*blockSize], NULL, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

/*!
    \brief      Writes fingerprint data to the specified index number.
    \param[in]  index: the index number to be written
                format: the fingerprint data format
                *fingerData: the fingerprint data to be written
                length: length of the fingerprint data
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmWriteFingerData(UINT16 index, UINT8 format, UINT8 *fingerData, UINT16 length, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum, size;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    temp = ((UINT32)format << 13) | (UINT32)index;
    retval = fpmCommand(HZ_WRITE_FINGER_DATA, 0, temp, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    blockNum = (length + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        if (i == (blockNum - 1)) {
            size = length % blockSize;
        }
        else {
            size = blockSize;
        }
        retval = fpmCommand(HZ_WRITE_FINGER_DATA, 1, temp, NULL, &fingerData[i*blockSize], &size, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

/*!
    \brief      Reads fingerprint data buffer with specified number.
    \param[in]  index: the index number to be read
                format: the fingerprint data format
    \param[out] *fingerData: the fingerprint data that has been read
                *length: length of the fingerprint data
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmReadFingerBuffer(UINT8 bufferIndex, UINT8 format, UINT8 *fingerData, UINT16 *length, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    temp = ((UINT32)format << 13) | (UINT32)bufferIndex;
    retval = fpmCommand(HZ_READ_FINGER_BUFFER, 0, temp, &temp, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    *length = temp;
    blockNum = (temp + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        retval = fpmCommand(HZ_READ_FINGER_BUFFER, 1, temp, NULL, &fingerData[i*blockSize], NULL, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

/*!
    \brief      Writes fingerprint data to the data buffer with specified number.
    \param[in]  index: the index number to be written
                format: the fingerprint data format
                *fingerData: the fingerprint data to be written
                length: length of the fingerprint data
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmWriteFingerBuffer(UINT8 bufferIndex, UINT8 format, UINT8 *fingerData, UINT16 length, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum, size;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    temp = ((UINT32)format << 13) | (UINT32)bufferIndex;
    retval = fpmCommand(HZ_WRITE_FINGER_BUFFER, 0, temp, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    blockNum = (length + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        if (i == (blockNum - 1)) {
            size = length % blockSize;
        }
        else {
            size = blockSize;
        }
        retval = fpmCommand(HZ_WRITE_FINGER_BUFFER, 1, temp, NULL, &fingerData[i*blockSize], &size, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}

// 32bit checksum calculation
static UINT32 calChksum32bit(UINT8 *buf, UINT32 len)
{
    UINT32 checksum = 0;

    while (len)
    {
        checksum += *buf;
        buf++;
        len--;
    }

    return checksum;
}

/*!
    \brief      Upgrades firmware of device.
    \param[in]  *fwLoader: the fwLoader.bin data
                fwLoaderLen: the the length of fwLoader.bin
                *firmware: the firmware data
                firmwareLen: length of the firmware data
    \param[out] none
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmWupdateFirmware(UINT8 *fwLoader, UINT32 fwLoaderLen, UINT8 *firmware, UINT32 firmwareLen, UINT8 *rollingKey)
{
    UINT8 retval, j = 0;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum, size;
    UINT8 *fwData;
    UINT32 fwLen;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    do {
        if (0 == j) {
            fwData = fwLoader;
            fwLen = fwLoaderLen;
        }
        else{
            fwData = firmware;
            fwLen = firmwareLen;
        }
        retval = fpmCommand(HZ_UPDATE_FIRMWARE, 0, fwLen, NULL, NULL, NULL, TIMEOUT_SHORT, rollingKey);
        if (HZERR_SUCCESS != retval) {
            return retval;
        }
        blockNum = (fwLoaderLen + blockSize - 1)/blockSize;
        for (i = 0; i < blockNum; i++) {
            temp = (i << 10) | blockSize;
            if (i == (blockNum - 1)) {
                size = fwLoaderLen % blockSize;
            }
            else {
                size = blockSize;
            }
            retval = fpmCommand(HZ_UPDATE_FIRMWARE, 1, temp, NULL, &fwData[i*blockSize], &size, TIMEOUT_NORMAL, rollingKey);
            if (HZERR_SUCCESS != retval) {
                break;
            }
        }
        if (HZERR_SUCCESS != retval) {
            return retval;
        }
        UINT32 chkSum =calChksum32bit(fwData,fwLen);
        retval = fpmCommand(HZ_UPDATE_FIRMWARE, 2, chkSum, NULL, NULL, NULL, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            return retval;
        }

        retval = fpmCommand(HZ_UPDATE_FIRMWARE, 3, 0, NULL, NULL, NULL, TIMEOUT_NORMAL, rollingKey);

        if (HZERR_SUCCESS != retval) {
            return retval;
        }
    } while (j++ < 1);
    return retval;
}

/*!
    \brief      read the enrolled list.
    \param[in]  none
    \param[out] *list: enrollment list read from the device
                *length: enrollment list length
    \param[i/o] *rollingKey: The key used to generate the host's signature and verify the device's signature.
                            It will automatically update after  the command  succeeded.
                            Set rollingkey as NULL if bidirectional signature verification is dismissed.
    \retval     error code
*/
UINT8 fpmReadEnrollList(UINT8 *list, UINT16 *length, UINT8 *rollingKey)
{
    UINT8 retval;
    UINT32 temp;
    UINT16 blockSize = COMM_BUFFER_SIZE - sizeof(ST_RSP_HOZO) - 2;
    UINT32 i, blockNum;

    blockSize = (rollingKey==NULL) ? blockSize : blockSize-SIGNATURE_SIZE;
    blockSize = (blockSize > 512) ? 512 : blockSize;
    retval = fpmCommand(HZ_READ_ENROLL_LIST, 0, 0, &temp, NULL, NULL, TIMEOUT_SHORT, rollingKey);
    if (HZERR_SUCCESS != retval) {
        return retval;
    }
    *length = temp;
    blockNum = (temp + blockSize - 1)/blockSize;
    for (i = 0; i < blockNum; i++) {
        temp = (i << 10) | blockSize;
        retval = fpmCommand(HZ_READ_ENROLL_LIST, 1, temp, NULL, &list[i*blockSize], NULL, TIMEOUT_NORMAL, rollingKey);
        if (HZERR_SUCCESS != retval) {
            break;
        }
    }
    return retval;
}


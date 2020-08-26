#ifndef _FPM_COMM_H_
#define _FPM_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hzTypes.h"
#include "hzDevice.h"

// command repeat cycle if failed to communication with fpm
#define	CMD_REPEAT_CYCLE				    2

// communication response time out definition in MS unit
#define	TIMEOUT_VERY_SHORT                 	(100U) // unit: 1ms
#define	TIMEOUT_SHORT                 		(500U) // unit: 1ms
#define	TIMEOUT_NORMAL                		(1000U) // unit: 1ms
#define	TIMEOUT_LONG                  		(3000U) // unit: 1ms

#if defined (__GNUC__)
#ifndef __PACKED 
#define __PACKED
#endif
#pragma pack(1)
#else  
#define __PACKED __packed
#endif
//Define Command, Response Packet
typedef __PACKED struct _ST_CMD_HOZO_
{
	UINT8	header;
	UINT8	command;
	UINT8	funcode;
	UINT32	cmddata;
	UINT16	exlen;
	UINT8	xorchk;
} ST_CMD_HOZO, *PST_CMD_HOZO;

typedef __packed struct _ST_RSP_HOZO_
{
	UINT8	header;
	UINT8	command;
	UINT8	rspcode;
	UINT32	rspdata;
	UINT16	exlen;
	UINT8	xorchk;
} ST_RSP_HOZO, *PST_RSP_HOZO;

typedef __PACKED struct _ST_DEV_INFO_HOZO_
{
	UINT16 	fwVer; 
	UINT16 	fplibVer; 
	UINT32 	baudrate; 
	UINT16 	maxCount; 
	UINT16 	enrollCount; 
	UINT8	matchingThreshold; 
	UINT8	uniqueConstrain; 
	UINT8	strictEnrollment; 
	UINT8	sampleSize; 
	UINT8	signature; 
	UINT8	reserve[15]; 
} ST_DEV_INFO_HOZO, *PST_DEV_INFO_HOZO;

#if defined (__GNUC__)
#pragma pack()
#endif

// Header code definition
#define	HZ_CMD_HEADER						0x33
#define HZ_RSP_HEADER					    0xCC

// signature size definition
#define SIGNATURE_SIZE                      32U

// 0x00 ~ 0x0F : device config code
#define	HZ_GET_DEVICE_INFO  				0x00
#define	HZ_GET_SIGNATURE					0x01
#define	HZ_SET_SIGNATURE					0x02
#define	HZ_GET_PARAM        				0x03
#define	HZ_SET_PARAM        				0x04
#define	HZ_GET_EMPTY_INDEX					0x05
#define	HZ_GET_INDEX_STATUS					0x06
#define	HZ_SET_SLEEP_MODE					0x07

// 0x10 ~0x1F : finger fucntion code
#define	HZ_DETECT_FINGER					0x10
#define	HZ_ENROLL_FINGER    				0x11
#define	HZ_VERIFY_FINGER    				0x12
#define	HZ_IDENTIFY_FINGER    				0x13
#define	HZ_DELETE_FINGER    				0x14
#define	HZ_UPDATE_FINGER    				0x15
#define	HZ_EXTRACT_FINGER_DATA 				0x16

// 0x20 ~ 0x2F : function code with block data
#define	HZ_READ_IMAGE_BUFFER				0x20
#define	HZ_WRITE_IMAGE_BUFFER       		0x21
#define	HZ_READ_FINGER_DATA					0x22
#define	HZ_WRITE_FINGER_DATA				0x23
#define	HZ_READ_FINGER_BUFFER				0x24
#define	HZ_WRITE_FINGER_BUFFER				0x25
#define	HZ_UPDATE_FIRMWARE					0x26
#define HZ_READ_ENROLL_LIST					0x27

// response error code, the same as fpLib.h
#define	HZERR_SUCCESS					    0x00
#define	HZERR_MEM_ERR			            0x01	
#define	HZERR_PARAM_ERR		                0x02
#define	HZERR_COMBINE_FAIL  	            0x03
#define	HZERR_BAD_IMAGE		                0x04
#define	HZERR_ID_EMPTY				        0x05
#define	HZERR_ID_EXIST			            0x06
#define	HZERR_FPLIB_EMPTY			        0x07
#define	HZERR_NO_EMPTY_ID			        0x08
#define	HZERR_INVALID_TMPL_DATA		        0x09
#define	HZERR_FP_DUPLICATION			    0x0A
#define	HZERR_UNMATCH				        0x0B
#define	HZERR_SEARCH_FAIL			        0x0C
#define	HZERR_ENROLL_FAIL					0x0D
#define	HZERR_FLASH_ERR					    0x0E
#define	HZERR_INVALID_ID			        0x0F
#define	HZERR_SIMILAR_AREA			        0x10
#define HZERR_EMPTY_IMAGE          	        0x11
#define	HZERR_SENSOR_ERR				    0x12
#define	HZERR_NO_FINGER			            0x13
#define	HZERR_IMG_CAP_FAIL				    0x14
#define	HZERR_RESIDUAL_IMAGE			    0x15
#define	HZERR_CONTINUE			            0x16
#define	HZERR_FPLIB_UNFORMAT			    0x17

#define HZERR_FRAME_ERR                     0x30
#define HZERR_BDATA_CSUM_ERR                0x31
#define HZERR_CMD_ERR                       0x32
#define	HZERR_SIGNATURE_ERR                 0x33
#define HZERR_FUNCODE_ERR					0x34
#define HZERR_FW_SIZE_ERR					0x35
#define HZERR_FW_CSUM_ERR					0x36
#define HZERR_FW_VERIFY_ERR					0x37

#define	HZERR_ILLEGAL_DEVICE                0xFC
#define HZERR_RSP_ERROR						0xFD
#define HZERR_RSP_TIMEOUT					0xFE
#define HZERR_RSP_INVALID_CMD				0xFF

UINT8 fpmGetDeviceInfo(ST_DEV_INFO_HOZO *deviceInfo);
UINT8 fpmGetSignature(UINT8 *rollingKey);
UINT8 fpmSetSignature(UINT8 *newKey, UINT8 *rollingKey);
UINT8 fpmGetParam(UINT32 *param, UINT8 *rollingKey);
UINT8 fpmSetParam(UINT32 param, UINT8 *rollingKey);
UINT8 fpmGetEmptyIndex(UINT16 *index, UINT8 *rollingKey);
UINT8 fpmGetIndexStatus(UINT16 index, UINT8 *status, UINT8 *rollingKey);
UINT8 fpmSetSleepMode(UINT32 sleepMode, UINT8 *rollingKey);
UINT8 fpmDetectFinger(UINT8 *rollingKey);
UINT8 fpmEnrollFinger(UINT16 index, UINT8 miniValidPress, UINT8 currentValidPress, UINT8 *rollingKey);
UINT8 fpmVerifyFinger(UINT16 index, UINT8 *rollingKey);
UINT8 fpmIdentifyFinger(UINT16 *index, UINT8 *rollingKey);
UINT8 fpmDeleteFinger(UINT16 startIndex, UINT16 terminalIndex, UINT8 *rollingKey);
UINT8 fpmUpdateFinger(UINT8 *update, UINT8 *rollingKey);
UINT8 fpmExtractFinger(UINT8 bufferIndex, UINT8 *rollingKey);
UINT8 fpmReadImageBuffer(UINT8 *imageData, UINT16 *width, UINT16 *height, UINT16 *dpi, UINT8 *rollingKey);
UINT8 fpmWriteImageBuffer(UINT8 *imageData, UINT16 width, UINT16 height, UINT16 dpi, UINT8 *rollingKey);
UINT8 fpmReadFingerData(UINT16 index, UINT8 format, UINT8 *fingerData, UINT16 *length, UINT8 *rollingKey);
UINT8 fpmWriteFingerData(UINT16 index, UINT8 format, UINT8 *fingerData, UINT16 length, UINT8 *rollingKey);
UINT8 fpmReadFingerBuffer(UINT8 bufferIndex, UINT8 format, UINT8 *fingerData, UINT16 *length, UINT8 *rollingKey);
UINT8 fpmWriteFingerBuffer(UINT8 bufferIndex, UINT8 format, UINT8 *fingerData, UINT16 length, UINT8 *rollingKey);
UINT8 fpmWupdateFirmware(UINT8 *fwLoader, UINT32 fwLoaderLen, UINT8 *firmware, UINT32 firmwareLen, UINT8 *rollingKey);
UINT8 fpmReadEnrollList(UINT8 *list, UINT16 *length, UINT8 *rollingKey);

UINT16 calChksum16bit(UINT8 *buf, UINT32 len);
UINT8 calXor8bit(UINT8 *buf, UINT32 len);
UINT8 fpmCommand(UINT8 cmd, UINT8 funcode, UINT32 cmdData, UINT32 *rspData, \
                        UINT8 *data, UINT32 *length,\
                        UINT32 timeout, UINT8 *rollingKey);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef _FPM_COMM_H_


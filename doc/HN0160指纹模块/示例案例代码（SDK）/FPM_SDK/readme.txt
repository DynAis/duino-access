####################################
#### HOZONOVO ALL RIGHTS RESERVED ####
####################################

fpmSDK implement note:

essential files need to be implemented:
fpmComm.c
fpmComm.h
hzTypes.h
hzDevice.c
hzDevice.h

Only 4 functions (in hzDevice) required to implement the protocol:

1)void write(const UINT8 *data, UINT32 len);	//send out data through UART port

2)UINT8 receive(UINT8 *data, UINT32 *len);	//receive data from fpm with UART

3)UINT32 getEndTime(UINT32 time);	//gets the expected ending timer tick in ms unit

4)UINT32 getCurTime(void);	//gets the current timer tick in ms unit

If bidirectional signature verification is required, there is another function that needs to be implemented:
5）void genRandom(UINT8 *ramdom,UINT8 len) // gen random number for signature

referenece files to realize the basic fingerprint functions:
fp.c
fp.h

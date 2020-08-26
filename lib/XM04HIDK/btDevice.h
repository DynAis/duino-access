#include "hzTypes.h"

typedef struct _BT_CMD_PACK_
{
	UINT32	header;
	UINT8	state;
	UINT8	keystart;
  UINT8 key1;
  UINT8 key2;
  UINT8 key3;
  UINT8 key4;
  UINT8 key5;
  UINT8 key6;
} BT_CMD_PACK;

void btInit(void);
UINT8 char2key(char a);
void btSendKey(UINT8 state, UINT8 key1, UINT8 key2, UINT8 key3, UINT8 key4, UINT8 key5, UINT8 key6);
void btSendPassworld(const char* text);
UINT8 resetBtCompear(void);
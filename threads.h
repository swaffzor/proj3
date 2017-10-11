#include "MKL25Z4.h"
#include "cmsis_os2.h" 

extern osMessageQueueId_t spiMsgQueueID;

void initRTOSobjects(void);

typedef struct{
	char letter;
	int value;
}MSG_T;

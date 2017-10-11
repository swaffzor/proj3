#include "threads.h"
#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5
#include "MKL25Z4.h"                    // Device header

osMessageQueueId_t spiMsgQueueID;

void initRTOSobjects(){
	
	spiMsgQueueID = osMessageQueueNew(5, sizeof(MSG_T), NULL);
	
}
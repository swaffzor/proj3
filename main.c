#include "integer.h"
#include <MKL25Z4.h>
#include "spi_io.h"
#include "sd_io.h"
#include "LEDs.h"
#include "debug.h"
#include "cmsis_os2.h"
#include "threads.h"

void initInterrupt(void);

SD_DEV dev[1];          // Create device descriptor
uint8_t buffer[512];    // Example of your buffer data
volatile uint32_t sum = 0;

extern uint32_t g_idle_counter;

osThreadId_t tid_SDmanager;	//thread ID

void SD_Manager(void * arg) {
	// On first run, init card and write test data to given block (sector_num) in flash. 
	// Then repeatedly read the sector and confirm its contents
	
	int i;
	uint32_t idleDiff = 0;
	DWORD sector_num = 0x23; // Manual wear leveling
  SDRESULTS res;
	
	// Erase buffer
	for (i=0; i<SD_BLK_SIZE; i++)
		buffer[i] = 0;
	
	// Load sample data into buffer
	buffer[0] = 0xDE;
	buffer[1] = 0xAD;
	buffer[2] = 0xC0;
	buffer[3] = 0xDE;
	
	buffer[508] = 0xFE;
	buffer[509] = 0xED;
	buffer[510] = 0xCA;
	buffer[511] = 0xFE;

	idleDiff = g_idle_counter;
  if(SD_Init(dev)!=SD_OK) {
			Control_RGB_LEDs(1,0,0); // Red - init failed
			while (1)
				;
	}
	idleDiff = g_idle_counter - idleDiff;
	
	// Change the data in this sector
	idleDiff = g_idle_counter;
	res = SD_Write(dev, (void*)buffer, sector_num);
	if(res!=SD_OK) {
		Control_RGB_LEDs(1,0,1); // Magenta - write failed
		while (1)
			;
	}
	idleDiff = g_idle_counter - idleDiff;
	
	Control_RGB_LEDs(0,0,1);	// Blue - written ok
	
	while (1) { // Repeat this loop forever, ten times per second
		// Need to insert an osDelay call here 
		idleDiff = g_idle_counter;
		osDelay(4000);
		idleDiff = g_idle_counter - idleDiff;
		
		
		// erase buffer
		for (i=0; i<SD_BLK_SIZE; i++)
			buffer[i] = 0;
		// read block again
		idleDiff = g_idle_counter;
		res = SD_Read(dev, (void*)buffer, sector_num, 0, 512);
		idleDiff = g_idle_counter - idleDiff;
		
		if(res==SD_OK) {
			for (i = 0, sum = 0; i < SD_BLK_SIZE; i++)
				sum += buffer[i];
		
			if (sum == 0x06DC)
				Control_RGB_LEDs(0,1,0); // Green - read was OK
			else
				Control_RGB_LEDs(1,0,0); // Red - checksum failed
		} else {
			Control_RGB_LEDs(1,0,0); // Red - read failed
		}
	}
}

int main(void)
{
	Init_Debug_Signals();
	Init_RGB_LEDs();
	Control_RGB_LEDs(1,1,0);	// Yellow - starting up
	initInterrupt();
	
	osKernelInitialize();
	osThreadNew(SD_Manager, NULL, NULL);		//create thread, returns the thread id number
	initRTOSobjects();
	osKernelStart();		//start multitasking
	
 	// Test function to write a block and then read back, verify
	//SD_Manager();
  
	while (1)
		;
}

void initInterrupt(void){
	/* Configure PORT peripheral. Select GPIO and enable pull-up
	resistors and interrupts on all edges for pins connected to switches */
	//PORTE->PCR[SPI1_IRQn] = PORT_PCR_MUX(1)| PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	
	/* Configure NVIC */
	NVIC_SetPriority(SPI1_IRQn, 2);
	NVIC_ClearPendingIRQ(SPI1_IRQn);
	NVIC_EnableIRQ(SPI1_IRQn);
	
	/* Optional: Configure PRIMASK in case interrupts were disabled. */
	__enable_irq();
}

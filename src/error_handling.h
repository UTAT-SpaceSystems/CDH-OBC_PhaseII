/*
Author: Keenan Burnett
***********************************************************************
* FILE NAME: error_handling.h
*
* PURPOSE:
* This file is to be used for the includes, prototypes, and definitions related to error_handling.c
*
* EXTERNAL VARIABLES:
*
* EXTERNAL REFERENCES: Same a File References.
*
* ABORNOMAL TERMINATION CONDITIONS, ERROR AND WARNING MESSAGES: None yet.
*
* ASSUMPTIONS, CONSTRAINTS, CONDITIONS: None
*
* NOTES:
*
* REQUIREMENTS/ FUNCTIONAL SPECIFICATION REFERENCES:
*
* DEVELOPMENT HISTORY:
* 11/26/2015		Created.
* 12/05/2015		Added Error IDs.
*
*/

#include "global_var.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "can_func.h"


/* Global signals tasks can wait on during High-Sev errors */
uint8_t hk_fdir_signal;
uint8_t time_fdir_signal;
uint8_t coms_fdir_signal;
uint8_t eps_fdir_signal;
uint8_t pay_fdir_signal;
uint8_t opr_fdir_signal;
uint8_t sched_fdir_signal;
uint8_t wdt_fdir_signal;
uint8_t mem_fdir_signal;
uint8_t high_error_array[152];
uint8_t low_error_array[152];
// SSMs will have their own local variables for doing this
// can_func.c will not be allowed to block on any signal, only error reports.

// Note that currently acquired mutex locks should be released during the waiting process.

//ERROR IDs as defined in FDIR document

#define SCHED_SPIMEM_R_ERROR			0x01
#define SCHED_SPIMEM_W_ERROR			0x02
#define SCHED_SPIMEM_CHIP_ERROR			0x03  
#define SCHED_COMMAND_EXEC_ERROR		0x04
#define SCHED_FIFO_RW_ERROR		        0x05
#define HK_FIFO_RW_ERROR				0x06
#define HK_COLLECT_ERROR				0x07
#define HK_SPIMEM_R_ERROR				0x08
#define HK_SPIMEM_W_ERROR				0x1C
#define TM_FIFO_RW_ERROR				0x08
#define SPIMEM_BUSY_CHIP_ERROR			0x0A
#define SPIMEM_CHIP_ERASE_ERROR			0x0B
#define SPIMEM_LOAD_SECTOR_ERROR		0x0C
#define SPIMEM_UPDATE_SPIBUFFER_ERROR	0x0D
#define SPIMEM_ERASE_SECTOR_ERROR		0x0E
#define SPIMEM_WRITE_SECTOR_ERROR		0x0F
#define SPIMEM_WR_ERROR					0x10
#define SPIMEM_ALL_CHIPS_ERROR			0x11
#define RTC_SPIMEM_R_ERROR				0x12
#define MEM_SPIMEM_CHIPS_ERROR			0x13
#define MEM_SPIMEM_MEM_WASH_ERROR		0x14
#define MEM_OTHER_SPIMEM_ERROR			0x15
#define MEM_FIFO_RW_ERROR				0x16
#define EPS_SSM_GET_SENSOR_DATA_ERROR	0x17
#define EPS_SET_VARIABLE_ERROR			0x18
#define OBC_COMS_TC_TM_ERROR			0x19
#define OBC_TC_PACKET_ERROR				0x1A
#define OBC_FIFO_RW_ERROR				0x1B

int errorREPORT(uint8_t task, uint32_t error, uint32_t* data);
int errorASSERT(uint8_t task, uint32_t error, uint8_t* data, SemaphoreHandle_t mutex);
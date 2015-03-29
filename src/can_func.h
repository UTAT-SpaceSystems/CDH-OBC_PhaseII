/*
    Author: Keenan Burnett

	***********************************************************************
	*	FILE NAME:		can_func.h
	*
	*	PURPOSE:		CAN related includes and structures, and includes for can_func.c
	*	
	*
	*	FILE REFERENCES:	sn65hvd234.h, can.h, stdio.h, string.h, board.h, sysclk.h, exceptions.h
	*						pmc.h, conf_board.h, conf_clock.h, pio.h
	*
	*	EXTERNAL VARIABLES:	
	*
	*	EXTERNAL REFERENCES:	Same a File References.
	*
	*	ABORNOMAL TERMINATION CONDITIONS, ERROR AND WARNING MESSAGES: None yet.
	*
	*	ASSUMPTIONS, CONSTRAINTS, CONDITIONS:	None
	*
	*	NOTES:	
	*
	*	REQUIREMENTS/ FUNCTIONAL SPECIFICATION REFERENCES:
	*	New tasks should be written to use as much of CMSIS as possible. The ASF and
	*	FreeRTOS API libraries should also be used whenever possible to make the program
	*	more portable.

	*	DEVELOPMENT HISTORY:
	*	01/02/2015		Created.
	*
	*	02/01/2015		Added a prototype for send_can_command().
	*
	*	02/18/2015		Added a prorotype for request_housekeeping().
	*
	*					Added to the list of ID and message definitions in order to communicate
	*					more effectively with the STK600.
	*
	*					PHASE II.
	*
	*					I am removing the functions command_in() and command_out() because they are no
	*					longer needed.
	*
	*	03/28/2015		I have added a global flag for when data has been received. In this way, the
	*					data-collection task will be able to see when a new value has been loaded into the
	*					CAN1_MB0 mailbox.
*/

#include <asf/sam/components/can/sn65hvd234.h>
#include <asf/sam/drivers/can/can.h>

#include <stdio.h>
#include <string.h>
#include "board.h"
#include "sysclk.h"
#include "exceptions.h"
#include "pmc.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "pio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "global_var.h"

SemaphoreHandle_t	Can1_Mutex;

typedef struct {
	uint32_t ul_mb_idx;
	uint8_t uc_obj_type;  /**< Mailbox object type, one of the six different
	                         objects. */
	uint8_t uc_id_ver;    /**< 0 stands for standard frame, 1 stands for
	                         extended frame. */
	uint8_t uc_length;    /**< Received data length or transmitted data
	                         length. */
	uint8_t uc_tx_prio;   /**< Mailbox priority, no effect in receive mode. */
	uint32_t ul_status;   /**< Mailbox status register value. */
	uint32_t ul_id_msk;   /**< No effect in transmit mode. */
	uint32_t ul_id;       /**< Received frame ID or the frame ID to be
	                         transmitted. */
	uint32_t ul_fid;      /**< Family ID. */
	uint32_t ul_datal;
	uint32_t ul_datah;
} can_temp_t;

/*		CAN MUTEXES DEFINED HERE		*/



/*		CURRENT PRIORITY LEVELS			
	Note: ID and priority are two different things.
		  For the sake of simplicity, I am making them the same.

		COMS TO CDH COMMAND (IMMED)	0
		PAYLOAD COMMAND				1
		COMS TO CDH COMMAND (SCHED) 2
		EPS COMMAND					3
		COMS COMMND					4
		COMS REQUESTING DATA		5
		COMS RECEIVING DATA			6
		RECEIVING PAYLOAD DATA		7
		REQUEST PAYLOAD DATA		8
		REQUEST HOUSEKEEPING		20
		TRANMITTING HOUSEKEEPING	15
		LED TOGGLE (LOWEST + 1) =	11
*/

#define COMMAND_OUT					0X01010101
#define COMMAND_IN					0x11111111

#define HK_TRANSMIT					0x12345678
#define CAN_MSG_DUMMY_DATA          0xFFFFFFFF

#define DUMMY_COMMAND				0XFFFFFFFF
#define MSG_ACK						0xABABABAB

#define HK_RETURNED					0XF0F0F0F0
#define HK_REQUEST					0x0F0F0F0F

#define DATA_REQUEST				0x55555555
#define DATA_RETURNED				0x00000000

#define CAN1_MB0				10
#define CAN1_MB1				11
#define CAN1_MB2				12
#define CAN1_MB3				13
#define CAN1_MB4				14
#define CAN1_MB5				15
#define CAN1_MB6				16
#define CAN1_MB7				17

#define SUB0_ID0				20
#define SUB0_ID1				21
#define SUB0_ID2				22
#define SUB0_ID3				23
#define SUB0_ID4				24
#define SUB0_ID5				25

#define COMMAND_PRIO			10
#define HK_REQUEST_PRIO			20
#define DATA_PRIO				25

/* CAN frame max data length */
#define MAX_CAN_FRAME_DATA_LEN      8

/* CAN0 Transceiver */
sn65hvd234_ctrl_t can0_transceiver;

/* CAN1 Transceiver */
sn65hvd234_ctrl_t can1_transceiver;

/* CAN0 Transfer mailbox structure */
can_mb_conf_t can0_mailbox;

/* CAN1 Transfer mailbox structure */
can_mb_conf_t can1_mailbox;

can_temp_t temp_mailbox_C0;
can_temp_t temp_mailbox_C1;

/*	DATA RECEPTION FLAG			   */
uint8_t	drf;

//uint32_t data_reg[2];

/* Function Prototypes */

void CAN1_Handler(void);
void CAN0_Handler(void);
void decode_can_msg(can_mb_conf_t *p_mailbox, Can* controller);
void reset_mailbox_conf(can_mb_conf_t *p_mailbox);
void can_initialize(void);
uint32_t can_init_mailboxes(uint32_t x);
void save_can_object(can_mb_conf_t *original, can_temp_t *temp);
void restore_can_object(can_mb_conf_t *original, can_temp_t *temp);
uint32_t send_can_command(uint32_t low, uint32_t high, uint32_t ID, uint32_t PRIORITY);		// API Function.
uint32_t request_housekeeping(uint32_t ID);													// API Function.
void read_can_message(uint32_t mb_id);														// API Function.


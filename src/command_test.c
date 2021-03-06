/*
    Author: Keenan Burnett

	***********************************************************************
	*	FILE NAME:		command_test.c
	*
	*	PURPOSE:		
	*	This file will be used to test commands with our CAN API.
	*
	*	FILE REFERENCES:	stdio.h, FreeRTOS.h, task.h, partest.h, asf.h, can_func.h
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
	*	02/01/2015		Created
	*
	*	02/06/2015		I managed to get the CAN API function send_command working.
	*
	*	02/17/2015		I made some changes to send_command() in can_func.c with regards to
	*					how the ID of the mailboxes is being set.
	*
	*	08/07/2015		Added the high_command_generator() function which generates the upper four
	*					bytes which are required in our new CAN organizational structure.
	*
	*	DESCRIPTION:	
	*
	*	This file is being used to test our CAN commands API. This file is used to encapsulate a 
	*	test function called command_test(), which will create a task that will send out a can message
	*	from CAN0 MB7 to CAN1 MB0 with the use of a CAN API function I have written send_can_command().
	*
	*	It will then delay 15 clock cycles and send the message again.
	*	
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Atmel library includes. */
#include "asf.h"

/* Common demo includes. */
#include "partest.h"

/* CAN Function includes */
#include "can_func.h"

/* Priorities at which the tasks are created. */
#define Command_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )		// Lower the # means lower the priority

/* Values passed to the two tasks just to check the task parameter
functionality. */
#define COMMAND_PARAMETER			( 0xABCD )

/*-----------------------------------------------------------*/

/*
 * Functions Prototypes.
 */
static void prvCommandTask( void *pvParameters );
void command_loop(void);

/************************************************************************/
/*			TEST FUNCTION FOR COMMANDS TO THE STK600                    */
/************************************************************************/
void command_loop( void )
{
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( prvCommandTask,					/* The function that implements the task. */
					"ON", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
					( void * ) COMMAND_PARAMETER, 			/* The parameter passed to the task - just to check the functionality. */
					Command_TASK_PRIORITY, 			/* The priority assigned to the task. */
					NULL );								/* The task handle is not required, so NULL is passed. */
	return;
}
/*-----------------------------------------------------------*/

/************************************************************************/
/*				COMMAND TASK		                                    */
/*	The sole purpose of this task is to send a single CAN message over	*/
/*	and over in order to test the STK600's CAN reception.				*/
/************************************************************************/
static void prvCommandTask( void *pvParameters )
{
	configASSERT( ( ( unsigned long ) pvParameters ) == COMMAND_PARAMETER );
	TickType_t	xLastWakeTime;
	const TickType_t xTimeToWait = 10000;	//Number entered here corresponds to the number of ticks we should wait.
	/* As SysTick will be approx. 1kHz, Num = 1000 * 60 * 60 = 1 hour.*/
	
	uint32_t low, PRIORITY;
	uint8_t byte_four = 0;
	
	low = DUMMY_COMMAND;
	PRIORITY = COMMAND_PRIO;
	
	/* @non-terminating@ */	
	for( ;; )
	{
		send_can_command(low, byte_four, OBC_ID, EPS_ID, REQ_RESPONSE, PRIORITY);				// Request response from COMS.
		send_can_command(low, byte_four, OBC_ID, COMS_ID, REQ_RESPONSE, PRIORITY);				// Request response from EPS.
		send_can_command(low, byte_four, OBC_ID, PAY_ID, REQ_RESPONSE, PRIORITY);				// Request response from PAY.
		
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime, xTimeToWait);
	}
}
/*-----------------------------------------------------------*/


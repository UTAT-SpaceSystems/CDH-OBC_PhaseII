/*
Author: Keenan Burnett
***********************************************************************
* FILE NAME: error_handling.c
*
* PURPOSE:
* This file is to be used to house the functions related to error handling in the OBSW.
*
* FILE REFERENCES: error_handling.h
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
* OBSW = On-Board Software
* GSSW = Groundstation Software
* FDIR = Failure Detection, Isolation & Recovery
*
* For more information on error handling, FDIR, resolution sequences, see: FDIR.docx in the Space Systems repo.
*
* REQUIREMENTS/ FUNCTIONAL SPECIFICATION REFERENCES:
*
* DEVELOPMENT HISTORY:
* 11/26/2015		Created.
*
* 03/27/2016		Added function headers which were missing.
*
* DESCRIPTION:
* When tasks come across issues that either need to be resolved immediately, or
* should be made apparent to the FDIR task / ground users, a message needs to be sent to FDIR.
*
* This file contains two functions, an "assert" for high severity issues, in which the task
* cannot continue regular operation without the issue being resolved, and an "error-report"
* for low severity issues that should be dealt with but do not prevent that task from continuing
* on with regular operation.
*/
#include "error_handling.h"

// I'm going to need "from-isr" versions of these functions.

/************************************************************************/
/*	ERRORASSERT				                                            */
/* @Purpose: Stops currently running program until the error has been	*/
/* resolved be the FDIR task.											*/
/* @param: task: ID of the task using this function. ex: HK_TASK_ID		*/
/* @param: code: Extra piece of information which may be provided to	*/
/* FDIR depending on the issue. See the implementation in fdir.c to see	*/
/* what it is using code for.											*/
/* @param: error: The error code (error_handling.h)						*/
/* @param: *data: pointer to data that fdir.c needs for resolution.		*/
/*	Again, check the implementation in FDIR.c							*/
/* @param: mutex: If the currently running task currently holds the		*/
/* mutex to a specific resource, it should specify it here so that this */
/* function will release it and then reacquire it before returning.		*/
/* @return: -1 = The error was not resolved, 1 = all good.				*/
/* @Note: This is for high-severity errors only.						*/
/* @Note: This function shall halt the regular operation of this task	*/
/* for a maximum of 5 minutes.											*/
/* @Note: *data should point to an array of at least 147 bytes.			*/
/************************************************************************/
int errorASSERT(uint8_t task, uint8_t code, uint32_t error, uint8_t* data, SemaphoreHandle_t mutex)
{
	uint8_t i;
	TickType_t wait_time = 5 * 60 * 1000;
	uint32_t timeout = 5 * 60 * 1000;
	int ret_val = 0;
	for(i = 0; i < 147; i++)
	{
		high_error_array[i] = *(data + i);	// Load the data into the high_error_array.
	}
	high_error_array[151] = (uint8_t)((error & 0xFF000000) >> 24);
	high_error_array[150] = (uint8_t)((error & 0x00FF0000) >> 16);
	high_error_array[149] = (uint8_t)((error & 0x0000FF00) > 8);
	high_error_array[148] = (uint8_t)(error & 0x000000FF);
	high_error_array[147] = task;
	high_error_array[146] = code;

	if (xSemaphoreTake(Highsev_Mutex, wait_time) == pdTRUE)		// Attempt to acquire Mutex, block for max 5 minutes.
	{
		xQueueSendToBack(high_sev_to_fdir_fifo, high_error_array, wait_time);		// This should return pdTrue
	}
	xSemaphoreGive(Highsev_Mutex);
	// Release the currently acquired mutex lock if there is one.
	if(mutex)
	xSemaphoreGive(mutex);
	// Wait for the error to be resolved.
	switch(task)
	{
		case HK_TASK_ID:
			hk_fdir_signal = 1;
			while(hk_fdir_signal & timeout--){taskYIELD();}	// Wait until the problem is solved for a maximum of 5 minutes.
			ret_val = -1;
			if(!hk_fdir_signal)
				ret_val =  1;
		case TIME_TASK_ID:
			time_fdir_signal = 1;
			while(time_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!time_fdir_signal)
				ret_val = 1;
		case COMS_TASK_ID:
			coms_fdir_signal = 1;
			while(coms_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!coms_fdir_signal)
				ret_val =  1;
		case EPS_TASK_ID:
			eps_fdir_signal = 1;
			while(eps_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!eps_fdir_signal)
				ret_val = 1;
		case PAY_TASK_ID:
			pay_fdir_signal = 1;
			while(pay_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!pay_fdir_signal)
				ret_val = 1;
		case OBC_PACKET_ROUTER_ID:
			opr_fdir_signal = 1;
			while(opr_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!opr_fdir_signal)
				ret_val = 1;
		case SCHEDULING_TASK_ID:
			sched_fdir_signal = 1;
			while(sched_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!sched_fdir_signal)
				ret_val = 1;
		case WD_RESET_TASK_ID:
			wdt_fdir_signal = 1;
			while(wdt_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!wdt_fdir_signal)
				ret_val = 1;
		case MEMORY_TASK_ID:
			mem_fdir_signal = 1;
			while(mem_fdir_signal & timeout--){taskYIELD();}
			ret_val = -1;
			if(!mem_fdir_signal)
				ret_val = 1;
		default:
			ret_val = -1;
	}
	if(mutex)
		xSemaphoreTake(mutex, wait_time);
	return ret_val;
}

/************************************************************************/
/*	ERRORREPORT				                                            */
/* @Purpose: Stops currently running program until the error has been	*/
/* resolved be the FDIR task.											*/
/* @param: task: ID of the task using this function. ex: HK_TASK_ID		*/
/* @param: code: Extra piece of information which may be provided to	*/
/* FDIR depending on the issue. See the implementation in fdir.c to see	*/
/* what it is using code for.											*/
/* @param: error: The error code (error_handling.h)						*/
/* @param: *data: pointer to data that fdir.c needs for resolution.		*/
/*	Again, check the implementation in FDIR.c							*/
/* @return: -1 = FDIR messaging failed									*/
/* @Note: This is for low-severity errors only.							*/
/* @Note: This function does not halt regular operation, nor is the		*/
/* error fixed at this time.											*/
/* @Note: *data should point to an array of at least 147 bytes.			*/
/************************************************************************/
int errorREPORT(uint8_t task, uint8_t code, uint32_t error, uint8_t* data)
{
	uint8_t i;
	TickType_t wait_time = 5 * 60 * 1000;
	for(i = 0; i < 147; i++)
	{
		low_error_array[i] = *(data + i);	// Load the data into the high_error_array.
	}
	low_error_array[151] = (uint8_t)((error & 0xFF000000) >> 24);
	low_error_array[150] = (uint8_t)((error & 0x00FF0000) >> 16);
	low_error_array[149] = (uint8_t)((error & 0x0000FF00) > 8);
	low_error_array[148] = (uint8_t)(error & 0x000000FF);
	low_error_array[147] = task;
	low_error_array[146] = code;
	if (xSemaphoreTake(Lowsev_Mutex, wait_time) == pdTRUE)		// Attempt to acquire Mutex, block for max 5 minutes.
	{
		if( xQueueSendToBack(low_sev_to_fdir_fifo, low_error_array, wait_time) != pdTRUE)
		{
			xSemaphoreGive(Lowsev_Mutex);
			return -1;
		}
		xSemaphoreGive(Lowsev_Mutex);
		return 1;
	}
	else
	return -1;
}

/************************************************************************/
/* xQueueSendToBackTask                                                 */
/* wrapper function for xQueueSendToBack for the purpose of catching    */
/* any FIFO errors                                                      */
/* @Note: For use with FIFO to/from OPR									*/
/************************************************************************/
// direction: 1 = TO OPR, 0 = FROM OPR
BaseType_t xQueueSendToBackTask(uint8_t task, uint8_t direction, QueueHandle_t fifo, uint8_t *itemToQueue, TickType_t ticks)
{
	uint8_t attempts = 0, error = 0;
	switch(task)
	{
		case HK_TASK_ID:
		error = HK_FIFO_RW_ERROR;
		case SCHEDULING_TASK_ID:
		error = SCHED_FIFO_RW_ERROR;
		case TIME_TASK_ID:
		error = TM_FIFO_RW_ERROR;
		case MEMORY_TASK_ID:
		error = MEM_FIFO_RW_ERROR;
		case EPS_TASK_ID:
		error = EPS_FIFO_W_ERROR;
		default:
		return pdFAIL;
	}
	while (attempts < 3 && xQueueSendToBack(fifo, itemToQueue, ticks) != pdTRUE )
	{
		attempts++;
	}
	if (attempts == 3)
	errorREPORT(task, direction, error, itemToQueue);
	return pdPASS;
}

/************************************************************************/
// direction: 1 = TO OPR, 0 = FROM OPR
BaseType_t xQueueReceiveTask(uint8_t task, uint8_t direction, QueueHandle_t fifo, uint8_t *itemToQueue, TickType_t ticks)
{
	
	uint8_t attempts = 0, error = 0;
	switch(task)
	{
		case HK_TASK_ID:
			error = HK_FIFO_RW_ERROR;
			break;
		case SCHEDULING_TASK_ID:
			error = SCHED_FIFO_RW_ERROR;
			break;
		case TIME_TASK_ID:
			error = TM_FIFO_RW_ERROR;
			break;
		case MEMORY_TASK_ID:
			error = MEM_FIFO_RW_ERROR;
			break;
		default:
			return pdFALSE;
	}
	while (attempts < 3 && xQueueReceive(fifo, itemToQueue, ticks) != pdPASS )
	{
		attempts++;
	}
	if (attempts == 3)
	{
		errorREPORT(task, direction, error, itemToQueue);
		return pdFALSE;
	}
	return pdTRUE;
}
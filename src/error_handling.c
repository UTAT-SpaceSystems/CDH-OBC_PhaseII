/*
Author: Keenan Burnett
***********************************************************************
* FILE NAME: error_handling.c
*
* PURPOSE:
* This file is to be used to house the functions related to error handling in the OBSW.
*
* FILE REFERENCES: stdio.h, FreeRTOS.h, task.h, partest.h, asf.h, can_func.h
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
* REQUIREMENTS/ FUNCTIONAL SPECIFICATION REFERENCES:
*
* DEVELOPMENT HISTORY:
* 11/26/2015		Created.
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

// I'm going to need "from-isr" versions of these functions.

// For high-severity errors only.
// Note: This function shall halt the regular operation of this task for a maximum of 5 minutes.
// Should the error not be resolved after this time, -1 shall be returned and the respective task
// should know what to do about that.
// 1 is returned if the error was corrected
// If the task currently holds a mutex, this function will handle releasing it / reacquiring it. (-1 if it couldn't reacquire)
// *data should point to an array holding 147 bytes.
int errorASSERT(uint8_t task, uint32_t error, uint8_t* data, SemaphoreHandle_t mutex)
{
	uint8_t i;
	TickType_t wait_time = 5 * 60 * 1000;
	uint32_t timeout = 5 * 60 * 1000;
	for(i = 0; i < 147; i++)
	{
		high_error_array[i] = *(data + i);	// Load the data into the high_error_array.
	}
	high_error_array[151] = (uint8_t)((error & 0xFF000000) >> 24);
	high_error_array[150] = (uint8_t)((error & 0x00FF0000) >> 16);
	high_error_array[149] = (uint8_t)((error & 0x0000FF00) > 8);
	high_error_array[148] = (uint8_t)(error & 0x000000FF);
	high_error_array[147] = task;

	if (xSemaphoreTake(Highsev_Mutex, wait_time) == pdTRUE)		// Attempt to acquire Mutex, block for max 5 minutes.
	{
		xQueueSendToBack(high_sev_to_fdir_fifo, high_error_array, wait_time));		// This should return pdTrue
		// Wait for the error to be resolved.
		switch(task)
		{
			case HK_TASK_ID:
				hk_fdir_signal = 1;
				while(hk_fdir_signal & timeout--){taskYIELD;}	// Wait until the problem is solved for a maximum of 5 minutes.
				if(!hk_fdir_signal)
					return 1;
				return -1;
			case TIME_TASK_ID:
				time_fdir_signal = 1;
				while(time_fdir_signal & timeout--){taskYIELD;}
				if(!time_fdir_signal)
					return 1;
				return -1;
			case COMS_TASK_ID:
				coms_fdir_signal = 1;
				while(coms_fdir_signal & timeout--){taskYIELD;}
				if(!coms_fdir_signal)
					return 1;
				return -1;
			case EPS_TASK_ID:
				eps_fdir_signal = 1;
				while(eps_fdir_signal & timeout--){taskYIELD;}
				if(!eps_fdir_signal)
					return 1;
				return -1;
			case PAY_TASK_ID:
				pay_fdir_signal = 1;
				while(pay_fdir_signal & timeout--){taskYIELD;}
				if(!pay_fdir_signal)
					return 1;
				return -1;
			case OBC_PACKET_ROUTER_ID:
				opr_fdir_signal = 1;
				while(opr_fdir_signal & timeout--){taskYIELD;}
				if(!opr_fdir_signal)
					return 1;
				return -1;
			case SCHEDULING_TASK_ID:
				sched_fdir_signal = 1;
				while(sched_fdir_signal & timeout--){taskYIELD;}
				if(!sched_fdir_signal)
					return 1;
				return -1;
			case WD_RESET_TASK_ID;
				wdt_fdir_signal = 1;
				while(wdt_fdir_signal & timeout--){taskYIELD;}
				if(!wdt_fdir_signal)
					return 1;
				return -1;
			case MEMORY_TASK_ID:
				mem_fdir_signal = 1;
				while(mem_fdir_signal & timeout--){taskYIELD;}
				if(!mem_fdir_signal)
					return 1;
				return -1;
			default:
				return -1;
		}
		
		xSemaphoreGive(Highsev_Mutex);
	}
	else
		return -1;
}

// For low-severity errors only.
// Note: This function does not halt regular operation, nor is the error fixed at this time.
// The only reason this function would return -1 is if sending a message to the FDIR task failed.
// 1 should really be returned.
int errorREPORT(uint8_t task, uint32_t error, uint32_t* data)
{
	uint8_t i;
	TickType_t wait_time = 5 * 60 * 1000;
	for(i = 0; i < 147; i++)
	{
		high_error_array[i] = *(data + i);	// Load the data into the high_error_array.
	}
	high_error_array[151] = (uint8_t)((error & 0xFF000000) >> 24);
	high_error_array[150] = (uint8_t)((error & 0x00FF0000) >> 16);
	high_error_array[149] = (uint8_t)((error & 0x0000FF00) > 8);
	high_error_array[148] = (uint8_t)(error & 0x000000FF);
	high_error_array[147] = task;
	if (xSemaphoreTake(Lowsev_Mutex, wait_time) == pdTRUE)		// Attempt to acquire Mutex, block for max 5 minutes.
	{
		if( xQueueSendToBack(low_sev_to_fdir_fifo, high_error_array, wait_time)) != pdTRUE)
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
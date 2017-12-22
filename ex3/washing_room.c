/*
Author - Dean Levi 302326640
Project - Ex3
Using - washing_room.h, thread_handler.h, input_output_handler.h
Description - implementation of the function that starts the washing room.
			  also implements the function that frees allocated memory.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include "washing_room.h"
#include "thread_handler.h"
#include "input_output_handler.h"

/*
Parameters - argv - list of addresses from command line.
Returns - none.
Description - handling the washing room's operation.
*/
void StartWashingRoom(char *argv[]);

/*
Parameters - none.
Returns - none.
Description - frees allocated memory.
*/
void FreeMemory();


void StartWashingRoom(char *argv[]) {
	if (argv == NULL) {
		WriteToDebugAndExit("StartWashingRoom Received NULL pointer");
	}
	DWORD wait_code;
	DWORD NumberOfThreadsToWaitFor = WashingRoom.NumberOfRoommates + 1; // + 1 for washing robot
	WashingRoom.NumberOfClothesInBasket = 0; // first no clothes are in basket
	WashingRoom.ReportPointer = argv[2];
	WashingRoom.DebugLogPointer = argv[3];
	WashingRoom.WashingRoomReachedTD = false; // reseting before starting threads

	CreateThreadsSemaphoresAndMutex();

	wait_code = WaitForMultipleObjects(NumberOfThreadsToWaitFor, WashingRoom.p_thread_handles, TRUE, INFINITE);
	if (WAIT_OBJECT_0 != wait_code)	{
		WriteToDebugAndExit("Error when waiting for program to end.\n");
	}

	CloseThreadsSemaphoresAndMutex();
}

void FreeMemory() {
	free(WashingRoom.RoommatesPointer);
	free(WashingRoom.p_thread_handles);
	free(WashingRoom.p_thread_ids);
}
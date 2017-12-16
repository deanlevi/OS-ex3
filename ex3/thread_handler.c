/*
Author - Dean Levi 302326640
Project - Ex3
Using - thread_handler.h, washing_room.h, input_output_handler.h
Description - implemantation of the functions that create and close threads/semaphores/mutexes.
			  also implements the threads functions and logic of the washing room.
*/
#include "thread_handler.h"
#include "washing_room.h"
#include "input_output_handler.h"

/*
Parameters - none.
Returns - none.
Description - create all threads/semaphores/mutexes used in the program.
*/
void CreateThreadsSemaphoresAndMutex();

/*
Parameters - p_start_routine - a pointer to the function to be executed by the thread,
			 p_thread_id - a pointer to a variable that receives the thread identifier (output parameter),
			 p_thread_argument - the argument to send to thread's function.
Returns - if the function succeeds, the return value is a handle to the new thread, if not prints error to debug log and exits.
Description - creating the thread with the right parameters.
*/
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id);

/*
Parameters - lpParam - a pointer to the running roommate's index.
Returns - none.
Description - implementation of the roommate's operation logic.
*/
void WINAPI RoommateThread(LPVOID lpParam);

/*
Parameters - none.
Returns - none.
Description - signal full basket semaphore.
*/
void SignalFullBasket();

/*
Parameters - none.
Returns - none.
Description -  wait for empty basket semaphore signal.
*/
void WaitForEmptyBasket();

/*
Parameters - RoommateIndex - the current roommate's index.
Returns - none.
Description - implementation of the roommate's changing clothing operation.
			  WashingRoom.NumberOfClothesInBasket is an output parameter.
			  NumberOfClothesInCloset is an output parameter of the current roommate.
*/
void ChangeClothingAndFillBasket(int RoommateIndex);

/*
Parameters - none.
Returns - none.
Description - implementation of the washing robot's operation logic.
*/
void WINAPI WashingRobotThread();

/*
Parameters - none.
Returns - none.
Description - signal empty basket semaphore to each of the stuck out of clothes roommates.
*/
void ReleaseOutOfClothesRoommates();

/*
Parameters - none.
Returns - none.
Description - implementation of the washing robot's washing clothes operation.
			  WashingRoom.NumberOfClothesInBasket is an output parameter.
			  NumberOfClothesInCloset is an output parameter for all roommate's that changed clothes.
*/
void WashClothesAndReturnToRoommates();

/*
Parameters - Mutex - a mutex to wait for.
Returns - none.
Description - wait for mutex.
*/
void WaitForOneMutex(HANDLE Mutex);

/*
Parameters - Mutex - a mutex to release.
Returns - none.
Description - release mutex.
*/
void ReleaseOneMutex(HANDLE Mutex);

/*
Parameters - none.
Returns - none.
Description - close all handles to threads/semaphores/mutexes used in the program.
*/
void CloseThreadsSemaphoresAndMutex();


void CreateThreadsSemaphoresAndMutex() {
	/* Create the mutex that will be used to synchronize access to NumberOfClothesInBasket */
	WashingRoom.ChangeClothingMutex = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == WashingRoom.ChangeClothingMutex) {
		WriteToDebugAndExit("Error when creating ChangeClothingMutex.\n");
	}

	/* Create the mutex that will be used to synchronize access to NumberOfClothesInBasket */
	WashingRoom.WritingToFileMutex = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == WashingRoom.WritingToFileMutex) {
		WriteToDebugAndExit("Error when creating WritingToFileMutex.\n");
	}

	/* Create the semaphore that will be used to signal EmptyBasket indication */
	WashingRoom.EmptyBasket = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not signaled */
		1,		/* Maximum Count */
		NULL);	/* un-named */

	if (WashingRoom.EmptyBasket == NULL) {
		WriteToDebugAndExit("Error when creating EmptyBasket semaphore.\n");
	}

	/* Create the semaphore that will be used to signal FullBasket indication */
	WashingRoom.FullBasket = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not full */
		1,		/* Maximum Count */
		NULL);	/* un-named */

	if (WashingRoom.FullBasket == NULL) {
		WriteToDebugAndExit("Error when creating FullBasket semaphore.\n");
	}

	/* Create a thread for each roommate, give index as parameter to indicate which thread is running */
	int ThreadIndex = 0;
	for (; ThreadIndex < WashingRoom.NumberOfRoommates; ThreadIndex++)
	{
		WashingRoom.p_thread_handles[ThreadIndex] = CreateThreadSimple((LPTHREAD_START_ROUTINE)RoommateThread,
						&(WashingRoom.RoommatesPointer[ThreadIndex].RoommateIndex), &WashingRoom.p_thread_ids[ThreadIndex]);
		if (NULL == WashingRoom.p_thread_handles[ThreadIndex]) {
			WriteToDebugAndExit("Failed to create thread.\n");
		}
	}

	/* Create a thread for the washing robot */
	WashingRoom.p_thread_handles[WashingRoom.NumberOfRoommates] = CreateThreadSimple((LPTHREAD_START_ROUTINE)WashingRobotThread, NULL,
																				&WashingRoom.p_thread_ids[WashingRoom.NumberOfRoommates]);
	if (NULL == WashingRoom.p_thread_handles[WashingRoom.NumberOfRoommates]) {
		WriteToDebugAndExit("Failed to create thread.\n");
	}
}

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id) {
	HANDLE thread_handle;

	if (NULL == p_start_routine) {
		WriteToDebugAndExit("Error when creating a thread. Received null pointer.\n");
	}

	if (NULL == p_thread_id) {
		WriteToDebugAndExit("Error when creating a thread. Received null pointer.\n");
	}

	thread_handle = CreateThread(
		NULL,                /*  default security attributes */
		0,                   /*  use default stack size */
		p_start_routine,     /*  thread function */
		p_thread_parameters, /*  argument to thread function */
		0,                   /*  use default creation flags */
		p_thread_id);        /*  returns the thread identifier */

	if (NULL == thread_handle) {
		WriteToDebugAndExit("Couldn't create thread.\n");
	}

	return thread_handle;
}

void WINAPI RoommateThread(LPVOID lpParam) {
	if (NULL == lpParam) {
		WriteToDebugAndExit("Error in RoommateThread. Received null pointer.\n");
	}
	int *RoommateIndexPointer = (int*)lpParam;
	while (TRUE) {
		Sleep(WashingRoom.RoommatesPointer[*RoommateIndexPointer].Ti); // wait Ti untill changing clothing
		WashingRoom.RoommatesPointer[*RoommateIndexPointer].RunningTime += WashingRoom.RoommatesPointer[*RoommateIndexPointer].Ti;

		WaitForOneMutex(WashingRoom.WritingToFileMutex);
		RoommateActiveReport(*RoommateIndexPointer); // print to report log that roommate [index] finished waiting
		ReleaseOneMutex(WashingRoom.WritingToFileMutex);

		if (WashingRoom.RoommatesPointer[*RoommateIndexPointer].RunningTime >= WashingRoom.TD &&
																	!WashingRoom.WashingRoomReachedTD) { // check if need to end running
			WashingRoom.WashingRoomReachedTD = true;
			ReleaseOutOfClothesRoommates();
			SignalFullBasket(); // signal FullBasket so washing machine will finish running
			break;
		}

		if (WashingRoom.RoommatesPointer[*RoommateIndexPointer].NumberOfClothesInCloset == 0) { // need to wait for washing robot
			WashingRoom.RoommatesPointer[*RoommateIndexPointer].ClosetIsEmpty = true;
			if (WashingRoom.WashingRoomReachedTD) { break; } // covering corner case
			WaitForOneMutex(WashingRoom.EmptyBasket);
		}
		if (WashingRoom.WashingRoomReachedTD) {
			break; // finish running
		}

		WaitForOneMutex(WashingRoom.ChangeClothingMutex);

		//ChangeClothingAndFillBasket(*RoommateIndexPointer); // todo check it's okay to exchange only when having clothes and washing
															// maching is not full

		if (WashingRoom.NumberOfClothesInBasket == WashingRoom.M) { // if basket is full
			SignalFullBasket();
			WaitForEmptyBasket();
		}

		ChangeClothingAndFillBasket(*RoommateIndexPointer);

		ReleaseOneMutex(WashingRoom.ChangeClothingMutex);
	}
}

void SignalFullBasket() {
	BOOL release_res;

	release_res = ReleaseSemaphore( // signal FullBasket
		WashingRoom.FullBasket,
		1, 		/* Signal that basket is full */
		NULL);
	if (release_res == FALSE) {
		WriteToDebugAndExit("Error when releasing FullBasket semaphore.\n");
	}
}

void WaitForEmptyBasket() {
	DWORD wait_code;

	wait_code = WaitForSingleObject(WashingRoom.EmptyBasket, INFINITE); // wait for empty basket indication
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToDebugAndExit("Error when waiting for EmptyBasket semaphore.\n");
	}
}

void ChangeClothingAndFillBasket(int RoommateIndex) {
	WashingRoom.RoommatesPointer[RoommateIndex].NumberOfClothesInCloset -= 1; // use one clothing
	WashingRoom.NumberOfClothesInBasket += 1; // fill basket
}

void WINAPI WashingRobotThread() {
	DWORD wait_code;
	BOOL release_res;
	DWORD ret_val;
	while (TRUE) {
		wait_code = WaitForSingleObject(WashingRoom.FullBasket, INFINITE); // wait for basket to be full
		if (WAIT_OBJECT_0 != wait_code) {
			WriteToDebugAndExit("Error when waiting for FullBasket semaphore.\n");
		}

		if (WashingRoom.WashingRoomReachedTD) {
			break; // finish running
		}

		wait_code = WaitForSingleObject(WashingRoom.WritingToFileMutex, INFINITE); // wait for Mutex access
		if (WAIT_OBJECT_0 != wait_code) {
			WriteToDebugAndExit("Error when waiting for WritingToFileMutex.\n");
		}
		RobotActiveReport(); // print to report log that robot starts washing
		ret_val = ReleaseMutex(WashingRoom.WritingToFileMutex); // release mutex
		if (FALSE == ret_val) {
			WriteToDebugAndExit("Error when releasing WritingToFileMutex.\n");
		}

		WashClothesAndReturnToRoommates();

		// releasing roommate that started washing robot
		release_res = ReleaseSemaphore( // signal EmptyBasket
			WashingRoom.EmptyBasket,
			1, 		/* Signal that basket is empty */
			NULL);
		if (release_res == FALSE) {
			WriteToDebugAndExit("Error when releasing EmptyBasket semaphore.\n");
		}
		ReleaseOutOfClothesRoommates();
	}
}

void ReleaseOutOfClothesRoommates() {
	BOOL release_res;
	int RoommateIndex = 0; // releasing all waiting out of clothes roommates

	for (; RoommateIndex < WashingRoom.NumberOfRoommates; RoommateIndex++) {
		if (WashingRoom.RoommatesPointer[RoommateIndex].ClosetIsEmpty) {
			release_res = ReleaseSemaphore( // signal EmptyBasket
				WashingRoom.EmptyBasket,
				1, 		/* Signal that basket is empty */
				NULL);
			if (release_res == FALSE) {
				WriteToDebugAndExit("Error when releasing EmptyBasket semaphore.\n");
			}
			WashingRoom.RoommatesPointer[RoommateIndex].ClosetIsEmpty = false;
		}
	}
}

void WashClothesAndReturnToRoommates() {
	int RoommateIndex = 0;
	for (; RoommateIndex < WashingRoom.NumberOfRoommates; RoommateIndex++) {
		WashingRoom.RoommatesPointer[RoommateIndex].NumberOfClothesInCloset = // return clothes to closet
														WashingRoom.RoommatesPointer[RoommateIndex].Si - 1;
	}
	WashingRoom.NumberOfClothesInBasket = 0; // empty basket
}

void WaitForOneMutex(HANDLE Mutex) {
	DWORD wait_code;
	wait_code = WaitForSingleObject(Mutex, INFINITE); // wait for Mutex access
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToDebugAndExit("Error when waiting for Mutex.\n");
	}
}

void ReleaseOneMutex(HANDLE Mutex) {
	BOOL ret_val;
	ret_val = ReleaseMutex(Mutex); // release mutex
	if (FALSE == ret_val) {
		WriteToDebugAndExit("Error when releasing Mutex.\n");
	}
}

void CloseThreadsSemaphoresAndMutex() {
	DWORD ret_val;
	
	/* Close mutexes */
	ret_val = CloseHandle(WashingRoom.ChangeClothingMutex);
	if (FALSE == ret_val) {
		WriteToDebugAndExit("Error when closing ChangeClothingMutex.\n");
	}
	
	ret_val = CloseHandle(WashingRoom.WritingToFileMutex);
	if (FALSE == ret_val) {
		WriteToDebugAndExit("Error when closing WritingToFileMutex.\n");
	}

	/* Close semaphores */
	ret_val = CloseHandle(WashingRoom.EmptyBasket);
	if (FALSE == ret_val) {
		WriteToDebugAndExit("Error when closing EmptyBasket semaphore.\n");
	}

	ret_val = CloseHandle(WashingRoom.FullBasket);
	if (FALSE == ret_val) {
		WriteToDebugAndExit("Error when closing FullBasket semaphore.\n");
	}

	/* Close roommates and washing robot threads */
	int ThreadIndex = 0;
	for (; ThreadIndex <= WashingRoom.NumberOfRoommates; ThreadIndex++)
	{
		ret_val = CloseHandle(WashingRoom.p_thread_handles[ThreadIndex]);
		if (FALSE == ret_val) {
			WriteToDebugAndExit("Error when closing threads.\n");
		}
	}
}
/*
Author - Dean Levi 302326640
Project - Ex3
Using - none
Description - declaration of the function that starts the washing room and the function that frees allocated memory.
			  defining the RoommateProperties struct with needed/helpful parameters.
			  defining the WashingRoomDatabase struct with needed/helpful parameters.
			  creating the WashingRoom instance of WashingRoomDatabase as a general database variable with all needed information.
*/
#ifndef WASHING_ROOM_H
#define WASHING_ROOM_H

#include <Windows.h>
#include <stdbool.h>

#define ERROR_CODE ((int)(-1))
#define SUCCESS_CODE ((int)(0))

typedef struct _RoommateProperties {
DWORD Ti; // number of milliseconds till changing a clothing
DWORD Si; // number of clothes
DWORD NumberOfClothesInCloset;
int RoommateIndex;
bool ClosetIsEmpty;
}RoommateProperties;

typedef struct _WashingRoomDatabase {
	// General Information
	DWORD TD; // program running time in milliseconds
	DWORD M; // maximum number of clothes to wash in the washing robot
	RoommateProperties *RoommatesPointer; // array of roommates
	int NumberOfRoommates; // the number of roommates using the washing robot
	DWORD NumberOfClothesInBasket; // current number of clothes in the basket

	// Thread / Semaphore / Mutex Information
	HANDLE *p_thread_handles;
	DWORD *p_thread_ids;
	HANDLE EmptyBasket; // semaphore for empty basket indication
	HANDLE FullBasket; // semaphore for full basket indication
	HANDLE ChangeClothingMutex;
	HANDLE WritingToFileMutex; // for writing to report log

	// Debug Information
	char *DebugLogPointer; // needed for printing errors before exiting
	char *ReportPointer; // needed to update report log
}WashingRoomDatabase;

WashingRoomDatabase WashingRoom;

void StartWashingRoom(char *argv[]);
void FreeMemory();

#endif
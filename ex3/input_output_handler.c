/*
Author - Dean Levi 302326640
Project - Ex3
Using - washing_room.h, input_output_handler.h
Description - implementation of all functions related to parsing input and writing to files.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

#include "washing_room.h"
#include "input_output_handler.h"

/*
Parameters - argv - list of addresses from command line.
Returns - none.
Description - handling the parsing of the program's parameters from the parameters file.
			  also allocates needed memory.
*/
void ParseDatabase(char *argv[]);

/*
Parameters - argv - list of addresses from command line.
Returns - none.
Description - updates the number of roommates.
			  WashingRoom.NumberOfRoommates is the output parameter.
*/
void UpdateNumberOfRoomates(char *argv[]);

/*
Parameters - argv - list of addresses from command line.
Returns - none.
Description - updates the program's parameters.
			  WashingRoom.TD, WashingRoom.M are the output parameters of WashingRoom.
			  Ti, Si, RoommateIndex, ClosetIsEmpty are the output parameters of RoommatesPointer array for each roommate.
*/
void UpdateWashingRoomParameters(char *argv[]);

/*
Parameters - DebugLogAddress - the address of the debug log.
Returns - none.
Description - initializes the debug log file.
*/
void InitDebugLog(char *DebugLogAddress);

/*
Parameters - ErrorToPrint - the error that occured to print to the debug log.
Returns - none.
Description - in case of an error, prints ErrorToPrint to debug log file, frees memory and exits program.
*/
void WriteToDebugAndExit(char *ErrorToPrint);

/*
Parameters - ReportLogAddress - the address of the report log.
Returns - none.
Description - prints to the report log that simulation started.
*/
void SimulationStartedReport(char *ReportLogAddress);

/*
Parameters - RoommateIndex - the index of the roommate that is active to print to the report log.
Returns - none.
Description - prints to the report log that roommate RoommateIndex is active.
*/
void RoommateActiveReport(int RoommateIndex);

/*
Parameters - none.
Returns - none.
Description - prints to the report log that the washing robot is active.
*/
void RobotActiveReport();


void ParseDatabase(char *argv[]) {
	if (argv == NULL) {
		WriteToDebugAndExit("ParseDatabase Received NULL pointer");
	}
	UpdateNumberOfRoomates(argv);
	WashingRoom.RoommatesPointer = malloc(sizeof(RoommateProperties) * WashingRoom.NumberOfRoommates);
	if (WashingRoom.RoommatesPointer == NULL) {
		WriteToDebugAndExit("ParseDatabase failed to allocate RoommatesPointer.\n");
	}
	/* Create a thread for each roommate + one thread for washing robot */
	WashingRoom.p_thread_handles = malloc(sizeof(HANDLE) * WashingRoom.NumberOfRoommates + 1);
	if (WashingRoom.p_thread_handles == NULL) {
		WriteToDebugAndExit("ParseDatabase failed to allocate p_thread_handles.\n");
	}
	WashingRoom.p_thread_ids = malloc(sizeof(DWORD) * WashingRoom.NumberOfRoommates + 1);
	if (WashingRoom.p_thread_ids == NULL) {
		WriteToDebugAndExit("ParseDatabase failed to allocate p_thread_ids.\n");
	}
	UpdateWashingRoomParameters(argv);
}

void UpdateNumberOfRoomates(char *argv[]) {
	if (argv == NULL) {
		WriteToDebugAndExit("UpdateNumberOfRoomates Received NULL pointer");
	}
	FILE *ParametersPointer = NULL;
	ParametersPointer = fopen(argv[1], "r");
	if (ParametersPointer == NULL) {
		WriteToDebugAndExit("Couldn't open parameters file.\n");
	}
	char CurrentLine[MAX_LINE_LENGTH];
	int LineNumberCounter = 0;
	while (fgets(CurrentLine, MAX_LINE_LENGTH, ParametersPointer) != NULL) {
		LineNumberCounter++;
	}
	fclose(ParametersPointer);
	WashingRoom.NumberOfRoommates = LineNumberCounter - 2; // first two lines aren't roommates
}

void UpdateWashingRoomParameters(char *argv[]) {
	if (argv == NULL) {
		WriteToDebugAndExit("UpdateWashingRoomParameters Received NULL pointer");
	}
	FILE *ParametersPointer = NULL;
	ParametersPointer = fopen(argv[1], "r");
	if (ParametersPointer == NULL) {
		WriteToDebugAndExit("Couldn't open parameters file.\n");
	}
	char CurrentLine[MAX_LINE_LENGTH], CurrentArgument[MAX_LINE_LENGTH];
	int CurrentLineNumber = 0, CurrentRoommateIndex;
	while (fgets(CurrentLine, MAX_LINE_LENGTH, ParametersPointer) != NULL) {
		strcpy(CurrentArgument, strtok(CurrentLine, ","));
		switch (CurrentLineNumber) {
		case 0:
			WashingRoom.TD = atoi(CurrentArgument);
			CurrentLineNumber++;
			break;
		case 1:
			WashingRoom.M = atoi(CurrentArgument);
			CurrentLineNumber++;
			break;
		default:
			CurrentRoommateIndex = CurrentLineNumber - 2;
			WashingRoom.RoommatesPointer[CurrentRoommateIndex].Ti = atoi(CurrentArgument);
			strcpy(CurrentArgument, strtok(NULL, ","));
			WashingRoom.RoommatesPointer[CurrentRoommateIndex].Si = atoi(CurrentArgument);
			CurrentLineNumber++;
			WashingRoom.RoommatesPointer[CurrentRoommateIndex].NumberOfClothesInCloset =
									WashingRoom.RoommatesPointer[CurrentRoommateIndex].Si - 1; // Si - 1 in the closet, 1 on the roommate
			WashingRoom.RoommatesPointer[CurrentRoommateIndex].RoommateIndex = CurrentLineNumber - 3; // save roommate index
			WashingRoom.RoommatesPointer[CurrentRoommateIndex].ClosetIsEmpty = false; // at first closet is not empty
		}
	}
	fclose(ParametersPointer);
}

void InitDebugLog(char *DebugLogAddress) {
	if (DebugLogAddress == NULL) {
		WriteToDebugAndExit("InitDebugLog Received NULL pointer");
	}
	FILE *DebugLogPointer = NULL;
	DebugLogPointer = fopen(DebugLogAddress, "w"); // clearing file if exists
	if (DebugLogPointer == NULL) {
		WriteToDebugAndExit("Couldn't open debug log file.\n");
	}
	fclose(DebugLogPointer);
}

void WriteToDebugAndExit(char *ErrorToPrint) {
	FILE *DebugLogPointer = NULL;
	FreeMemory();
	DebugLogPointer = fopen(WashingRoom.DebugLogPointer, "w");
	if (DebugLogPointer == NULL) {
		printf("Couldn't open debug log file.\n");
		exit(ERROR_CODE);
	}
	fputs(ErrorToPrint, DebugLogPointer);
	fclose(DebugLogPointer);
	exit(ERROR_CODE);
}

void SimulationStartedReport(char *ReportLogAddress) {
	if (ReportLogAddress == NULL) {
		WriteToDebugAndExit("SimulationStartedReport Received NULL pointer");
	}
	FILE *ReportPointer = NULL;
	ReportPointer = fopen(ReportLogAddress, "w"); // clearing file if exists
	if (ReportPointer == NULL) {
		WriteToDebugAndExit("Couldn't open report file.\n");
	}
	fputs("Simulation Started\n", ReportPointer);
	fclose(ReportPointer);
}

void RoommateActiveReport(int RoommateIndex) {
	FILE *ReportPointer = NULL;
	ReportPointer = fopen(WashingRoom.ReportPointer, "a");
	if (ReportPointer == NULL) {
		WriteToDebugAndExit("Couldn't open report file.\n");
	}
	fprintf(ReportPointer, "Roommate %d Active\n", RoommateIndex);
	fclose(ReportPointer);
}

void RobotActiveReport() {
	FILE *ReportPointer = NULL;
	ReportPointer = fopen(WashingRoom.ReportPointer, "a");
	if (ReportPointer == NULL) {
		WriteToDebugAndExit("Couldn't open report file.\n");
	}
	fputs("Robot Active\n", ReportPointer);
	fclose(ReportPointer);
}
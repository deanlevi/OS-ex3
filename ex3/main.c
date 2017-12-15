/*
Author - Dean Levi 302326640
Project - Ex3
Using - washing_room.h, input_output_handler.h
Description - main file that initiates debug log, prints that simulation started, parses database and starts washing room.
*/
#include <stdio.h>

#include "washing_room.h"
#include "input_output_handler.h"

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Not the right amount of input arguments.\nNeed to give three.\nExiting...\n"); // first is path, other three are inputs
		return ERROR_CODE;
	}
	InitDebugLog(argv[3]); // sending debug log address
	SimulationStartedReport(argv[2]); // sending report log address
	ParseDatabase(argv);
	StartWashingRoom(argv);
	return SUCCESS_CODE;
}
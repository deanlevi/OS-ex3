/*
Author - Dean Levi 302326640
Project - Ex3
Using - none
Description - declaration of all functions related to parsing input and writing to files.
*/
#ifndef INPUT_OUTPUT_HANDLER_H
#define INPUT_OUTPUT_HANDLER_H

#define MAX_LINE_LENGTH 50

void ParseDatabase(char *argv[]);
void InitDebugLog(char *DebugLogAddress);
void WriteToDebugAndExit(char *ErrorToPrint);
void SimulationStartedReport(char *ReportLogAddress);
void RoommateActiveReport(int RoommateIndex);
void RobotActiveReport();

#endif
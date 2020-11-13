// A header file for helpers.c
// Declare any additional functions in this file

#include "linkedList.h"

void inputRedirection(char* input);
void outputRedirection(char *output);
void stderrRedirection(char *stderr);
void bothRedirection(char *both);
bgentry_t* matchPID (pid_t givenPID, List_t* list);
void printASCII(FILE *fptr);
void onePipe(char* cmnd, char* cmnd2, char** argv, char** argv2);
List_t* createList(int (*compare)(void*, void*));
int List_tComparator(void* lhs, void* rhs);
int getIndex (pid_t givenPID, List_t* list);
int getRearPID(List_t* list);
int checkPIDList(pid_t givenPID, List_t* list);
void morePipes(job_info* job, int pipeCount);
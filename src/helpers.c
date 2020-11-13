// Your helper functions need to be here.
#include "shell.h"
#include "linkedList.h"
#include "helpers.h"

void inputRedirection(char* input){
	int fd1 = open(input, O_RDONLY);
    if (fd1 < 0) {
        fprintf(stderr, "%s", RD_ERR);
		exit(EXIT_FAILURE);
    }
    dup2(fd1, STDIN_FILENO);
    close(fd1);
    return;
}

void outputRedirection(char *output) {
    int fd2 = open(output, O_CREAT | O_WRONLY, 0644);
    if (fd2 < 0) {
	   	fprintf(stderr, "%s", RD_ERR);
		exit(EXIT_FAILURE);
    }
    dup2(fd2, STDOUT_FILENO);
    close(fd2);
    return; 
}

void stderrRedirection(char *stderr1) {
    int fd3 = open(stderr1, O_CREAT | O_WRONLY, 0644);
    if (fd3 < 0) {
	   fprintf(stderr, "%s", RD_ERR);
		exit(EXIT_FAILURE);
    }
    dup2(fd3, STDERR_FILENO);
    close(fd3);
    return; 
}

void bothRedirection(char *both) {
    int fd4 = open(both, O_CREAT | O_WRONLY, 0644);
    if (fd4 < 0) {
	   	fprintf(stderr, "%s", RD_ERR);
		exit(EXIT_FAILURE);
    }
    dup2(fd4, STDOUT_FILENO);
    dup2(fd4, STDERR_FILENO);
    close(fd4);
    return; 
}

bgentry_t* matchPID(pid_t givenPID, List_t* list){

	node_t* head = list->head; 
    pid_t testPID = 0;

    if (list == NULL){
        return NULL;
    }

	while (head != NULL){

		bgentry_t* bge = (bgentry_t*)head->value;
		if (bge->pid == givenPID){
			return bge;
		}
        head = head->next;

	}

    return NULL;
}

void printASCII(FILE *fptr)
{
    char read_string[128];

    while(fgets(read_string,sizeof(read_string),fptr) != NULL){
        printf("%s",read_string);
    }

    printf("\n");
}

void onePipe(char* cmnd, char* cmnd2, char** argv, char** argv2){
    int process[2];
    int pid;
    int exec_result;

    pipe(process);
    pid = fork();

    if (pid == 0){
      dup2(process[1], 1);
      close(process[0]);
      execvp(cmnd, argv);
    }
    else{
      dup2(process[0], 0);
      close(process[1]);
      exec_result = execvp(cmnd2, argv2);
    }
}

void morePipes(job_info* job, int pipeCount){
    pipeCount -= 1;
    int status;
    pid_t pid;
    int piping = 2 * pipeCount;
    int process[piping];

    for(int i = 0; i < (pipeCount); i++){
        if(pipe(process + i * 2) < 0) {
            exit(EXIT_FAILURE);
        }
    }

    int k = 0;

    // For each command we have
    while(job->procs) {
        pid = fork();
        if(pid == 0) {
            if(k != 0 ){
                if(dup2(process[k - 2], 0) < 0){
                    exit(EXIT_FAILURE);
                }
            }
            
            if(job->procs->next_proc){
                if(dup2(process[k + 1], 1) < 0){
                    exit(EXIT_FAILURE);
                }
            }

            for(int i = 0; i < piping; i++){
                 close(process[i]);
            }

            if(execvp(job->procs->cmd, job->procs->argv) < 0 ){
                    exit(EXIT_FAILURE);
            }

        } 

        else if(pid < 0){
            exit(EXIT_FAILURE);
        }

        // Go to next command
        job->procs = job->procs->next_proc;
        k += 2;
    }

    for(int i = 0; i < piping; i++){
        close(process[i]);
    }

    for(int i = 0; i < pipeCount + 1; i++){
        wait(&status);
    }
}

List_t* createList(int (*compare)(void*, void*)) {
    List_t* list = (List_t*)malloc(sizeof(List_t));
    list->comparator = compare;
    list->length = 0;
    list->head = NULL;
    
    return list;
}

int List_tComparator(void* lhs, void* rhs){

    bgentry_t* listLeft = lhs;
    bgentry_t* listRight = rhs;

    pid_t pidLeft = listLeft->pid;
    pid_t pidRight = listRight->pid;

    if (pidLeft == pidRight){
        return 0;
    }

    if (pidLeft < pidRight){
        return -1;
    }

    else if (pidLeft > pidRight){
        return 1;
    }

}

int getIndex(pid_t givenPID, List_t* list){
    node_t* head = list->head;
    int index = 0;

    if (list == NULL){
        return -1;
    }

    while (head != NULL){
        bgentry_t* bge = (bgentry_t*)head->value;

        if (bge->pid == givenPID){
            return index;
        }

        head = head->next;
        index += 1;

    }
}

int getRearPID(List_t* list){
    node_t* head = list->head;
    bgentry_t* bge = (bgentry_t*)head->value;

    if (list == NULL){
        fprintf(stderr, "%s", PID_ERR);
        return -1;
    }

    while (head != NULL){
        bgentry_t* bge = (bgentry_t*)head->value;
        head = head->next;
        if (head == NULL){
            return bge->pid;
        }
    }
}

int checkPIDList(pid_t givenPID, List_t* list){

    node_t* head = list->head; 
    bgentry_t* bge = (bgentry_t*)head->value;

    if (list == NULL){
        return -1;
    }

    while (head != NULL){
        if (bge->pid == givenPID){
            return bge->pid;
        }
        head = head->next;

    }
    
    return -1;
}
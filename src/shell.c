#include "shell.h"
#include "helpers.h"
#include "linkedList.h"
#include <readline/readline.h>

// Global variables
int bgProcessesIndex = -1;
pid_t bgProcesses[1000];
volatile sig_atomic_t childTerm;
pid_t childPID = 0;

void signalHandler(int signo){
    pid_t PID;

    // Handle SIGUSR2
    if (signo == SIGUSR2){
        printf("Hi User! I am process %d\n", PID);
    }

    // Handle SIGCHLD
    else if (signo == SIGCHLD){
        int status;
        while (childPID = waitpid(-1, &status, WUNTRACED | WNOHANG) > 0){
        	sleep(1);
            if(WIFEXITED(status)){
				childTerm = 1;
			}    
        }

    }

    return;
}

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
#ifdef GS
    rl_outstream = fopen("/dev/null", "w");
#endif

    // List to store all current background processes
    List_t* list = createList(&List_tComparator);

	// Signal handlers
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR2, signalHandler) == SIG_ERR) {
		perror("Failed to handle SIGUSR2 signal");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGCHLD, signalHandler) == SIG_ERR) {
		perror(" Failed to handle SIGCHLD signal");
		exit(EXIT_FAILURE);
	}

	// If background process is terminated
	if (childTerm == 1){
			bgentry_t* bgToClear = matchPID(childPID, list);

			printf(BG_TERM, bgToClear->pid, bgToClear->job->line);
			int bgToClearIndex = getIndex(childPID, list);
			if (bgToClearIndex != -1){
				removeByIndex(list, bgToClearIndex);
			}
		
			bgProcessesIndex--;
			free(bgToClear->job->line);
			free_job(bgToClear->job);
			free(bgToClear);
			childTerm = 0;
	}

    // Print prompt and get command from user
	while ((line = readline(SHELL_PROMPT)) != NULL) {	

		job_info* job = validate_input(line);

        if (job == NULL) { // Command was empty string or invalid
			free(line);
			continue;
		}

		// example built-in: exit
		if (strcmp(job->procs->cmd, "exit") == 0) {
			// Terminating the shell

			// Terminate all bg process
			if (bgProcessesIndex > -1){
				while (bgProcessesIndex != -1){
					pid_t bgProcessID = bgProcesses[bgProcessesIndex];
					bgentry_t* bge = matchPID(bgProcessID, list);

					kill(bgProcessID, SIGKILL);
					printf(BG_TERM, bge->pid, bge->job->line);

					bgProcessesIndex--;
					free_job(bge->job);
					free(bge);
				}
			}

            deleteList(list);
            free(list);
			free(line);
			free_job(job);
            validate_input(NULL);
            return 0;
		}

		char* validatedCommand = job->procs->cmd;
		char current[100]; 
		char prev[100]; 
		int dirChangeStatus;

		if (strcmp(validatedCommand, "cd") == 0) {
		
			char* newDir = job->procs->argv[1];

			if (newDir == NULL){
				// Go to home environment
				newDir = getenv("HOME");
			}

			// Go to previous directory
			if (strcmp(newDir, "-") == 0){
				if (prev == NULL){
					fprintf(stderr, "%s", DIR_ERR);
					continue;
				}
				else {
					getcwd(current, 100);
					dirChangeStatus = chdir(prev);
					strcpy(prev, current);
				}
			}

			else {
				dirChangeStatus = chdir(newDir);
				strcpy(prev, current);	
			}
			

			if (dirChangeStatus == 0){
				fprintf(stdout, "%s\n", getcwd(current, 100));
			}

			else {
				fprintf(stderr, "%s", DIR_ERR);
			}

			continue;
		}

		if (strcmp(validatedCommand, "estatus") == 0) {

	    	if (WIFEXITED(exit_status)){ 
	    		// Get exit status of recently terminated process
	        	int childExitStatus = WEXITSTATUS(exit_status);         
	        	printf("%d\n", childExitStatus);
	        } 

	        continue;
        }

        if (strcmp(validatedCommand, "bglist") == 0) {
        	// Print list of background processess
	    	printList(list, STR_MODE);
	        continue;
        }

        if (strcmp(validatedCommand, "fg") == 0) {
        	pid_t bgPID;
        	int status;

	    	if (job->procs->argv[1] != NULL){
	    		// Get PID from command line
	    		bgPID = atoi(job->procs->argv[1]);
	    	}
	    	else {
	    		// Get PID from processes list
	    		if (list == NULL){
	    			fprintf(stderr, "%s", PID_ERR);
	    			continue;
	    		}

	    		bgPID = getRearPID(list);
	    	}

	    	// Check if provided pid is in the list
	    	int checkPID = checkPIDList(bgPID, list);

	    	if (checkPID != -1){
	    		kill(bgPID, SIGCONT);
	    		waitpid(bgPID, &status, WUNTRACED);
	    		continue;
	    	}

	    	else {
	    		// If not, print error and cont.
	    		fprintf(stderr, "%s", PID_ERR);
	    		continue;
	    	}
        }

        int backgroundFlag = job->bg;

        signal(SIGCHLD, signalHandler);

		int process[4];
		pipe(process);
		pipe(process+2);

		// example of good error handling!
		if ((pid = fork()) < 0) {
			exit(EXIT_FAILURE);
		}

		if (pid == 0) {  //If zero, then it's the child process
            //get the first command in the job list
		    proc_info* proc = job->procs;

			// File redirection
			char* inputFile = job->procs->in_file;
			char* outputFile = job->procs->out_file;
			char* stderrFile = job->procs->err_file;
			char* outerrFile = job->procs->outerr_file;
			
	        // Check if input file exists
	        if (inputFile != NULL){

	        	if (inputFile == outputFile || inputFile == stderrFile || inputFile == outerrFile){
	        		fprintf(stderr, "%s", RD_ERR);
					exit(EXIT_FAILURE);
	        	}

	        	inputRedirection(job->procs->in_file);
	        }

	        // Check if output file exists
	        if (outputFile != NULL){

	        	if (outputFile == inputFile || outputFile == stderrFile || outputFile == outerrFile){
	        		fprintf(stderr, "%s", RD_ERR);
					exit(EXIT_FAILURE);
	        	}

	        	outputRedirection(job->procs->out_file);
	        }

	        // Check if stderr file exists
	        if (stderrFile != NULL){

	        	if (stderrFile == inputFile || stderrFile == outputFile || stderrFile == outerrFile){
	        		fprintf(stderr, "%s", RD_ERR);
					exit(EXIT_FAILURE);
	        	}

				stderrRedirection(job->procs->err_file);
	        }

	        // Check if outerr file exists
	        if (outerrFile != NULL){

	        	if (outerrFile == inputFile || outerrFile == outputFile || outerrFile == stderrFile){
	        		fprintf(stderr, "%s", RD_ERR);
					exit(EXIT_FAILURE);
	        	}

				bothRedirection(job->procs->outerr_file);
	        }


	        // Piping
	        int pipeCount = job->nproc;

			if(pipeCount == 2){
				// One pipe operator
				struct proc_info *second_proc = proc->next_proc;
	        	char* command1 = proc->cmd;
	        	char* command2 = second_proc->cmd;
	        	char** argv1 = proc->argv;
	        	char** argv2 = second_proc->argv;
	        	onePipe(command1, command2, argv1, argv2);
			}

			else if (pipeCount > 2){
				// More than 1 pipe operators
				morePipes(job, pipeCount);
			}

			else if (pipeCount == 1){
				exec_result = execvp(proc->cmd, proc->argv);
				if (exec_result < 0) {  
					//Error checking
					printf(EXEC_ERR, proc->cmd);
					free_job(job);  
					free(line);
	    			validate_input(NULL);

					exit(EXIT_FAILURE);
				}
			}	

		} else {
            // Wait for the foreground job to finish
            if (backgroundFlag == 1){
            	bgentry_t* bgEntry = (bgentry_t*)malloc(sizeof(bgentry_t));
				
				bgEntry->job = job;
				bgEntry->seconds = clock();
				bgEntry->pid = pid;

				// Add process to list of background processes
				insertRear(list, (bgentry_t*)bgEntry);

            	bgProcessesIndex++;
				bgProcesses[bgProcessesIndex] = pid; 
            }
            else {
            	wait_result = waitpid(pid, &exit_status, 0);
				if (wait_result < 0) {
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
			}
            }
		}


		if (backgroundFlag == 0){
			// Free job if foreground process
			free_job(job);
		}
		free(line);
	}

    validate_input(NULL);

#ifndef GS
	fclose(rl_outstream);
#endif
	return 0;
}
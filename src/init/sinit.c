/* simple init system */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_NAME 100
#define MAX_PROCESSES 100
#define MAX_SIZE 1024
#define MAX_TRIES 5
#define CONFIG_FILE "sinit.conf"

#define MOTD "=== Simple Init ==="

#ifdef DEBUGGING
	#define DEBUG 1
#else
	#define DEBUG 0
#endif

struct Process {
	char name[MAX_NAME];
	pid_t pid;
	int restart;
	struct Process *next;
} head;

size_t process_count = 0;
struct Process processes[MAX_PROCESSES];

void panic(){
	fprintf(stderr, "[ERROR] A fatal error has occured! Dropping you into a shell... Good luck.\n");
	system("/bin/sh"); //TODO replace with exec and default shell
	printf("[INFO] Trying again after a fatal error.\n");
}

void *xmalloc(size_t size){
	int i = 0;
	void *ptr = NULL;
	do {
		for(i; i < MAX_TRIES; i++){
			ptr = malloc(size);
			if(ptr != NULL) return ptr;
			if(DEBUG) fprintf(stderr, "[DEBUG] Failed to allocate memory.\n");
			sleep(1);
		}
		fprintf(stderr, "[ERROR] FATAL - Unable to allocate memory after %d attempts!\n", MAX_TRIES);
		//exit(1);
		panic();
	} while(ptr == NULL);
}

void clear_process(const size_t count){
	processes[count].name[0] = '\0';
	processes[count].pid = 0;
	processes[count].restart = 0;
}

int restart(const char *program){
	strncpy(processes[process_count].name, program, MAX_NAME-1);
	processes[process_count].pid = getpid();
	processes[process_count].restart = 1;
	//size_t place = process_count++;
	size_t i = 0;
	do{
		if(DEBUG) fprintf(stderr, "[DEBUG] Starting process '%s' at PID %d\n", program, (int) getpid());
		system(program); //TODO maybe exec?
		char *argv[2];
		argv[0] = program;
		argv[1] = NULL;
		execv(program, argv);
		if(DEBUG) fprintf(stderr, "[DEBUG] Process '%s' at PID %d died.\n", program, (int) getpid());
		sleep(2);
		if(++i >= MAX_TRIES){
			fprintf(stderr, "[ERROR] Process '%s' at PID %d failed more than %d times! Stopping it.\n", program, (int)getpid(), MAX_TRIES);
			break;
		}
	} while(processes[process_count].restart);
	
	return 0;
}

int run(const char* program){
	// fork a process
	pid_t pid = fork();
	if(pid == 0){
		restart(program);
		exit(0);
	} else if(pid == -1){
		if(DEBUG) fprintf(stderr, "[DEBUG] Error forking '%s'\n", program);
		return -1;
	} else
		return 0;
}

char *parse(){

	FILE *fp = NULL;
	fp = fopen(CONFIG_FILE, "r");
	if(fp == NULL){
		fprintf(stderr, "[ERROR] FATAL - Config file '%s' not found!\n", CONFIG_FILE);
		panic();
		printf("[INFO] Attempting to load config file again...\n");
		return NULL;
	}
	char *programs = NULL;
	programs = (char*) xmalloc(MAX_SIZE * sizeof(char));
	char data = 0;
	size_t i = 0;
	for(data = (char)fgetc(fp); data != EOF; data = (char)fgetc(fp)){
		if(data == '\n') data = ' '; //TODO Handle CR
		programs[i++] = data;
	}
	fclose(fp);
	programs[i] = '\0';
	return programs;
}

int boot(){
	//mount root fs
	system("mount -o remount,rw /");
	//set hostname /etc/hostname
	
	//mount all drives in /etc/fstab
	//udev?

	//clean: /tmp /var/lock /var/run
	if(DEBUG)
		system("rm -rfi /tmp/* && rm -rfI /var/lock/* && rm -rfI /var/run/*");
	else
		system("rm -rf /tmp/* && rm -rf /var/lock/* && rm -rf /var/run/*");

	//start a login shell
	
}

int main(int argc, char *argv[]){
//TODO add logging
	int status = 0;
	size_t i = 0;
	puts(MOTD);

	if((int)getpid() != 1 && DEBUG) fprintf(stderr, "[DEBUG] Warning, I am not the init system. My PID is %d\n", (int)getpid());

	char *programs = NULL;
	do {
		programs = parse();
	} while(programs == NULL);

	char *program = NULL;
//TODO differentiate between needing to restart the program or not
	for(program = strtok(programs, " "); program != NULL; program = strtok(NULL, " "))
		run(program);  
	free(programs);
	while(1){
		sleep(2);
		pid_t childpid = wait(&status);
		if((int)childpid == -1) break; //if all programs are done running, time to stop?
		if(DEBUG) fprintf(stderr, "[DEBUG] Process PID %d exited with %d\n", (int) childpid, status);
		status = 0;
	}
	printf("[INFO] No more programs to run. Press enter to exit...");
	getchar();
	// check if we are root
	// parse init config file
	// start each process
	// add each process to the global table
	// check if a process dies, if so, restart it
	return 0; //probably should just loop
}

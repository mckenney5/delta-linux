/* simple init system */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

#include "sinit.config.h" // configuration file

char current_dir[PATH_MAX] = {'\0'}; //cwd
enum message_type{INFO, WARN, ERROR};

void msg(enum message_type msg_type, const char *info){
// handles messages to the user
/*  TODO
 * - make this like printf
 * - add colors (white for text, different colors for msg_type)
 */

	char type[6] = {'\0'};
	FILE *stream = stdout;

	switch(msg_type){
		case INFO:
			strcpy(type, "INFO");
			break;
		case WARN:
			strcpy(type, "WARN");
			break;
		case ERROR:
			strcpy(type, "ERROR");
			stream = stderr;
			break;
		default:
			strcpy(type, "???");
	}
	fprintf(stream, "[ %s ] %s", type, info);
}

void panic(){
// Allows users to attempt to fix their computer after fatal errors
// TODO make the program to run a var in the config
	msg(ERROR, "A fatal error has occured! Dropping you into a shell... Good luck.\n");
	system("/bin/sh"); //TODO replace with exec and default shell
	msg(INFO, "Trying again after a fatal error.\n");
}

void *xmalloc(size_t size){
// Allocates memory safely, will never return NULL
	unsigned long long i = 0;
	void *ptr = NULL;
	do {
		for(i; i < MAX_TRIES; i++){
			ptr = malloc(size);
			if(ptr != NULL) return ptr;
			if(DEBUG) fprintf(stderr, "[ DEBUG ] Failed to allocate memory.\n");
			usleep(USLEEP_TIME);
		}
		msg(ERROR, "FATAL - Unable to allocate memory after ");
		fprintf(stderr, "%llu out of %d  attempts!\n", i, MAX_TRIES);
		panic();
	} while(ptr == NULL);
}

void start(char *program){
// Handles the spawning of programs, this should never be called by the main thread
	printf("[ INFO ] Starting process '%s' at PID %d\n", program, (int) getpid());
	int returnnum = 0;
	char *argv[2];
	argv[0] = program;
	argv[1] = NULL;

	chdir(DAEMON_LOCATION);
	do {
		returnnum = execv(program, argv);
		if(returnnum == -1){
			msg(ERROR, "Failed to start program via execv, more info: ");
			perror(program);
			usleep(USLEEP_TIME);
		}
	} while(returnnum == -1);
	chdir(current_dir);

	printf("[ INFO ] Process '%s' at PID %d died.\n", program, (int) getpid());
}

int run(char* program){
// Handles forking of sinit
	msg(INFO, "Starting ");
	printf("%s\n", program);
	// fork a process
	pid_t pid = fork();
	if(pid == 0){
		start(program);
		exit(0);
	} else if(pid == -1){
		if(DEBUG) fprintf(stderr, "[ DEBUG ] Error forking '%s'\n", program);
		return -1;
	} else
		return 0;
}

char **get_daemons(const char* location){
// Gets and returns a list of programs in a directory (normally from DAEMON_LOCATION in the sinit.config.h file)
	char **programs = NULL;
	DIR *d;
	struct dirent *dir;
	d = opendir(location);
	size_t i = 0, l = 0, size = 10;
	if(!d){
		msg(ERROR, "Cannot read files.\n");
		perror("opendir");
		return NULL;
	}
	programs = (char**) xmalloc(sizeof(programs) * size);
	while ((dir = readdir(d)) != NULL) {
		if(dir->d_name[0] != '.'){ //ignore hidden files and . and ..
			programs[i] = xmalloc(sizeof(char) * strlen(dir->d_name) + 1);
			strcpy(programs[i], dir->d_name);
			if(++i >= size){
				programs = realloc(programs, sizeof(programs) * (size + 10)); //TODO test this
				size = size + 10;
			}
		}
	}
		closedir(d);
		programs[i] = NULL;
	return programs;
}

void boot(){
// Handles the boot process
// TODO on fail, return 1 and have main restart it

	//mount root fs
	msg(INFO, "Mounting root filesystem...");
	system("mount -o remount,rw /");
	getcwd(current_dir, sizeof(current_dir));
	puts("done.");
	//set hostname /etc/hostname
	
	//mount all drives in /etc/fstab
	//udev?

	//clean up files
	msg(INFO, "Cleaning up from last boot...");
	const char *command = "rm ";
	char args[6] = "-rf ";
	if(DEBUG) strcpy(args, "-rfI ");
	char *full_command = NULL;
	full_command = xmalloc((PATH_MAX + strlen(command) + strlen(args + 1)) * sizeof(char));
	full_command[0] = 0;
	size_t i=0;
	for(i = 0; CLEAN_ON_BOOT[i] != NULL; i++){
		strcpy(full_command, command);
		strncat(full_command, args, strlen(args));
		strncat(full_command, CLEAN_ON_BOOT[i], PATH_MAX);
		if(DEBUG) fprintf(stderr, "[ DEBUG ] Deleting files in '%s' with command '%s'\n", CLEAN_ON_BOOT[i], full_command);
		else printf("'%s',", CLEAN_ON_BOOT[i]);
		system(full_command);
	}
	free(full_command);

	puts("\b... done.");
	//start a login shell or getty?
}

void start_daemons(){
// Starts the programs from and frees the list of daemons from DAEMON_LOCATION
	char **programs = NULL;
	size_t i = 0;
	do {
		programs = get_daemons(DAEMON_LOCATION);
	} while(programs == NULL);

	for(i=0; programs[i] != NULL; i++){
		run(programs[i]);
		free(programs[i]);
	}
	free(programs[i]);
	free(programs);
}

void check_if_init(){
// checks if the program is the init and root
	if((int)getpid() != 1){
		msg(WARN, "I am not the init system (PID 1) My PID is ");
		printf("%d\n", (int)getpid());
	}
}

void watch_daemons(){
// Waits for daemons to stop and reports it
	int status = 0;
	while(1){
		usleep(USLEEP_TIME);
		pid_t childpid = wait(&status);
		if((int)childpid == -1) break; //if all programs are done running, time to stop?
		if(DEBUG) fprintf(stderr, "[ DEBUG ] Process PID %d exited with %d\n", (int) childpid, status);
		status = 0;
	}

}

int main(int argc, char *argv[]){
	// Displays the message of the day in sinit.config.h
	puts(MOTD);

	// Checks the status of this process
	check_if_init();

	// Starts the boot process
	boot();

	// Starts the daemons
	start_daemons();

	// Waits for the daemons to stop
	watch_daemons();

	// Once all the daemons are stopped, we are done
	msg(INFO, "No more programs to run. Press enter to exit...");
	getchar();
	return 0; //probably should just loop
}

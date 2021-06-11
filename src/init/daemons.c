/* handles daemons for the simple init system */

#include "daemons.h"
char current_dir[PATH_MAX] = {'\0'}; //cwd


void start(const char *location, char *program){
// Handles the spawning of programs, this should never be called by the main thread
	printf("[ INFO ] Starting process '%s' at PID %d\n", program, (int) getpid());
	int returnnum = 0;
	char *argv[2];
	argv[0] = program;
	argv[1] = NULL;
	chdir(location);
	do {
		returnnum = execv(program, argv);
		if(returnnum == -1){
			msg(ERR, "Failed to start program via execv, more info: ");
			perror(program);
			error_sleep();
		}
	} while(returnnum == -1);
	chdir(current_dir);

	printf("[ INFO ] Process '%s' at PID %d died.\n", program, (int) getpid());
}

int run(const char *location, char* program){
// Handles forking of sinit
	msg(INFO, "Starting ");
	printf("%s%s\n", location, program);
	// fork a process
	pid_t pid = fork();
	if(pid == 0){
		start(location, program);
		exit(0);
	} else if(pid == -1){
		fprintf(stderr, "[ ERROR ] Error forking '%s'\n", program);
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
		msg(ERR, "Cannot read files.\n");
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

void start_daemons(const char *location){
// Starts the programs from and frees the list of daemons from DAEMON_LOCATION
	char **programs = NULL;
	size_t i = 0;
	do {
		programs = get_daemons(location);
	} while(programs == NULL);

	for(i=0; programs[i] != NULL; i++){
		run(location, programs[i]);
		free(programs[i]);
	}
	free(programs[i]);
	free(programs);
}

void watch_daemons(){
// Waits for daemons to stop and reports it
	int status = 0;
	while(1){
		pid_t childpid = wait(&status);
		if((int)childpid == -1) break; //if all programs are done running, time to stop?
		printf("[ INFO ] Process PID %d exited with %d\n", (int) childpid, status);
		status = 0;
		error_sleep();
	}
}

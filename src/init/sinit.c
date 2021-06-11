/* simple init system */

#include "sinit.h"
#include "sinit.config.h"

void msg(enum message_type msg_type, const char *info){
// handles messages to the user
/*  TODO
 * - make this like printf
 * - add colors (white for text, different colors for msg_type)
 * - add logs
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
		case ERR:
			strcpy(type, "ERROR");
			stream = stderr;
			break;
		default:
			strcpy(type, "???");
	}
	fprintf(stream, "[ %s ] %s", type, info);
	fflush(stream);
}

void error_sleep(){
// sleeps after an error to stop spamming
	usleep(USLEEP_TIME);
}

void panic(){
// Allows users to attempt to fix their computer after fatal errors
// TODO make the program to run a var in the config
	msg(ERR, "A fatal error has occured! Dropping you into a shell... Good luck.\n");
	system(PANIC_PROGRAM); //TODO replace with exec and default shell
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
			error_sleep();
		}
		msg(ERR, "FATAL - Unable to allocate memory after ");
		fprintf(stderr, "%llu out of %d  attempts!\n", i, MAX_TRIES);
		panic();
	} while(ptr == NULL);
}

void set_hostname(){
// sets the hostname from HOSTNAME_FILE in the config header
	char hostname[HOST_NAME_MAX] = {'\0'};
	char data = 0;
	size_t i = 0;
	//open file / hostname location

	FILE *fp = NULL;
	fp = fopen(HOSTNAME_FILE, "r");
	if(fp == NULL){
		msg(ERR, "Failed to get hostname file, more info: ");
		perror("fopen");
	} else {
		for(data = (char)fgetc(fp); data != EOF && i < HOST_NAME_MAX; data = (char)fgetc(fp))
			hostname[i++] = data;
		fclose(fp);
		if(sethostname(hostname, strlen(hostname))){
			msg(ERR, "Failed to set hostname, more info: ");
			perror("sethostname");
		}	
	}
}

void clean_files(){
// deletes files defined in CLEAN_ON_BOOT in the config header
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
		printf("'%s', ", CLEAN_ON_BOOT[i]);
		system(full_command);
	}
	free(full_command);
}

void boot(){
// Handles the boot process
// TODO on fail, return 1 and have main restart it

	//mount root fs
	msg(INFO, "Mounting root filesystem...");
	system("mount -o remount,rw /");
	getcwd(current_dir, sizeof(current_dir));
	puts("done.");
	
	//set hostname
	msg(INFO, "Setting the hostname...");
	set_hostname();
	puts("done.");
	
	//mount all drives in /etc/fstab
	//udev?

	//clean up files
	msg(INFO, "Cleaning up from last boot...");
	clean_files();
	puts("\b\b... done.");
	//start a login shell or getty?

}

void check_if_init(){
// checks if the program is the init and root
	if((int)getpid() != 1){
		msg(WARN, "I am not the init system (PID 1), my PID is ");
		printf("%d\n", (int)getpid());
	}
	if(geteuid() != 0)
		msg(WARN, "I am not root\n");
}

int main(int argc, char *argv[]){
	// Displays the message of the day in sinit.config.h
	puts(MOTD);

	// Checks the status of this process
	check_if_init();

	// Starts the boot process
	boot();

	// Starts the daemons
	start_daemons(DAEMON_LOCATION);

	// Waits for the daemons to stop
	watch_daemons();

	// Once all the daemons are stopped, we are done
	msg(INFO, "No more programs to run. Press enter to exit...");
	getchar();
	return 0; //probably should just loop
}

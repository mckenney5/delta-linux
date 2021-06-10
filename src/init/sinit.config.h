/* simple init system's configuration */
#ifndef _USING_SINIT_CONFIG_H
#define _USING_SINIT_CONFIG_H

// Folder location of daemons
//#define DAEMON_LOCATION "/startup/"
#define DAEMON_LOCATION "/tmp/startup/"

// Largest name for a program
#define MAX_NAME 100

// Largest ammount of process to run
#define MAX_PROCESSES 100

// Max size of input
#define MAX_SIZE 1024

// Max tries to allocate memory AND to restart programs
#define MAX_TRIES 5

// Microsecond sleep time before a loop (changing this can make the program faster but also spam faster on errors)
#define USLEEP_TIME 100000

// Message of the day, displayed at boot
#define MOTD "=== Simple Init ==="

// Used for debugging/diagnostic messages  
#define DEBUG 1  // if you always want diagnostic messages on. Or
//#define DEBUG 0  // if you want debug messages off

// These are the folders that get wiped (rm -rf) at boot, make sure to use this format
// WATCH FOR TYPOS.
const char *CLEAN_ON_BOOT[MAX_NAME] = {"/tmp/*", "/var/lock/*", "/var/run/*", NULL};

#endif // end of SINITS configuration

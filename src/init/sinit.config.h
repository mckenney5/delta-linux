/* simple init system's configuration */
#ifndef _USING_SINIT_CONFIG_H
#define _USING_SINIT_CONFIG_H

// Largest name for a program
#define MAX_NAME 100

// Largest ammount of process to ru
#define MAX_PROCESSES 100

// Max size of input
#define MAX_SIZE 1024

// Max tries to allocate memory AND to restart programs
#define MAX_TRIES 5

// Location of the start up file (contains programs to be run at start up)
#define CONFIG_FILE "sinit.conf"

// Message of the day, displayed at boot
#define MOTD "=== Simple Init ==="

// Used for debugging messages if compiled with -DDEBUGGING in GCC
/* You can replace these lines with just: */
#define DEBUG 1  // if you always want diagnostic messages on. Or
//#define DEBUG 0  // if you want debug messages off

#endif // end of SINITS configuration

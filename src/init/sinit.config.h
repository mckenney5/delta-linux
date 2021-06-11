/* simple init system's configuration */
#ifndef _USING_SINIT_CONFIG_H
#define _USING_SINIT_CONFIG_H

// Folder location of daemons, it must exist!
//#define DAEMON_LOCATION "/startup/"
#define DAEMON_LOCATION "/tmp/startup/"

/*
// Location of log file
#define LOG_FILE "/var/log/init.log"
#define LOG 1 //set to 0 to turn off
*/

// Run this program when sinit runs into a fatal error via 'sh -c'
// *should* be a shell so you can fix what ever issue
#define PANIC_PROGRAM "sh"

// Max tries to allocate memory before panic
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
// PATH_MAX is the largest path you can use (see https://www.gnu.org/software/libc/manual/html_node/Limits-for-Files.html)
const char *CLEAN_ON_BOOT[PATH_MAX] = {"/tmp/*", "/var/lock/*", "/var/run/*", NULL};

#endif // end of SINITS configuration

/* handles daemons for the simple init system */
#ifndef _USING_DAEMONS_H
#define _USING_DAEMONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

#include "sinit.h"

extern char current_dir[PATH_MAX]; //cwd

void start(const char *location, char *program); // Handles the spawning of programs, this should never be called by the main thread

int run(const char *location, char* program); // Handles forking of sinit

char **get_daemons(const char* location); // Gets and returns a list of programs in a directory (normally from DAEMON_LOCATION in the sinit.config.h file)

void start_daemons(const char *location); // Starts the programs from and frees the list of daemons from DAEMON_LOCATION

void watch_daemons(); // Waits for daemons to stop and reports it

#endif

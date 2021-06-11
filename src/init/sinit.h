/* simple init system */
#ifndef _USING_SINIT_H
#define _USING_SINIT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

#include "daemons.h" // handles daemons

enum message_type{INFO, WARN, ERR};
void msg(enum message_type msg_type, const char *info); // handles messages to the user

void error_sleep(); // sleeps to prevent errors from spamming

void panic(); // Allows users to attempt to fix their computer after fatal errors

void *xmalloc(size_t size); // Allocates memory safely, will never return NULL

void boot(); // Handles the boot process

void check_if_init(); // checks if the program is the init and root

#endif

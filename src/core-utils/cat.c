/* Copyright 2024 (c) Adam McKenney - See LICENSE for details */
/* A way slimmer version of cat without the GNU bloat features that I have never used nor needed */
#include <stdlib.h> 	  //malloc
#include <err.h> 	  //err, errx
#include <sysexits.h> 	  //EX_OK, EX_OSERR, EX_IOERR, EX_NOINPUT
#include <unistd.h> 	  //read, write, close
#include <sys/types.h> 	  //stat
#include <sys/stat.h> 	  //stat
#include <sys/sendfile.h> //sendfile
#include <fcntl.h> 	  //open, STDIN_FILENO, STDOUT_FILENO

#define NAME "cat" //Program name, can be changed so a user can keep their bloated version of cat
#define HELP "Usage: " NAME " [-hu] [file...]\nWe concatenate files(s) to standard output with about 60 lines of code.\n\nWith no file, or when file is -, read standard input.\nExamples:\n  " NAME " -h     Displays the help\n  " NAME " -u     Tells the OS that we dont want a buffer (warning may be a lot slower with a large file).\n  " NAME " x - y  Output x's contents, then standard input, then y's contents.\n  " NAME "        Copy standard input to standard output."
#define BUFF_SIZE 10000 //a larger buffer trades memory for speed
#define MAX_SENDFILE_LINUX 2147479552 //max file size for sendfile in Linux

void cat(const int fd, const char no_buf, size_t size, const size_t files_pending){
/* reads from the file descriptor into buf, until zero bytes are read  */
	//tries sendfile if available (since it is faster, if not, do sys read & write)
	if(fd == STDIN_FILENO || size > MAX_SENDFILE_LINUX || sendfile(STDOUT_FILENO, fd, 0, size) != size){
		//if we cannot use sendfile, use the code below
		size_t r = 0;
		if(no_buf) size = 1;
		static void *buf = NULL;
		static size_t buf_size = 0;
		if(buf_size < size){
			buf = realloc(buf, size); //reuse buffer if we can, if buf starts as null then it will malloc automatically
			buf_size = size;
		}
		if(buf == NULL) err(EX_OSERR, "while allocating buf in cat()");
		
		while((r=read(fd, buf, size)) > 0)
			if(write(STDOUT_FILENO, buf, r) != r) err(EX_IOERR, "while writing to fd in cat(). %ld bytes were written", r); //if writing stops for some reason
		if(r == -1) err(EX_IOERR, "while reading fd in cat()."); //if read(2) throws an error
		if(files_pending == 1){
		 //allows us to reuse the buffer until we are at the last file
			free(buf);
			buf_size = 0;
		}
	} 
}
void get_file(const char* file_name, const char no_buf, const size_t files_pending){
/* tries to open the user's file, then calls cat() with its file pointer */
	if(file_name[0] == '-' && file_name[1] == '\0'){ cat(STDIN_FILENO, no_buf, BUFF_SIZE, files_pending); return;}
	struct stat buffer;
	int fd = -1, status = stat(file_name, &buffer);
	if(status != 0) err(EX_NOINPUT, "while checking file '%s' in get_file()", file_name);
	fd = open(file_name, O_RDONLY);
	if(fd == -1) err(EX_IOERR, "while opening file '%s' in get_file()", file_name); 
	cat(fd, no_buf, buffer.st_size, files_pending);
	close(fd);
}
int main(int argc, char *argv[]){
	size_t i = 0;
	char no_buf = 0; //-u option
	for(i=1; i <= argc; i++){ //step through all the vars, this is more readable than pointer arythmatic
		if(argv[1] == NULL)
			cat(STDIN_FILENO, no_buf, BUFF_SIZE, argc-i); 
		else if(argv[i][0] == '-' && argv[i][1] != '\0'){ //if its a command line arg
			if(argv[i][1] == 'h') errx(EX_OK, "\n%s\n", HELP);
			else if(argv[i][1] == 'u') no_buf = 1; //required by POSIX, turns off buffered output
			else errx(EX_USAGE, "while reading command line arg %ld in main(): unknown command line arg '%s'\n", i, argv[i]);
		} else {
			for( ; i < argc; i++) get_file(argv[i], no_buf, argc-i); //step through each arg as if it's a file, even if its a -
		}
	}
	return EX_OK;
}

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef WWPATH
#define WWPATH "/user/ww" //define WWPATH with directory of ww.c
#endif

int isDir(char *name){//check if the string is a directory
	struct stat data;
	int err = stat(name, &data);
	if(err == 1){
		perror(name);
		return -1;}
	if(S_ISDIR(data.st_mode)){
		write(2, "Directory Error\n", 16);
		return 1;}
	return 0;
}

int main (int argc, char **argv){
	//reformats text files, so that each line has a certain number, x, of characters per line
	//x will be determined by user
	//then the code concatenates all the reformatted text files
	int errorTest = 0;
	int width = atoi(argv[1]);
	int prevEmpty = 1;
	if(width < 0){
		write(2, "Invalid width\n", 14);
		return EXIT_FAILURE;}
	if(argc < 3){
		errorTest = 1;}
	for(int inc = 2; inc < argc; inc++){
		int checkInput = isDir(argv[inc]);
		if((checkInput == 1) || (checkInput == -1)){
			errorTest = 1;}
		else{int fd[2];
			pipe(fd);
			pid_t pidTest = fork();
			if(pidTest == -1){
				write(2, "Fork failed\n", 12);
				return EXIT_FAILURE;} 
			if(pidTest == 0){//execute ww with child process
				errorTest = close(fd[0]);
				if(errorTest != 0){
					write(2, "Close failed\n", 13);
					errorTest = 1;}
				dup2(fd[1], 1);
				errorTest = execl(WWPATH, WWPATH, argv[1], argv[inc], NULL);
				if(errorTest != 0){
					write(2, "Execl failed\n", 13);
					errorTest = 1;}}
			errorTest = close(fd[1]);
			if(errorTest != 0){
				write(2, "Close failed\n", 13);
				errorTest = 1;}
			char buf[100];
			int r;
			r = read(fd[0], buf, 100);
			if(prevEmpty == 0){
					write(1, "\n", 1);}
			if(r != 0){
				prevEmpty = 0;}
			else{prevEmpty = 1;}
			while (r > 0){
				write(1, &buf, r);
				r = read(fd[0], buf, 100);}
			errorTest = close(fd[0]);
			if(errorTest != 0){
				write(2, "Close failed\n", 13);
				errorTest = 1;}
			int wstatus;
			pidTest = wait(&wstatus);
			if(pidTest == -1){
				write(2, "Wait failed\n", 12);
				return EXIT_FAILURE;}
			if((WIFEXITED(wstatus) == 0) || (WEXITSTATUS(wstatus) != EXIT_SUCCESS)){
				write(2, "Child failed\n", 13);
				errorTest = 1;}}}
	if(errorTest == 0){
		return EXIT_SUCCESS;}
	else{return EXIT_FAILURE;}
}
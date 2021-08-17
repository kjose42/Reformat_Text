#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

int wrap(unsigned w, int inputFd, int outputFd){
	//reads from inputFd
	//changes inputFd's text to only have w amount of characters per line
	//then writes to outputFd
    	int return_code = EXIT_SUCCESS;
	int i = 0;
	//i keeps track of how many characters to write to output
	int lastspaceindex = 0;
	//lastspaceindex keeps track of the index of the space after the most recently discovered word
    	int lcount = 0; 
	//lcount keeps track of the index of the most recently discovered word
	int ncount = 0;
	//ncount keeps track of number of \n characters before the start of a word
	int scount = 0; 
	//scount keeps track of number of white-space characters before the start of a word
	int wcount = 0;
	//wcount keeps track of number of characters in a word
    	int size = 16352;
    	char* buf = (char *) malloc(size);
    	char c;
	while ( read(inputFd, &c, 1) > 0){
        	if ( (c == ' ' || c == '\t' || c == '\n' ) ){
		//if character is a white space, a word is found
			if (i > 0){
				if (c == '\n'){
					ncount++;
					c = ' ';}
            			if (scount==0){ 
                			buf[i]=' ';
                			if (wcount>w ){ 
                    				return_code = EXIT_FAILURE;
 	                    			if (lastspaceindex > 0)
                        				buf[lastspaceindex]='\n';
                    				buf[i] = '\n';
                    				lcount = 0; 
                			}
                			if (lcount>w) {
                   				if (lastspaceindex > 0)
                        				buf[lastspaceindex]='\n';
                    				lcount = wcount;
                			}
					if (lcount > 0)
						lcount++;
                			scount++; 
					wcount=0; 
                			lastspaceindex = i;
                			i++;}
			}
        	}
        	else{	if (ncount > 1 ){
				if (buf[i-1] == ' ')
					buf[i-1] = '\n';
				buf[i++] = '\n';
				ncount=0;
				lcount=0;
			}
            		buf[i++]=c;
            		scount = 0; ncount = 0; lcount++; wcount++;        
        	}
        if (i > size - 8 && 2*size - 1 < INT_MAX )
            size = 2*size; 
        char *p = realloc(buf, sizeof(int) * size);
        buf = p;
	} 
    if (wcount>w){ 
        return_code = EXIT_FAILURE;
        lcount = 0;
        if (lastspaceindex > 0)
            buf[lastspaceindex]='\n';
    }
    if (lcount>w) {
        if (lastspaceindex > 0)
            buf[lastspaceindex]='\n';
    }
	if (buf[i] != '\n' && buf[i-1] != '\n')
		buf[i++]='\n';
    buf[i]='\0';
	write(outputFd, buf, i);
    free(buf);
    return return_code;
}

int isdir(char *name){//checks if the string is a directory
	struct stat data;
	int err = stat(name, &data);
	if(err == 1){
		perror(name);
		return -1;}
	if(S_ISDIR(data.st_mode) == 1){
		return 1;}
	return 0;
}

int isreg(char *name){//checks if the string is a file
	struct stat data;
	int err = stat(name, &data);
	if(err == 1){
		perror(name);
		return -1;}
	if(S_ISREG(data.st_mode) == 1){
		return 1;}
	return 0;
}

int main (int argc, char **argv){
	//reformats text so that each line only has a certain number, x, of characters
	//x is determined by the user
	int inputFd;
	int width = atoi(argv[1]);
	assert(width > 0);
	int error = 0;
	if(argc == 2){
	//if a second argument isnt present, then the code reads standard input
	//then writes to standard output
		error = wrap(width, 0, 1);}
	else{	int checkInput = isdir(argv[2]);
		if(checkInput == -1){
			return EXIT_FAILURE;}
		if(checkInput == 1){
			//if second argument is a directory, then the code goes through each file in the directory
			//for each file, the code reads the text
				//then creates a text file and writes in it for output
			DIR *dirp = opendir(argv[2]);
			struct dirent *de;
			while((de = readdir(dirp))){
				if(strncmp(de->d_name, "wrap.", 5) != 0 && strncmp(de->d_name, ".", 1) != 0){
					if(de->d_type == 8){
						int dirLength = strlen(argv[2]);
						int wordLength = strlen(de->d_name);
						char *changeName1 = malloc (sizeof(char) * dirLength + wordLength + 2);
						char *changeName2 = malloc (sizeof(char) * dirLength + wordLength + 7);
						strcpy(changeName1, argv[2]);
						strcpy(changeName1+dirLength, "/");
						strcpy(changeName1+dirLength+1, de->d_name);
						strcpy(changeName2, argv[2]);
						strcpy(changeName2+dirLength, "/wrap.");
						strcpy(changeName2+dirLength+6, de->d_name);
						inputFd = open(changeName1, O_RDONLY, S_IRWXU);
						if(inputFd == -1){
							perror(changeName1);
							return EXIT_FAILURE;}
						int outputFd = open(changeName2, O_WRONLY|O_TRUNC|O_CREAT, S_IRWXU);
						if(outputFd == -1){
							perror(changeName2);
							return EXIT_FAILURE;}
						error = wrap(width, inputFd, outputFd);
						close(inputFd);
						close(outputFd);
						free(changeName1);
						free(changeName2);}}}
		closedir(dirp);}
		checkInput = isreg(argv[2]);
		if(checkInput == -1){
			return EXIT_FAILURE;}
		if(checkInput == 1){
			//if the second argument is a file, then the code reads from the file
			//then writes to standard output
			inputFd = open(argv[2], O_RDONLY);
			if(inputFd == -1){
				perror(argv[2]);
				return EXIT_FAILURE;}
			error = wrap(width, inputFd, 1);
			close(inputFd);}}
	if(error != 0){
		return EXIT_FAILURE;}
	return EXIT_SUCCESS;
}
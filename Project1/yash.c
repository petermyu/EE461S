
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
	
int hasPipe(char* string){

	int i;
	for(i = 0;string[i] != '\0';i++){
		if(string[i] == '|'){
			return 1;
		}
	}
	return 0;
}

void parse(char* buffer, char** args){
     while (*buffer != '\0') { 
          while (*buffer == ' ' || *buffer == '\t' || *buffer == '\n'){
               *buffer++ = '\0';
          }
          *args++ = buffer; 
          while (*buffer != '\0' && *buffer != ' ' && *buffer != '\t' && *buffer != '\n'){ 
               buffer++;
           }
     }
     *args = NULL;

}
int main(int argc, char* argv[]){
	
	int pipefd[2];
	pid_t pid;
	char buffer[2000];
	char *args[100];

	int input_len;
	char* str[100];

	fgets(buffer,2000,stdin);
	buffer[strlen(buffer)-1] = '\0';
	parse(buffer, args);
//	fprintf(stdout,"%s",&buffer);
	//pid = fork(); //test
	if(hasPipe(buffer) != 1){
		pid = fork();
	}

	else{


	}
//	fprintf(stdout,"%s",&str);	
	
	if(pid == 0){
		//child
		if(args != NULL){
			execvp(*args, args);
			//printf("could not find %s\n", str[0]); 

			printf("ran command %s\n",args[0]);
		}

	}

	return 0;
}


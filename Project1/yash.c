
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
	
bool hasPipe(char* string){

	int i;
	for(i = 0;string[i] != '\0';i++){
		if(string[i] = '|'){
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]){
	
	int pipefd[2];
	pid_t pid;
	char buffer[2000];
	char * path;
	
	fgets(buffer,2000,stdin);
//	fprintf(stdout,"%s",&buffer);
	buffer[strlen(buffer)-1] = '\0';

	if(hasPipe(buffer)){
		pid = fork();
	}
	else{
		int i = 0;
		int k = 0;
		int j =0 ;
		int input_len;
		char* str[100];
		char temp[200];
		while(buffer[k] != '\0'){
			while(buffer[k] != ' ' &&  buffer[k] != '\0'){
	//			strcat(str,temp);
				k++;
			}
			strncpy(temp,buffer+j, k-j);
			strcat(temp,"\0");
			str[i] = temp;
			i++;
			k++;
			j=k;
		}
	}
//	fprintf(stdout,"%s",&str);	
	
	if(pid == 0){
		//child


	}
	else{
		// parent
		char *test;
		test = getenv("PAGER");
		path = getenv(str[0]);
		if(path != NULL){
			if (execl(path, str[0], (char *) 0) == -1){
				printf("could not find %s\n", argv[0]); 
			}
		}
	}

	return 0;
}


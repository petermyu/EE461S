
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char* argv[]){
	
	int pipefd[2];
	pid_t pid;
	char buffer[2000];
	FILE *fp;
	char* input[3];
	fgets(buffer,2000,stdin);
//	fprintf(stdout,"%s",&buffer);
	buffer[strlen(buffer)-1] = '\0';
	pid = fork();
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
//	fprintf(stdout,"%s",&str);	
	
	input_len = k+1;
	if(pid == 0){
		//child
	
//		if (execl(strlen(input),input) == -1){
//			printf("could not find %s\n", argv[0]); 
//		}
//		else{}
	}
	else{
		// parent
		
		
}
return 0;
}


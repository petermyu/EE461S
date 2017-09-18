
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
int findRedirect(char** tokens, int* redirects){
	int i;
	int num = 0;
	for(i=0;tokens[i] != NULL;i++){
		if(*tokens[i] == '<' || *tokens[i] == '>' || (*tokens[i] == '2' && *tokens[i]+1 == '>')){
			num++;
			*redirects = i;
			redirects++;
		}
	}
	return num;
}
void doRedirect(char** tokens, int* redirects, int num){

	int i = 0;
	FILE *file;

	while(i < num){
		if ((file = fopen(tokens[redirects[i]+1], "rw")) == NULL){
			fprintf(stderr,"can't open %s", tokens[redirects[i]+1]);
		}
		if(*tokens[redirects[i]] == '<'){
		//	file = fopen(tokens[redirects[i]+1], "w");
			dup2(fileno(file),STDIN_FILENO);
		}
		else if(*tokens[redirects[i]] == '>'){
		//	file = fopen(tokens[redirects[i]+1], "w");
			dup2(fileno(file),STDOUT_FILENO);
		}
		else if(*tokens[redirects[i]] == '2' && *tokens[redirects[i]]+1 == '>'){
		//	file = fopen(tokens[redirects[i]+1], "rw");
			dup2(fileno(file),STDERR_FILENO);
		}
		i++;
	}
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
	while(1){
		int pipefd[2];
		pid_t pid;
		char buffer[2000];
		char *args[100];
		int redirects[6];
		int num_redirects;
		fgets(buffer,2000,stdin);
		buffer[strlen(buffer)-1] = '\0';
		parse(buffer, args);
		if((num_redirects = findRedirect(args,redirects)) > 0){
			doRedirect(args,redirects,num_redirects);
		}
		fprintf(stdout,"testing!!!!!!");
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
	}
	return 0;
}


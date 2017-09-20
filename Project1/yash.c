
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>
	

typedef struct{

	struct job *next;
	struct job *prev;
	pid_t jobpid;
	char **argv;
	int fg;//0 false, 1 true
	int status;
} job;

int childflag;
int numJobs = 0;
job *first = NULL;
job *last = NULL;

int pipefd[2];
pid_t pid, cpid1, cpid2;
int status;

static void sig_int(int signo) {
	if(cpid1 != 0){
	  printf("Sending signals to group:%d\n",cpid1); 
	  kill(-cpid1,SIGINT);
	}
}
static void sig_tstp(int signo) {
	if(cpid1 != 0){
	  printf("Sending SIGTSTP to group:%d\n",cpid1);
	  kill(-cpid1,SIGTSTP);
	}
}
static void sig_child(int signo){
 	pid_t wpid;
 	//printf("waiting for child change");

      	// Parent's wait processing is based on the sig_ex4.c
      	pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);

      	if (pid == -1) {
      		perror("waitpid");
      	}
      	if(WIFEXITED(status)) {
      	 	 printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));
      	} else if (WIFSIGNALED(status)) {
      	  	printf("child %d killed by signal %d\n", pid, WTERMSIG(status));
      	} else if (WIFSTOPPED(status)) {
      	 	 printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
      	} else if (WIFCONTINUED(status)) {
      		  printf("Continuing %d\n",pid);
    	}
   	}

int hasPipe(char** string){

	int i;
	for(i = 0;string[i] != '\0';i++){
		if(strcmp(string[i], "|") == 0){
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
		
		if(*tokens[redirects[i]] == '<'){
			if ((file = fopen(tokens[redirects[i]+1], "r")) == NULL){
				fprintf(stderr,"file does not exist %s", tokens[redirects[i]+1]);
			}
			else{
				dup2(fileno(file),STDIN_FILENO);
			}
		}
		else if(*tokens[redirects[i]] == '>'){
			file = fopen(tokens[redirects[i]+1], "w+");
			dup2(fileno(file),STDOUT_FILENO);
		}
		else if(*tokens[redirects[i]] == '2' && *tokens[redirects[i]]+1 == '>'){
			file = fopen(tokens[redirects[i]+1], "w+");
			dup2(fileno(file),STDERR_FILENO);
		}
		i++;
	}
}
void splitPipe(char** args, char** args2){
	int i = 0;
	int k = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],"|") == 0){
			args[i] = NULL;
			i++;
			break;
		}
		i++;
	}
	while(args[i] != '\0'){
		args2[k] = args[i];
		i++;
		k++;
	}
}
int parse(char* buffer, char** args){
	int x = 0;
	int background;
	if(buffer == NULL){
		return;
	}
	while(buffer[x] != '\0')
		x++;
	if(buffer[x-1] == '&'){
		background = 1; // cut off &
	}
	else{
		background = 0;
	}
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
     return background;

}
void handleExec(char **args, int *redirects){
	int num_redirects;
	int z = 0;
	while(args[z] != NULL)
		z++;
	if(strcmp(args[z-1],"&") == 0){
		args[z-1] = NULL;
	}
	if(args != NULL){
		char *cut[100];
		int i =0;
		if((num_redirects = findRedirect(args,redirects)) > 0){
			doRedirect(args,redirects,num_redirects);
			while(i<redirects[0]){
				cut[i] = args[i];
				i++;
			}
			cut[i] = NULL;
			execvp(*cut, cut);
		}
		else{
		//	printf("exec %s (child 2)\n",*args2);
			execvp(*args,args);
		}	
		exit(1);
	}
}
void handleJob(char **argv, int fg, int status, pid_t pid){
	job newJob;
	job temp;
	numJobs++;
	int i= 0;
	if(first == NULL){
		first = &newJob;
		last = &newJob;
	}
	else{
		last->next = &newJob;
		newJob.prev = last;
		last = &newJob;
	}

	newJob.argv = argv;
	newJob.fg = fg;
	newJob.jobpid = pid;

}
int isJobs(char buffer[],char ** args){
	job *temp;
	char ground;
	char *state;
	temp = first;
	if(args[0] == NULL){
		return;
	}
	if(strcmp(args[0],"jobs") == 0){
		int x = 0;
		while(x<numJobs){
			
			if(temp->fg == 1){
				ground = '+';
			}
			else{
				ground ='-';
			}

			if(temp->status == 1){
				state = "Running";
			}
			else{
				state = "Stopped";
			}
			printf("[%d] %c %s %s",x,ground,buffer);
			temp = temp->next;
		}
		return 1;
	}
	else{
		return 0;
	}
}
void initSignalHandler(){

	if (signal(SIGINT, sig_int) == SIG_ERR)
		printf("signal(SIGINT) error");
	if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
		printf("signal(SIGTSTP) error");
	 if(signal(SIGCHLD, sig_child) == SIG_ERR)
	 	printf("signal(SIGCHILD) error");

}
void handleSignal(int n){

}
void handleSIGCHILD(){

}
int main(int argc, char* argv[]){

		//initShell();
		initSignalHandler();
		char buffer[2001];

		printf("# ");
		while(fgets(buffer,2001,stdin) != NULL){

		char *args[1000];
		char *args2[1000];
		int redirects[6];
		int redirects2[6];
		int num_redirects;
		int background;
		buffer[strlen(buffer)-1] = '\0';
		pipe(pipefd);

		background = parse(buffer, args);
		if(isJobs(buffer,args) == 1){// checks if jobs
		
		}
		else{
		//ONLY ONE COMMAND, NO PIPE
		if(hasPipe(args) == 0){
			
			cpid1 = fork();
			if(cpid1 == 0){
				//child
				handleExec(args,redirects);
			}
			else{
				//parent
				handleJob(args,background,0,cpid1);
				handleSignal(1);
			}

		}
		//PIPE EXISTS, SO HANDLE
		else{
			splitPipe(args,args2);
			cpid1 = fork();
			if(cpid1 > 0){
				//parent
				cpid2 = fork();
				if(cpid2 > 0){
					close(pipefd[0]);
					close(pipefd[1]);
					handleSignal(2);
					handleJob(args,background,0,cpid1);
				}
				else{
				//child2
					sleep(1);
				//	printf("Child2 pid = %d\n",cpid2);
					setpgid(0,cpid1);
					close(pipefd[1]);
					dup2(pipefd[0],STDIN_FILENO);
					handleExec(args2,redirects2);
				}
			}
			else{
				//child 1
				setsid();
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
				handleExec(args,redirects);
			}
		}
		printf("# ");
	}
	}
	return 0;
}


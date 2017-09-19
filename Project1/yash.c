
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>
	

typedef struct job {

	struct *job next;
	pid_t jpid;
	char *argv;
	int fg;//0 false, 1 true
	int status;
} job;

job *first = NULL:
job *last = NULL;
int pipefd[2];
pid_t pid, cpid1, cpid2;
int status;

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

static void sig_int(int signo) {
  printf("Sending signals to group:%d\n",cpid1); 
  kill(-cpid1,SIGINT);
}
static void sig_tstp(int signo) {
  printf("Sending SIGTSTP to group:%d\n",cpid1);
  kill(-cpid1,SIGTSTP);
}

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
void handleExec(char **args, int *redirects){
	int num_redirects;
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
void handleJob(job newJob, char *argv, int fg, int status, pid_t pid){
	if(*first == NULL){
		*first == *newJob;
	}
	else{
		//something
	}
	newJob->argv = argv;
	newJob->fg = fg;
	newJob->jpid = pid;

}
void handleSignal(int n){

	int count=0;
	if (signal(SIGINT, sig_int) == SIG_ERR)
		printf("signal(SIGINT) error");
	if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
		printf("signal(SIGTSTP) error");

	while (count < n) {
      	// Parent's wait processing is based on the sig_ex4.c
      	pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
      	if (pid == -1) {
      	  perror("waitpid");
      	}
      	
      	if (WIFEXITED(status)) {
      	  printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));count++;
      	} else if (WIFSIGNALED(status)) {
      	  printf("child %d killed by signal %d\n", pid, WTERMSIG(status));count++;
      	} else if (WIFSTOPPED(status)) {
      	  printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
      	} else if (WIFCONTINUED(status)) {
      	  printf("Continuing %d\n",pid);
      	}
      }
}
int main(int argc, char* argv[]){

		initShell();

		char buffer[2001];

		printf("# ");
		while(fgets(buffer,2001,stdin) != NULL){
		job newJob;
		char *args[1000];
		char *args2[1000];
		int redirects[6];
		int redirects2[6];
		int num_redirects;
		// if(fgets(buffer,2001,stdin) == NULL){
		// 	printf("\n");
		// 	exit(1);
		// }

		buffer[strlen(buffer)-1] = '\0';
		pipe(pipefd);
	//	fprintf(stdout,"testing!!!!!!");
		//pid = fork(); //test
		//printf("initial cpid1 %d\n",cpid1);

			//ONLY ONE COMMAND, NO PIPE
		if(hasPipe(buffer) == 0){
			cpid1 = fork();
			if(cpid1 == 0){
				//child
				parse(buffer, args);
				handleExec(args,redirects);
			}
			else{
				//parent
				handleJob(newJob,args,1,0,cpid1);
				handleSignal(1);
			}

		}
		//PIPE EXISTS, SO HANDLE
		else{
			parse(buffer, args);
			splitPipe(args,args2);
			cpid1 = fork();
			if(cpid1 > 0){
				//parent
				cpid2 = fork();
				if(cpid2 > 0){
					close(pipefd[0]);
					close(pipefd[1]);
					handleSignal(2);
					handleJob(newJob,args,1,0,cpid1);
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
	return 0;
}


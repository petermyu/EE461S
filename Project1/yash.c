
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
	

typedef struct process
{
  struct process *next;
  char **argv;
  pid_t pid;
  char completed;
  char stopped;
  int status;
} process;


// void initProcess(process *p, pid_t pgid,
//                 int infile, int outfile, int errfile,
//                 int foreground){
// 	pid_t pid;

// 	if (shell_is_interactive){
// 	/* Put the process into the process group and give the process group
// 	 the terminal, if appropriate.
// 	 This has to be done both by the shell and in the individual
// 	 child processes because of potential race conditions.  */
// 	pid = getpid();
// 	if (pgid == 0) pgid = pid;
// 		setpgid (pid, pgid);
// 	if (foreground)
// 		tcsetpgrp (shell_terminal, pgid);

// 	/* Set the handling for job control signals back to the default.  */
// 	signal (SIGINT, SIG_DFL);
// 	signal (SIGQUIT, SIG_DFL);
// 	signal (SIGTSTP, SIG_DFL);
// 	signal (SIGTTIN, SIG_DFL);
// 	signal (SIGTTOU, SIG_DFL);
// 	signal (SIGCHLD, SIG_DFL);
// 	/* Set the standard input/output channels of the new process.  */
// 	if (infile != STDIN_FILENO){
// 		dup2 (infile, STDIN_FILENO);
// 		close (infile);
// 	}
// 	if (outfile != STDOUT_FILENO){
// 		dup2 (outfile, STDOUT_FILENO);
// 		close (outfile);
// 	}
// 	if (errfile != STDERR_FILENO){
// 		dup2 (errfile, STDERR_FILENO);
// 		close (errfile);
// 	}

// 	/* Exec the new process.  Make sure we exit.  */
// 	execvp (p->argv[0], p->argv);
// 	perror ("execvp");
// 	exit (1);
// }

//     }
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
	}
}
int main(int argc, char* argv[]){
		int pipefd[2];
		pid_t pid, cpid1, cpid2;
		char buffer[2001];
		char *args[1000];
		char *args2[1000];
		int redirects[6];
		int redirects2[6];
		int num_redirects;
		fgets(buffer,2001,stdin);
		buffer[strlen(buffer)-1] = '\0';

 		pipe(pipefd);
	//	fprintf(stdout,"testing!!!!!!");
		//pid = fork(); //test
		//printf("initial cpid1 %d\n",cpid1);

 		//ONLY ONE COMMAND, NO PIPE
		if(hasPipe(buffer) == 0){
			parse(buffer, args);
			printf("no pipe");
			if(args != NULL){
						if(num_redirects > 0){
							char *cut[100];
							int i =0;
							if((num_redirects = findRedirect(args,redirects)) > 0){
								doRedirect(args,redirects,num_redirects);
							}
							while(i<redirects[0]){
								cut[i] = args[i];
								i++;
							}
							printf("exec redirected");
							cut[i] = NULL;
							execvp(*cut, cut);
						}
						else{
							printf("exec %s (child1)\n",*args);
							execvp(*args,args);
						}
				}
		}
		//PIPE EXISTS, SO HANDLE
		else{
			parse(buffer, args);
			splitPipe(args,args2);
			cpid1 = fork();
			if(cpid1 > 0){
				//parent
				printf("Child1 pid = %d\n",cpid1);
				if(hasPipe(buffer) == 1){
					printf("has pipe so forked again");
					cpid2 = fork();
				}
				if(cpid2 > 0){
					close(pipefd[0]);
					close(pipefd[1]);
				}
				else{
				//child2
					sleep(1);
					printf("Child2 pid = %d\n",cpid2);
					setpgid(0,cpid1);
					close(pipefd[1]);
					dup2(pipefd[0],STDIN_FILENO);
					handleExec(args2,redirects2);
					// if(args2 != NULL){
					// 	if(num_redirects > 0){
					// 		char *cut[100];
					// 		int i =0;
					// 		if((num_redirects = findRedirect(args2,redirects2)) > 0){
					// 			doRedirect(args2,redirects2,num_redirects);
					// 		}
					// 		while(i<redirects[0]){
					// 			cut[i] = args2[i];
					// 			i++;
					// 		}
					// 		cut[i] = NULL;
					// 		execvp(*cut, cut);
					// 	}
					// 	else{
					// 	//	printf("exec %s (child 2)\n",*args2);
					// 		execvp(*args2,args2);
					// 	}	
					// }
				}
			}
			else{
				//child 1
				setsid();
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
				handleExec(args,redirects);
				// if(args != NULL){
				// 	if(num_redirects > 0){
				// 		char *cut[100];
				// 		int i =0;
				// 		if((num_redirects = findRedirect(args,redirects)) > 0){
				// 			doRedirect(args,redirects,num_redirects);
				// 		}
				// 		while(i<redirects[0]){
				// 			cut[i] = args[i];
				// 			i++;
				// 		}
				// 		//strcat(cut[i],"\0"); 
				// 		cut[i] = NULL;
				// 		execvp(*cut, cut);
				// 	}
				// 	else{
				// 	//	printf("exec %s (child1)\n",*args);
				// 		execvp(*args,args);
				// 	}
				// }
			}
		}
	return 0;
}


/*
Logan Lasiter - cssc0049
Professor: Carroll
Class: CS570
Due Date: 11/29/17

File Usage: parses through user input on the command line and performs basic shell functions
*/

#include "p2.h"

char *args[STORAGE]; // 2D array to hold user input
char *tmpArgs[STORAGE];
char word[STORAGE];
int count = 0;
int readFrom = 0; // flag for incoming redirection '<'
int writeTo = 0; // flag for outgoing redirection '>'
int bg = 0; // flag for background process '&'
int overWrite = 0; // flag for overwriting a file '>!'
int pipeCount = 0;
int idx = 0;
int execFlag = 0;

// redirection/pipe storage arrays
char inWord[256];
char outWord[256];
char readWord[256];
char writeWord[256];
char overWriteWord[256];
int inpfd, outpfd, ifd, ofd, owfd, devnull; // file descriptors for in/out

// performs redirection of input and output
void redirect() {
	if(overWrite == 2) {
		overWriteWord[0] = '\0';
		dup2(owfd, STDOUT_FILENO);
		close(owfd);
	}
	if(readFrom == 2) {
		inWord[0] = '\0';
		dup2(ifd, STDIN_FILENO);
		close(ifd);
	}
	if(writeTo == 2) {
		outWord[0] = '\0';
		dup2(ofd, STDOUT_FILENO);
		close(ofd);
	}
	if(bg != 0 && readFrom == 0) {
		dup2(devnull, 0);
		close(devnull);
	}
}

// Gets the desired command from the argument array created in parse and stores it in a temporary argument array for execution
void getCommand(int commandNum) {
	int i;
	int j = 0;
	int k = 0;
	for(i=0; i<commandNum; i++){
		while(args[j]) {
			if(i==(commandNum-1)) {
				tmpArgs[k] = (char*)calloc(1, sizeof(args[j]));
				strcpy(tmpArgs[k], args[j]);
				k++;
			}
			j++;
		}
		j++;
	}
	tmpArgs[k+1] = NULL;
	return;
}

int main() {
    int parseReturn;
	pid_t pid, c_pid;
	int status;
	int i;

	signal(SIGTERM,handler);
	setpgid(0, getpid());
	for(;;) {
		printf("p2: ");
		parseReturn = parse();
		// EOF at beginning of line
		if(parseReturn == -1) break;
		// Empty line, reprompt user
		if(parseReturn == 2) continue;
		if(parseReturn == 0) {
			// exec
			if(execFlag == 1) {
				execvp(args[0], args);
				perror("execvp");
				memset(args, 0, sizeof(args));
				memset(tmpArgs, 0, sizeof(tmpArgs));
				exit(9);
			}
			//ls-F command entered
			if(strcmp(args[0],"ls-F") == 0) {
				if(count >= 2) {
					int j = 0;
					int k = 0;
					while(args[j+1]) {
						struct stat st;
						if(stat(args[j+1], &st) == -1) {
							perror("stat");
							exit(-1);
						} 
						if(S_ISDIR(st.st_mode)) {
							char cwd[256];
							getcwd(cwd, sizeof(cwd));
							DIR *dirp;
							struct dirent *dp;
							struct stat st1;
							struct stat st2;
							struct stat st3;
							if((dirp = opendir(args[j+1])) != NULL) {
								chdir(args[j+1]);
								while(dirp) {
									if((dp = readdir(dirp)) != NULL) {
										stat(dp->d_name, &st1);
										if(S_ISDIR(st1.st_mode)) {
											printf("%s/\n",dp->d_name);
										} else if (S_ISREG(st1.st_mode)) {
											if((k = lstat(dp->d_name, &st2)) < 0)  {
												perror("lstat");
											} else {
												// check if link broken
												if(stat(dp->d_name, &st3) < 0) {
													printf("%s&\n", dp->d_name);
												} else if (S_ISLNK(st2.st_mode)) {
													// link is active
													printf("%s@\n", dp->d_name);
												} else {
													if(st1.st_mode & 0111) {
														// execution bits set
														printf("%s*\n", dp->d_name);
													} else {
														printf("%s\n", dp->d_name);
													} 
												}
											}
										} else {
											printf("%s\n", dp->d_name);
										} 
									} else {
										closedir(dirp);
										chdir(cwd);
										break;
									}
								}
							} else {
								perror("opendir");
								fprintf(stderr, "cannot open: %s\n", args[j+1]);
							}
							j++;
							continue;
						}
						if(S_ISREG(st.st_mode)) {
							struct stat st2;
							if((k = lstat(args[j+1], &st2)) < 0)  {
								printf("%s&\n", args[j+1]);
							} else {
								if (S_ISLNK(st2.st_mode)) {
									printf("%s@\n", args[j+1]);
								} else {
									if(st2.st_mode & 0111) {
										printf("%s*\n", args[j+1]);
									} else {
										printf("%s\n", args[j+1]);
									} 
								}
							}
						}
						j++;
					}
				// list current directory
				} else {
					DIR *dirp;
					struct dirent *dp;
					struct stat st;
					int k = 0;
					if((dirp = opendir(".")) != NULL) {
						if(stat(".", &st) == -1) {
							memset(args, 0, sizeof(args));
							perror("stat");
						} else {
							while(dirp) {
								if((dp = readdir(dirp)) != NULL) {
									if(stat(dp->d_name, &st) == -1){ 
										memset(args, 0, sizeof(args));
										perror("stat");
									}
									if(S_ISDIR(st.st_mode)) {
										printf("%s/\n",dp->d_name);
									} else if (S_ISREG(st.st_mode)) {
										if((k = lstat(dp->d_name, &st)) < 0)  {
											printf("%s&\n", dp->d_name);
										} else {
											if (S_ISLNK(st.st_mode)) {
												printf("%s@\n", dp->d_name);
											} else {
												if(st.st_mode & 0111) {
													printf("%s*\n", dp->d_name);
												} else {
													printf("%s\n", dp->d_name);
												} 
											}
										}
									} else {
										printf("%s\n", dp->d_name);
									} 
								} else {
									closedir(dirp);
									break;
								}
							}
						}
					} else {
						perror("opendir");
						fprintf(stderr, "cannot open: '.'\n");
					}
				}
				continue;
			}
			// cd command
			if(strcmp(args[0],"cd") == 0) {
				if(count > 2) {
					memset(args, 0, sizeof(args));
					fprintf(stderr, "Too many arguments given to cd\n");
				} else if(count == 1){
					chdir(getenv("HOME"));
				} else {
					chdir(args[1]);
				}
				continue;
			}
			// ">!" seen
			if(overWrite != 0){
				if(overWrite > 2) {
				 fprintf(stderr, "Too many overwrite commands\n");
				 memset(args, 0, sizeof(args));
				 inWord[0] = '\0';
				 continue;
				}
				if(overWriteWord[0] == '\0'){
					fprintf(stderr, "Missing arg for overwrite.\n");
					memset(args, 0, sizeof(args));
					inWord[0] = '\0';
					continue;
				}
				if((owfd = open(overWriteWord, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
					memset(args, 0, sizeof(args));
					perror("open");
					fprintf(stderr, "cannot open: %s\n", overWriteWord);
					overWriteWord[0] = '\0';
					continue;
				}
			}
			// "<" seen
			if(readFrom != 0){
				if(readFrom > 2) {
				 fprintf(stderr, "Too many inward redirections\n");
				 memset(args, 0, sizeof(args));
				 inWord[0] = '\0';
				 continue;
				}
				if(inWord[0] == '\0'){
					fprintf(stderr, "Missing arg for redirect.\n");
					memset(args, 0, sizeof(args));
					inWord[0] = '\0';
					continue;
				}
				if((ifd = open(inWord, O_RDONLY)) < 0){
					memset(args, 0, sizeof(args));
					perror("open");
					fprintf(stderr, "cannot open: %s\n", inWord);
					inWord[0] = '\0';
					continue;
				}
			}
			// ">" seen
			if(writeTo != 0) {
				if(writeTo > 2){ 
					fprintf(stderr, "Too many outward redirections\n");
					memset(args, 0, sizeof(args));
					outWord[0] = '\0';
					continue;
				}
				if(outWord[0] == '\0'){
					fprintf(stderr, "Missing arg for redirect.\n");
					memset(args, 0, sizeof(args));
					outWord[0] = '\0';
					continue;
				}
				if((ofd = open(outWord, O_RDWR | O_CREAT | O_EXCL, S_IRWXU)) < 0){
					memset(args, 0, sizeof(args));
					perror("open");
					fprintf(stderr, "cannot open: %s\n", outWord);
					outWord[0] = '\0';
					continue;
				}
			}
			// "&" seen and no "<" seen
			if(bg != 0 && readFrom == 0) {
				if((devnull = open("/dev/null", O_RDONLY)) < 0){
					memset(args, 0, sizeof(args));
					perror("open");
					exit(9);
				}
			}
			
			// redirections done now need to fork
			fflush(stdout);
			fflush(stderr);

			c_pid = fork();
			if (c_pid == -1) {  
				memset(args, 0, sizeof(args));  
		        perror("fork");
		        exit(1);
			} else if(c_pid == 0){
				redirect();
				if(pipeCount > 0) {
					// Need ot make room for enough pipes for all the commands
					int pipefd[2*pipeCount];
					pid_t children[pipeCount];
					int k;

					for(i=0; i<pipeCount; i++) {
						pipe(pipefd + (i*2));
						
						children[i] = fork();
						if (children[i] == -1) {
							memset(args, 0, sizeof(args)); 
							memset(tmpArgs, 0, sizeof(tmpArgs));   
					        perror("fork");
					        exit(1);
						} else if(children[i] == 0) { // child
							if(i<pipeCount-1) { // middle child
								continue;
							} else { // last command (far left)
								if(pipeCount==1) {
									getCommand(1);
									if(dup2(pipefd[1], STDOUT_FILENO)<0) {
										perror("dup2");
										exit(1);
									}
									close(pipefd[1]);
									execvp(args[0], args);
									perror("execvp");
									memset(args, 0, sizeof(args));
									memset(tmpArgs, 0, sizeof(tmpArgs));
									exit(9);
								} else {
									dup2(pipefd[(pipeCount*2)-1], STDOUT_FILENO);
									for(k=0; k<(pipeCount*2)-1; k++) {
										close(pipefd[k]);
									}
									execvp(args[0], args);
									perror("execvp");
									memset(args, 0, sizeof(args));
									memset(tmpArgs, 0, sizeof(tmpArgs));
									exit(9);
								}
							}
						} else { // parent
							if(i > 0) { // middle parent
								getCommand((pipeCount+1)-i);
								dup2(pipefd[i*2], STDIN_FILENO);
								dup2(pipefd[(i*2)-1], STDOUT_FILENO);
								for(k=0; k<(i*2)+2; k++) {
									close(pipefd[k]);
								}
								execvp(tmpArgs[0], tmpArgs);
								perror("execvp");
								memset(args, 0, sizeof(args));
								memset(tmpArgs, 0, sizeof(tmpArgs));
								exit(9);
							} else { // first parent (far right command)
								getCommand(pipeCount+1);
								if(dup2(pipefd[0],  STDIN_FILENO) < 0) {
									perror("dup2");
									exit(1);
								}
								close(pipefd[0]);
								close(pipefd[1]);
								execvp(tmpArgs[0], tmpArgs);
								perror("execvp");
								memset(args, 0, sizeof(args));
								memset(tmpArgs, 0, sizeof(tmpArgs));
								exit(9);
							}
						}
					}

					
				} else {
		     		execvp(args[0], args);
		     		memset(args, 0, sizeof(args));
					perror("execvp");
					exit(9);
				}
			} else {
				// background process
				if(bg != 0) {
					printf("%s [%d]\n", args[0] , c_pid);
					continue;
				} else {
					// wait for children to finish
					 while(wait(NULL) != c_pid);
				}	
			}
        }
        // clear the argument arrays
        memset(args, 0, sizeof(args));
        memset(tmpArgs, 0, sizeof(tmpArgs));
	}   
	killpg(getpgrp(), SIGTERM); // terminate all running children
	printf("p2 terminated.\n");
	exit(0);
}

// Gets the line from STDIN splitting it into an args of words
int parse() {
	int c;
	count = 0;
	readFrom = 0;
	writeTo = 0;
	bg = 0;
	pipeCount = 0;
	overWrite = 0;
	execFlag = 0;
	// get words until EOF, newline, ';', or '&'
	for(;;) {
		c = getword(word);
		if(c == -3) {
			fprintf(stderr, "error: mismatched quotes\n");
			continue;
		}
		if(strcmp(word,"exec") == 0) {
			execFlag++;
			continue;
		}
		if(word[0] == '&' && c==1) {
			bg++;
			break;
		}
		if(readFrom == 1 && c > 0) {
			strcpy(inWord, word);
			readFrom++;
			continue;
		}
		if(word[0] == '<' && c==1) {
			readFrom++;
			continue;
		}
		if(overWrite == 1 && c > 0) {
			strcpy(overWriteWord, word);
			overWrite++;
			continue;
		}
		if(word[0] == '>' && word[1] == '!' && c==2) {
			overWrite++;
			continue;
		}
		if(writeTo == 1 && c > 0) {
			strcpy(outWord, word);
			writeTo++;
			continue;
		}
		if(word[0] == '>' && c==1) {
			writeTo++;
			continue;
		}
		if(word[0] == '|' && c==1) {
			args[count] = NULL;
			pipeCount++;
			count++;
			continue;
		}
		if((c == 0) && (count == 0)) return 2;
		if(c == -1) return -1;
		if((c == 0) && (count != 0)) break;
		// store command and following arguments
	    args[count] = (char*)calloc(1, sizeof(word));
	    strcpy(args[count], word);
	    count++;
	}
	args[count+1] = NULL;
	return 0;
}

void handler(){};
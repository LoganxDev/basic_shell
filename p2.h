#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

#include "getword.h"

int parse();

void handler();
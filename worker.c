#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "campus.h"
int worker_demo(int fail) {
    int status; printf("\n=== Worker: %s ===\n",fail?"controlled simulated failure":"normal completion"); fflush(stdout);
    pid_t p=fork(); if(p<0) { perror("fork"); return 1; }
    if(p==0) { printf("Child PID %ld: %s\n",(long)getpid(),fail?"simulating failure, exiting 1":"work complete, exiting 0"); fflush(stdout); _exit(fail?1:0); }
    printf("Parent created child PID %ld.\n",(long)p); fflush(stdout);
    while(waitpid(p,&status,0)<0) { if(errno!=EINTR) { perror("waitpid"); return 1; } }
    if(WIFEXITED(status)) { int code=WEXITSTATUS(status); printf("Parent collected worker: exit status %d%s.\n",code,fail?" (simulated failure)":""); return code==(fail?1:0)?0:1; }
    if(WIFSIGNALED(status)) fprintf(stderr,"Worker terminated by signal %d.\n",WTERMSIG(status));
    else fprintf(stderr,"Worker ended unexpectedly.\n");
    return 1;
}

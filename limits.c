#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "campus.h"
int limit_demo(void) {
    int status; puts("\n=== Resource limit: RLIMIT_NOFILE (child only) ==="); fflush(stdout);
    pid_t p=fork(); if(p<0) { perror("fork"); return 1; }
    if(p==0) {
        struct rlimit lim; int fds[128], count=0, observed=0;
        if(getrlimit(RLIMIT_NOFILE,&lim)<0) { perror("getrlimit"); _exit(1); }
        if(lim.rlim_cur>32) lim.rlim_cur=32;
        if(setrlimit(RLIMIT_NOFILE,&lim)<0) { perror("setrlimit"); _exit(1); }
        printf("Child soft open-file limit: %llu\n",(unsigned long long)lim.rlim_cur);
        while(count<128) { int fd=open("/dev/null",O_RDONLY); if(fd<0) { int e=errno; printf("After %d opens: %s\n",count,strerror(e)); observed=(e==EMFILE); break; } fds[count++]=fd; }
        for(int i=0;i<count;++i) close(fds[i]);
        printf("Closed %d descriptors; parent limit unchanged.\n",count); fflush(stdout); _exit(observed?0:1);
    }
    while(waitpid(p,&status,0)<0) { if(errno!=EINTR) { perror("waitpid"); return 1; } }
    if(!WIFEXITED(status) || WEXITSTATUS(status)!=0) { fprintf(stderr,"Resource-limit demonstration failed.\n"); return 1; }
    puts("Resource-limit child collected."); return 0;
}

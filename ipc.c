#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "campus.h"

int ipc_demo(void) {
    char shmname[80], semname[80]; int fd=-1, status=0, error=0, shm_created=0, sem_created=0;
    char *shared=MAP_FAILED; sem_t *sem=SEM_FAILED; pid_t child=-1;
    snprintf(shmname,sizeof shmname,"/campus_shm_%ld_%ld",(long)getuid(),(long)getpid());
    snprintf(semname,sizeof semname,"/campus_sem_%ld_%ld",(long)getuid(),(long)getpid());
    puts("\n=== POSIX shared-memory IPC ==="); fflush(stdout);
    fd=shm_open(shmname,O_CREAT|O_EXCL|O_RDWR,0600);
    if(fd<0) { perror("shm_open"); return 1; } shm_created=1;
    if(ftruncate(fd,256)<0) { perror("ftruncate"); error=1; goto cleanup; }
    shared=mmap(NULL,256,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(shared==MAP_FAILED) { perror("mmap"); error=1; goto cleanup; }
    sem=sem_open(semname,O_CREAT|O_EXCL,0600,0);
    if(sem==SEM_FAILED) { perror("sem_open"); error=1; goto cleanup; } sem_created=1;
    child=fork();
    if(child<0) { perror("fork"); error=1; goto cleanup; }
    if(child==0) {
        int rc; do { rc=sem_wait(sem); } while(rc<0 && errno==EINTR);
        if(rc<0) { perror("child sem_wait"); _exit(1); }
        printf("Child %ld received through shared memory: %s\n",(long)getpid(),shared); fflush(stdout);
        sem_close(sem); munmap(shared,256); close(fd); _exit(0);
    }
    snprintf(shared,256,"Job 1 is ready for processing");
    printf("Parent %ld wrote message; signaling named semaphore.\n",(long)getpid()); fflush(stdout);
    if(sem_post(sem)<0) { perror("sem_post"); error=1; /* Avoid a child waiting forever. */
        if(kill(child, SIGTERM)<0) perror("kill"); }
    while(waitpid(child,&status,0)<0) { if(errno!=EINTR) { perror("waitpid"); error=1; break; } }
    if(!WIFEXITED(status) || WEXITSTATUS(status)!=0) { fprintf(stderr,"IPC child did not complete successfully.\n"); error=1; }
cleanup:
    if(sem!=SEM_FAILED && sem_close(sem)<0) { perror("sem_close"); error=1; }
    if(sem_created && sem_unlink(semname)<0) { perror("sem_unlink"); error=1; }
    if(shared!=MAP_FAILED && munmap(shared,256)<0) { perror("munmap"); error=1; }
    if(fd>=0 && close(fd)<0) { perror("close shm"); error=1; }
    if(shm_created && shm_unlink(shmname)<0) { perror("shm_unlink"); error=1; }
    if(!error) printf("Child collected; removed program-created IPC names %s and %s.\n",shmname,semname);
    return error;
}

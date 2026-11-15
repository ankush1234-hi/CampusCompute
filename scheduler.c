#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "campus.h"

/* Equal arrivals use smaller ID. RR arrivals at a boundary enter before the
   interrupted job is requeued. Input bounds keep all timeline arithmetic safe. */
static int before(const Job *a, const Job *b) {
    return a->arrival < b->arrival || (a->arrival == b->arrival && a->id < b->id);
}
static void sort(Job *j, int n) {
    for (int i=1; i<n; ++i) { Job x=j[i]; int k=i; while (k>0 && before(&x,&j[k-1])) { j[k]=j[k-1]; --k; } j[k]=x; }
}
static int read_jobs(const char *path, Job *j, int *n, int *quantum) {
    FILE *f=fopen(path,"r"); if (!f) { perror(path); return 1; }
    if (fscanf(f,"%d%d",n,quantum)!=2 || *n<1 || *n>MAX_JOBS || *quantum<1 || *quantum>10000) {
        fprintf(stderr,"Invalid job header: expected count 1..%d and quantum 1..10000.\n",MAX_JOBS); fclose(f); return 1;
    }
    for (int i=0;i<*n;++i) {
        if (fscanf(f,"%d%d%d%d",&j[i].id,&j[i].arrival,&j[i].burst,&j[i].priority)!=4 ||
            j[i].id<1 || j[i].arrival<0 || j[i].arrival>10000 || j[i].burst<1 || j[i].burst>10000 ||
            j[i].priority<0 || j[i].priority>10000) {
            fprintf(stderr,"Invalid job %d: expected positive ID, arrival 0..10000, burst 1..10000, priority 0..10000.\n",i+1); fclose(f); return 1;
        }
        for (int k=0;k<i;++k) if(j[k].id==j[i].id) { fprintf(stderr,"Duplicate job ID %d.\n",j[i].id); fclose(f); return 1; }
        j[i].remaining=j[i].burst; j[i].completion=0;
    }
    char extra[2]; if (fscanf(f,"%1s",extra)==1) { fprintf(stderr,"Unexpected extra content in %s.\n",path); fclose(f); return 1; }
    if (fclose(f)!=0) { perror("closing job file"); return 1; }
    sort(j,*n); return 0;
}
static void results(Job *j,int n) {
    double tw=0,tt=0; puts("ID Arrival Burst Priority Completion Turnaround Waiting Status");
    for(int i=0;i<n;++i) { int t=j[i].completion-j[i].arrival, w=t-j[i].burst; tt+=t; tw+=w;
        printf("%2d %7d %5d %8d %10d %10d %7d completed\n",j[i].id,j[i].arrival,j[i].burst,j[i].priority,j[i].completion,t,w); }
    printf("Average turnaround: %.2f | Average waiting: %.2f\n",tt/n,tw/n);
}
static void run(const char *policy, const Job *input, int n, int quantum) {
    Job j[MAX_JOBS]; memcpy(j,input,(size_t)n*sizeof(*j)); int t=0, done=0;
    printf("\n=== %s ===\n",policy);
    puts("Ties: earlier arrival, then smaller job ID. Priority: smaller number is higher.");
    if(!strcmp(policy,"rr")) {
        int q[MAX_JOBS], head=0, tail=0, count=0, next=0;
        printf("Time quantum: %d; boundary arrivals enter before requeued job.\n",quantum);
        while(done<n) {
            if(count==0 && next<n && t<j[next].arrival) { printf("IDLE [%d,%d)\n",t,j[next].arrival); t=j[next].arrival; }
            while(next<n && j[next].arrival<=t) { q[tail]=next++; tail=(tail+1)%MAX_JOBS; ++count; }
            int x=q[head]; head=(head+1)%MAX_JOBS; --count;
            int slice=j[x].remaining<quantum?j[x].remaining:quantum;
            printf("Job %d [%d,%d)\n",j[x].id,t,t+slice); t+=slice; j[x].remaining-=slice;
            while(next<n && j[next].arrival<=t) { q[tail]=next++; tail=(tail+1)%MAX_JOBS; ++count; }
            if(j[x].remaining) { q[tail]=x; tail=(tail+1)%MAX_JOBS; ++count; }
            else { j[x].completion=t; ++done; }
        }
    } else {
        while(done<n) {
            int x=-1;
            for(int i=0;i<n;++i) if(j[i].remaining && j[i].arrival<=t &&
                (x<0 || (!strcmp(policy,"priority") && j[i].priority<j[x].priority))) x=i;
            if(x<0) { int earliest=INT_MAX; for(int i=0;i<n;++i) if(j[i].remaining && j[i].arrival<earliest) earliest=j[i].arrival;
                printf("IDLE [%d,%d)\n",t,earliest); t=earliest; continue; }
            printf("Job %d [%d,%d)\n",j[x].id,t,t+j[x].burst); t+=j[x].burst; j[x].remaining=0; j[x].completion=t; ++done;
        }
    }
    results(j,n);
}
int schedule_command(const char *policy,const char *path) {
    Job j[MAX_JOBS]; int n,q;
    if(strcmp(policy,"fcfs") && strcmp(policy,"rr") && strcmp(policy,"priority") && strcmp(policy,"compare")) {
        fprintf(stderr,"Invalid policy: use fcfs, rr, priority, or compare.\n"); return 1;
    }
    if(read_jobs(path,j,&n,&q)) return 1;
    if(!strcmp(policy,"compare")) { run("fcfs",j,n,q); run("rr",j,n,q); run("priority",j,n,q); }
    else run(policy,j,n,q);
    return 0;
}

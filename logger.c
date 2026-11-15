#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "campus.h"

#define CAPACITY 3
#define EVENTS 8
typedef struct {
    char entries[CAPACITY][96]; int head, tail, count, finished, failed;
    pthread_mutex_t mutex; pthread_cond_t not_empty, not_full; FILE *file;
} Queue;

static void *produce(void *arg) {
    Queue *q=arg;
    for(int i=1;i<=EVENTS;++i) {
        char entry[96]; snprintf(entry,sizeof entry,"Event %02d: simulated job activity",i);
        pthread_mutex_lock(&q->mutex);
        while(q->count==CAPACITY) pthread_cond_wait(&q->not_full,&q->mutex);
        strcpy(q->entries[q->tail],entry); q->tail=(q->tail+1)%CAPACITY; ++q->count;
        printf("Producer queued: %s (queue=%d)\n",entry,q->count);
        pthread_cond_signal(&q->not_empty); pthread_mutex_unlock(&q->mutex);
    }
    pthread_mutex_lock(&q->mutex); q->finished=1; pthread_cond_signal(&q->not_empty); pthread_mutex_unlock(&q->mutex);
    return NULL;
}
static void *consume(void *arg) {
    Queue *q=arg;
    for(;;) {
        char entry[96];
        pthread_mutex_lock(&q->mutex);
        while(q->count==0 && !q->finished) pthread_cond_wait(&q->not_empty,&q->mutex);
        if(q->count==0 && q->finished) { pthread_mutex_unlock(&q->mutex); break; }
        strcpy(entry,q->entries[q->head]); q->head=(q->head+1)%CAPACITY; --q->count;
        printf("Consumer removed: %s (queue=%d)\n",entry,q->count);
        pthread_cond_signal(&q->not_full); pthread_mutex_unlock(&q->mutex);
        if(fprintf(q->file,"%s\n",entry)<0) q->failed=1;
    }
    return NULL;
}
int logger_demo(void) {
    Queue q={0}; pthread_t producer,consumer; int rc;
    q.file=fopen("campuscompute.log","a"); if(!q.file) { perror("campuscompute.log"); return 1; }
    if((rc=pthread_mutex_init(&q.mutex,NULL))) { fprintf(stderr,"mutex init: %s\n",strerror(rc)); fclose(q.file); return 1; }
    if((rc=pthread_cond_init(&q.not_empty,NULL))) { fprintf(stderr,"condition init: %s\n",strerror(rc)); pthread_mutex_destroy(&q.mutex); fclose(q.file); return 1; }
    if((rc=pthread_cond_init(&q.not_full,NULL))) { fprintf(stderr,"condition init: %s\n",strerror(rc)); pthread_cond_destroy(&q.not_empty); pthread_mutex_destroy(&q.mutex); fclose(q.file); return 1; }
    puts("\n=== Producer-consumer logger: capacity 3 ===");
    if((rc=pthread_create(&consumer,NULL,consume,&q))) { fprintf(stderr,"consumer create: %s\n",strerror(rc)); q.failed=1; goto cleanup; }
    if((rc=pthread_create(&producer,NULL,produce,&q))) {
        fprintf(stderr,"producer create: %s\n",strerror(rc)); pthread_mutex_lock(&q.mutex); q.finished=1;
        pthread_cond_signal(&q.not_empty); pthread_mutex_unlock(&q.mutex); pthread_join(consumer,NULL); q.failed=1; goto cleanup;
    }
    pthread_join(producer,NULL); pthread_join(consumer,NULL);
    puts("Producer and consumer joined; all queued messages processed.");
cleanup:
    pthread_cond_destroy(&q.not_full); pthread_cond_destroy(&q.not_empty); pthread_mutex_destroy(&q.mutex);
    if(fclose(q.file)!=0) { perror("closing log"); q.failed=1; }
    if(q.failed) { fprintf(stderr,"Logger encountered an error.\n"); return 1; }
    puts("Log saved to campuscompute.log; synchronization objects destroyed."); return 0;
}

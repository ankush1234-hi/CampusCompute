#ifndef CAMPUS_H
#define CAMPUS_H
#define MAX_JOBS 50
#define MAX_RES 10
typedef struct { int id, arrival, burst, priority, remaining, completion; } Job;
int schedule_command(const char *policy, const char *path);
int banker_command(const char *path);
int logger_demo(void);
int ipc_demo(void);
int worker_demo(int fail);
int limit_demo(void);
#endif

#include <stdio.h>
#include <string.h>
#include "campus.h"

static void help(void) {
    puts("CampusCompute - OS mini-project\n"
         "Usage: ./campuscompute COMMAND [arguments]\n"
         "  schedule fcfs|rr|priority|compare JOB_FILE\n"
         "  banker RESOURCE_FILE\n"
         "  logger                 bounded producer-consumer log\n"
         "  ipc                    POSIX shared memory and semaphore\n"
         "  worker normal|fail     child process status demonstration\n"
         "  limit                  file-descriptor limit demonstration\n"
         "  demo                   run all features with sample files\n"
         "  help                   show this message\n"
         "See README.md for file formats and examples.");
}

int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "help")) { help(); return 0; }
    if (argc == 4 && !strcmp(argv[1], "schedule"))
        return schedule_command(argv[2], argv[3]);
    if (argc == 3 && !strcmp(argv[1], "banker")) return banker_command(argv[2]);
    if (argc == 2 && !strcmp(argv[1], "logger")) return logger_demo();
    if (argc == 2 && !strcmp(argv[1], "ipc")) return ipc_demo();
    if (argc == 3 && !strcmp(argv[1], "worker")) {
        if (!strcmp(argv[2], "normal")) return worker_demo(0);
        if (!strcmp(argv[2], "fail")) return worker_demo(1);
    }
    if (argc == 2 && !strcmp(argv[1], "limit")) return limit_demo();
    if (argc == 2 && !strcmp(argv[1], "demo")) {
        int failures = 0;
        puts("=== Integrated CampusCompute demonstration ===");
        failures += schedule_command("compare", "jobs.txt") != 0;
        failures += logger_demo() != 0;
        failures += ipc_demo() != 0;
        failures += banker_command("safe.txt") != 0;
        failures += banker_command("unsafe.txt") != 0;
        failures += worker_demo(0) != 0;
        failures += worker_demo(1) != 0;
        failures += limit_demo() != 0;
        printf("=== Integrated summary: %d module error(s) ===\n", failures);
        return failures ? 1 : 0;
    }
    fprintf(stderr, "Error: invalid command or arguments.\n"); help(); return 1;
}

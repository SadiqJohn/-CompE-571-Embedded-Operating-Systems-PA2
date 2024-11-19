#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define WORKLOAD1 100000
#define WORKLOAD2 50000
#define WORKLOAD3 25000
#define WORKLOAD4 10000
#define QUANTUM 5  // Optimal 

void myfunction(int param) {
    int i = 2;
    int j, k;
    while (i < param) {
        k = i;
        for (j = 2; j <= k; j++) {
            if (k % j == 0) {
                k = k / j;
                j--;
                if (k == 1) break;
            }
        }
        i++;
    }
}

int main(int argc, char const *argv[]) {
    pid_t pid1, pid2, pid3, pid4;
    struct timespec creation_time[4], start_time[4], context_switch_start, context_switch_end;
    long response_times[4];
    int started[4] = {0, 0, 0, 0};  // Flags to mark if a process has started
    long context_switch_times[12];  // Array to store context switch times in nanoseconds
    int cs_index = 0;

    int running1 = 1, running2 = 1, running3 = 1, running4 = 1;

    // Create each process and immediately stop it
    clock_gettime(CLOCK_MONOTONIC, &creation_time[0]);
    pid1 = fork();
    if (pid1 == 0) {
        myfunction(WORKLOAD1);
        exit(0);
    }
    kill(pid1, SIGSTOP);

    clock_gettime(CLOCK_MONOTONIC, &creation_time[1]);
    pid2 = fork();
    if (pid2 == 0) {
        myfunction(WORKLOAD2);
        exit(0);
    }
    kill(pid2, SIGSTOP);

    clock_gettime(CLOCK_MONOTONIC, &creation_time[2]);
    pid3 = fork();
    if (pid3 == 0) {
        myfunction(WORKLOAD3);
        exit(0);
    }
    kill(pid3, SIGSTOP);

    clock_gettime(CLOCK_MONOTONIC, &creation_time[3]);
    pid4 = fork();
    if (pid4 == 0) {
        myfunction(WORKLOAD4);
        exit(0);
    }
    kill(pid4, SIGSTOP);

    // Start Round Robin scheduling
    while (running1 > 0 || running2 > 0 || running3 > 0 || running4 > 0) {
        if (running1 > 0) {
            clock_gettime(CLOCK_MONOTONIC, &context_switch_start);  // Start timing

            if (!started[0]) {
                clock_gettime(CLOCK_MONOTONIC, &start_time[0]);
                started[0] = 1;
            }
            kill(pid1, SIGCONT);
            usleep(QUANTUM);
            kill(pid1, SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &context_switch_end);  // End timing
            context_switch_times[cs_index++] = (context_switch_end.tv_sec - context_switch_start.tv_sec) * 1000000000L
                                               + (context_switch_end.tv_nsec - context_switch_start.tv_nsec);
        }

        if (running2 > 0) {
            clock_gettime(CLOCK_MONOTONIC, &context_switch_start);

            if (!started[1]) {
                clock_gettime(CLOCK_MONOTONIC, &start_time[1]);
                started[1] = 1;
            }
            kill(pid2, SIGCONT);
            usleep(QUANTUM);
            kill(pid2, SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &context_switch_end);
            context_switch_times[cs_index++] = (context_switch_end.tv_sec - context_switch_start.tv_sec) * 1000000000L
                                               + (context_switch_end.tv_nsec - context_switch_start.tv_nsec);
        }

        if (running3 > 0) {
            clock_gettime(CLOCK_MONOTONIC, &context_switch_start);

            if (!started[2]) {
                clock_gettime(CLOCK_MONOTONIC, &start_time[2]);
                started[2] = 1;
            }
            kill(pid3, SIGCONT);
            usleep(QUANTUM);
            kill(pid3, SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &context_switch_end);
            context_switch_times[cs_index++] = (context_switch_end.tv_sec - context_switch_start.tv_sec) * 1000000000L
                                               + (context_switch_end.tv_nsec - context_switch_start.tv_nsec);
        }

        if (running4 > 0) {
            clock_gettime(CLOCK_MONOTONIC, &context_switch_start);

            if (!started[3]) {
                clock_gettime(CLOCK_MONOTONIC, &start_time[3]);
                started[3] = 1;
            }
            kill(pid4, SIGCONT);
            usleep(QUANTUM);
            kill(pid4, SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &context_switch_end);
            context_switch_times[cs_index++] = (context_switch_end.tv_sec - context_switch_start.tv_sec) * 1000000000L
                                               + (context_switch_end.tv_nsec - context_switch_start.tv_nsec);
        }

        waitpid(pid1, &running1, WNOHANG);
        waitpid(pid2, &running2, WNOHANG);
        waitpid(pid3, &running3, WNOHANG);
        waitpid(pid4, &running4, WNOHANG);
    }

    // Calculate response times in nanoseconds
    for (int i = 0; i < 4; i++) {
        response_times[i] = (start_time[i].tv_sec - creation_time[i].tv_sec) * 1000000000L;
        response_times[i] += (start_time[i].tv_nsec - creation_time[i].tv_nsec);
        printf("Response Time for Process %d: %ld nanoseconds\n", i + 1, response_times[i]);
    }

    // Calculate and print average response time
    long total_response_time = 0;
    for (int i = 0; i < 4; i++) {
        total_response_time += response_times[i];
    }
    long avg_response_time = total_response_time / 4;
    printf("Average Response Time: %ld nanoseconds\n", avg_response_time);

    // Calculate total and average context switch overhead
    long total_context_switch_time = 0;
    for (int i = 0; i < cs_index; i++) {
        total_context_switch_time += context_switch_times[i];
        printf("Context switch time %d: %ld nanoseconds\n", i + 1, context_switch_times[i]);
    }
    long average_context_switch_time = cs_index > 0 ? total_context_switch_time / cs_index : 0;
    printf("Total Context Switch Overhead: %ld nanoseconds\n", total_context_switch_time);
    printf("Average Context Switch Time: %ld nanoseconds\n", average_context_switch_time);

    return 0;
}

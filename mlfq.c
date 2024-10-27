#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

/************************************************************************************************ 
		These DEFINE statements represent the workload size of each task and 
		the time quantum values for Round Robin scheduling for each task.
*************************************************************************************************/

#define WORKLOAD1 100000
#define WORKLOAD2 50000
#define WORKLOAD3 25000
#define WORKLOAD4 10000

#define QUANTUM 1000   // Time quantum for the first-level Round Robin queue

/************************************************************************************************ 
					DO NOT CHANGE THE FUNCTION IMPLEMENTATION
*************************************************************************************************/
void myfunction(int param){

	int i = 2;
	int j, k;

	while(i < param){
		k = i; 
		for (j = 2; j <= k; j++)
		{
			if (k % j == 0){
				k = k/j;
				j--;
				if (k == 1){
					break;
				}
			}
		}
		i++;
	}
}
/************************************************************************************************/

/************************************************************************************************
	Function to calculate time difference in nanoseconds to ensure no negative values.
************************************************************************************************/
long calculate_time_difference(struct timespec start, struct timespec end) {
    long seconds_diff = end.tv_sec - start.tv_sec;
    long nanoseconds_diff = end.tv_nsec - start.tv_nsec;

    if (nanoseconds_diff < 0) {
        seconds_diff -= 1;
        nanoseconds_diff += 1000000000L;
    }

    return seconds_diff * 1000000000L + nanoseconds_diff;
}

int main(int argc, char const *argv[])
{
	pid_t pid1, pid2, pid3, pid4;
	struct timespec start_time[4] = {0}, end_time[4] = {0}, switch_start, switch_end;
	long response_times[4];
	long context_switch_times[100]; // Array to store context switch times
	int switch_count = 0;           // Counter for context switches

	// Create and immediately stop each process
	pid1 = fork();

	if (pid1 == 0){

		myfunction(WORKLOAD1);

		exit(0);
	}
	kill(pid1, SIGSTOP);

	pid2 = fork();

	if (pid2 == 0){

		myfunction(WORKLOAD2);

		exit(0);
	}
	kill(pid2, SIGSTOP);

	pid3 = fork();

	if (pid3 == 0){

		myfunction(WORKLOAD3);

		exit(0);
	}
	kill(pid3, SIGSTOP);

	pid4 = fork();

	if (pid4 == 0){

		myfunction(WORKLOAD4);

		exit(0);
	}
	kill(pid4, SIGSTOP);

	pid_t pids[4] = {pid1, pid2, pid3, pid4};
	int completed[4] = {0, 0, 0, 0};   // Track completion status of each process
	int in_fcfs_queue[4] = {0, 0, 0, 0};  // Track processes that have moved to the FCFS queue
	int finished = 0;

	/************************************************************************************************ 
		At this point, all newly-created child processes are stopped and ready for scheduling.
	*************************************************************************************************/



	/************************************************************************************************
		- Scheduling code starts here
		- The following code implements Multi-Level Feedback Queue (MLFQ) scheduling.
	************************************************************************************************/

	// First Queue: Round Robin, Second Queue: FCFS
	while (finished < 4) {
		int round_robin_active = 0; // Track if there are processes in the first-level queue

		for (int i = 0; i < 4; i++) {
			if (completed[i] || in_fcfs_queue[i]) continue;

			// Record start time only once when process runs for the first time in Round Robin
			if (start_time[i].tv_sec == 0 && start_time[i].tv_nsec == 0) {
				clock_gettime(CLOCK_MONOTONIC, &start_time[i]);
			}

			// Record time before context switch (process start)
			clock_gettime(CLOCK_MONOTONIC, &switch_start);

			// Run the process for the fixed time quantum in the Round Robin queue
			kill(pids[i], SIGCONT);
			usleep(QUANTUM);
			kill(pids[i], SIGSTOP);

			// Record time after context switch (process stop)
			clock_gettime(CLOCK_MONOTONIC, &switch_end);

			// Calculate and store context switch time
			context_switch_times[switch_count] = calculate_time_difference(switch_start, switch_end);
			switch_count++;
			round_robin_active = 1;

			// Check if the process has completed
			int status;
			if (waitpid(pids[i], &status, WNOHANG) > 0) {
				clock_gettime(CLOCK_MONOTONIC, &end_time[i]);
				completed[i] = 1;
				finished++;
			} else {
				// If not completed, move it to the FCFS queue
				in_fcfs_queue[i] = 1;
			}
		}

		// Process the second queue (FCFS) only if no processes are active in the first queue
		if (!round_robin_active) {
			for (int i = 0; i < 4; i++) {
				if (completed[i] || !in_fcfs_queue[i]) continue;

				// Capture context switch before starting process in FCFS
				clock_gettime(CLOCK_MONOTONIC, &switch_start);

				// Start and wait until process completes in FCFS
				kill(pids[i], SIGCONT);
				waitpid(pids[i], NULL, 0);
				clock_gettime(CLOCK_MONOTONIC, &end_time[i]);

				// Capture context switch after stopping
				clock_gettime(CLOCK_MONOTONIC, &switch_end);

				// Calculate and store context switch time for FCFS switch
				context_switch_times[switch_count] = calculate_time_difference(switch_start, switch_end);
				switch_count++;
				completed[i] = 1;
				finished++;
			}
		}
	}

	/************************************************************************************************
		- Scheduling code ends here
	************************************************************************************************/

	// Calculate response times and context switch overhead
	long total_response_time = 0;
	for (int i = 0; i < 4; i++) {
		response_times[i] = calculate_time_difference(start_time[i], end_time[i]);
		total_response_time += response_times[i];
		printf("Response time for process %d: %ld nanoseconds\n", i + 1, response_times[i]);
	}
	long average_response_time = total_response_time / 4;
	printf("Average response time: %ld nanoseconds\n", average_response_time);

	// Calculate total and average context switch overhead
	long total_context_switch_time = 0;
	for (int i = 0; i < switch_count; i++) {
		total_context_switch_time += context_switch_times[i];
		printf("Context switch time %d: %ld nanoseconds\n", i + 1, context_switch_times[i]);
	}
	long average_context_switch_time = total_context_switch_time / switch_count;
	printf("Total context switch overhead: %ld nanoseconds\n", total_context_switch_time);
	printf("Average context switch time per switch: %ld nanoseconds\n", average_context_switch_time);

	return 0;
}

#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <stdlib.h>  
#include <sys/wait.h> 
#include <string.h> 
#include <time.h> 
#include <signal.h>

/************************************************************************************************ 
		These DEFINE statements represent the workload size of each task and 
		the time quantum values for Round Robin scheduling for each task.
*************************************************************************************************/

#define WORKLOAD1 100000
#define WORKLOAD2 50000
#define WORKLOAD3 25000
#define WORKLOAD4 10000

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

int main(int argc, char const *argv[])
{
	pid_t pid1, pid2, pid3, pid4;
	struct timespec start_time[4], end_time[4];
	long response_times[4];
	long context_switch_times[3];  // Array to store context switch times between processes

	// Create the first process and immediately stop it
	pid1 = fork();
	if (pid1 == 0){
		myfunction(WORKLOAD1);
		exit(0);
	}
	kill(pid1, SIGSTOP);

	// Create the second process and immediately stop it
	pid2 = fork();
	if (pid2 == 0){
		myfunction(WORKLOAD2);
		exit(0);
	}
	kill(pid2, SIGSTOP);

	// Create the third process and immediately stop it
	pid3 = fork();
	if (pid3 == 0){
		myfunction(WORKLOAD3);
		exit(0);
	}
	kill(pid3, SIGSTOP);

	// Create the fourth process and immediately stop it
	pid4 = fork();
	if (pid4 == 0){
		myfunction(WORKLOAD4);
		exit(0);
	}
	kill(pid4, SIGSTOP);

	/************************************************************************************************ 
		At this point, all newly-created child processes are stopped and ready for scheduling.
	*************************************************************************************************/

	/************************************************************************************************
		- Scheduling code starts here
		- The following code implements First-Come, First-Served (FCFS) scheduling.
	************************************************************************************************/

	// Start each process in order and wait for it to complete before moving to the next

	// Start process 1 and record its response time
	clock_gettime(CLOCK_MONOTONIC, &start_time[0]);  // Capture start time
	kill(pid1, SIGCONT);                 // Start process 1
	waitpid(pid1, NULL, 0);              // Wait until process 1 completes
	clock_gettime(CLOCK_MONOTONIC, &end_time[0]);    // Capture end time

	// Start process 2 and record its response time
	clock_gettime(CLOCK_MONOTONIC, &start_time[1]);  // Capture start time
	kill(pid2, SIGCONT);                 // Start process 2
	waitpid(pid2, NULL, 0);              // Wait until process 2 completes
	clock_gettime(CLOCK_MONOTONIC, &end_time[1]);    // Capture end time

	// Calculate context switch time between process 1 and 2
	context_switch_times[0] = (start_time[1].tv_sec - end_time[0].tv_sec) * 1000000000L + (start_time[1].tv_nsec - end_time[0].tv_nsec);

	// Start process 3 and record its response time
	clock_gettime(CLOCK_MONOTONIC, &start_time[2]);  // Capture start time
	kill(pid3, SIGCONT);                 // Start process 3
	waitpid(pid3, NULL, 0);              // Wait until process 3 completes
	clock_gettime(CLOCK_MONOTONIC, &end_time[2]);    // Capture end time

	// Calculate context switch time between process 2 and 3
	context_switch_times[1] = (start_time[2].tv_sec - end_time[1].tv_sec) * 1000000000L + (start_time[2].tv_nsec - end_time[1].tv_nsec);

	// Start process 4 and record its response time
	clock_gettime(CLOCK_MONOTONIC, &start_time[3]);  // Capture start time
	kill(pid4, SIGCONT);                 // Start process 4
	waitpid(pid4, NULL, 0);              // Wait until process 4 completes
	clock_gettime(CLOCK_MONOTONIC, &end_time[3]);    // Capture end time

	// Calculate context switch time between process 3 and 4
	context_switch_times[2] = (start_time[3].tv_sec - end_time[2].tv_sec) * 1000000000L + (start_time[3].tv_nsec - end_time[2].tv_nsec);

	/************************************************************************************************
		- Scheduling code ends here
	************************************************************************************************/

	// Calculate response times in nanoseconds and print the average
	long total_response_time = 0;
	for (int i = 0; i < 4; i++) {
		response_times[i] = (end_time[i].tv_sec - start_time[i].tv_sec) * 1000000000L + (end_time[i].tv_nsec - start_time[i].tv_nsec);
		total_response_time += response_times[i];
		printf("Response time for process %d: %ld nanoseconds\n", i + 1, response_times[i]);
	}
	long average_response_time = total_response_time / 4;
	printf("Average response time: %ld nanoseconds\n", average_response_time);

	// Calculate total and average context switch overhead
	long total_context_switch_time = 0;
	for (int i = 0; i < 3; i++) {
		total_context_switch_time += context_switch_times[i];
		printf("Context switch time between process %d and %d: %ld nanoseconds\n", i + 1, i + 2, context_switch_times[i]);
	}
	long average_context_switch_time = total_context_switch_time / 3;
	printf("Total context switch overhead: %ld nanoseconds\n", total_context_switch_time);
	printf("Average context switch time per switch: %ld nanoseconds\n", average_context_switch_time);

	return 0;
}

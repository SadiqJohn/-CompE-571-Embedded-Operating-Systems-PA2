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

#define QUANTUM1 1000
#define QUANTUM2 1000
#define QUANTUM3 1000
#define QUANTUM4 1000

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
	struct timeval start_time[4], end_time[4];
	long response_times[4];

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
	gettimeofday(&start_time[0], NULL);  // Capture start time
	kill(pid1, SIGCONT);                 // Start process 1
	waitpid(pid1, NULL, 0);              // Wait until process 1 completes
	gettimeofday(&end_time[0], NULL);    // Capture end time

	// Start process 2 and record its response time
	gettimeofday(&start_time[1], NULL);  // Capture start time
	kill(pid2, SIGCONT);                 // Start process 2
	waitpid(pid2, NULL, 0);              // Wait until process 2 completes
	gettimeofday(&end_time[1], NULL);    // Capture end time

	// Start process 3 and record its response time
	gettimeofday(&start_time[2], NULL);  // Capture start time
	kill(pid3, SIGCONT);                 // Start process 3
	waitpid(pid3, NULL, 0);              // Wait until process 3 completes
	gettimeofday(&end_time[2], NULL);    // Capture end time

	// Start process 4 and record its response time
	gettimeofday(&start_time[3], NULL);  // Capture start time
	kill(pid4, SIGCONT);                 // Start process 4
	waitpid(pid4, NULL, 0);              // Wait until process 4 completes
	gettimeofday(&end_time[3], NULL);    // Capture end time

	/************************************************************************************************
		- Scheduling code ends here
	************************************************************************************************/

	// Calculate response times in microseconds and print the average
	long total_response_time = 0;
	for (int i = 0; i < 4; i++) {
		response_times[i] = (end_time[i].tv_sec - start_time[i].tv_sec) * 1000000L + (end_time[i].tv_usec - start_time[i].tv_usec);
		total_response_time += response_times[i];
		printf("Response time for process %d: %ld microseconds\n", i + 1, response_times[i]);
	}
	long average_response_time = total_response_time / 4;
	printf("Average response time: %ld microseconds\n", average_response_time);

	return 0;
}

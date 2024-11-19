#include <stdio.h>
#include <string.h>
#include <math.h>

#define FILE_PATH1 "input1.txt"
#define FILE_PATH2 "input2.txt"
#define MAX_TASK_NAME_LENGTH 10
#define MAX_TASKS 10

void swapTasks(int *a, int *b, int rowA[], int rowB[]) {
    int temp = *a;
    *a = *b;
    *b = temp;

    for (int i = 0; i < 4; ++i) {
        temp = rowA[i];
        rowA[i] = rowB[i];
        rowB[i] = temp;
    }
}

void sortByDeadlines(int numTasks, int deadlines[], char taskNames[][MAX_TASK_NAME_LENGTH + 1], int wcet[][4]) {
    for (int i = 0; i < numTasks - 1; i++) {
        for (int j = 0; j < numTasks - i - 1; j++) {
            if (deadlines[j] > deadlines[j + 1]) {
                swapTasks(&deadlines[j], &deadlines[j + 1], wcet[j], wcet[j + 1]);
                char temp[MAX_TASK_NAME_LENGTH + 1];
                strcpy(temp, taskNames[j]);
                strcpy(taskNames[j], taskNames[j + 1]);
                strcpy(taskNames[j + 1], temp);
            }
        }
    }
}

void print_aggregate_statistics(int total_time, double total_energy, int idle_time) {
    double idle_percentage = ((double)idle_time / total_time) * 100;

    printf("\n=== Aggregate Statistics ===\n");
    printf("Total System Execution Time: %d seconds\n", total_time);
    printf("Total Energy Consumption: %.2lf Joules\n", total_energy);
    printf("Percentage of Time Spent Idle: %.2lf%%\n", idle_percentage);
}

void schedule_EDF(int numTasks, int totalTime, int cpuPower[], char taskNames[][MAX_TASK_NAME_LENGTH + 1], int deadlines[], int wcet[][4]) {
    int remainingTime[MAX_TASKS];
    double total_energy = 0.0; // Track total energy
    int idle_time = 0;        // Track total idle time

    sortByDeadlines(numTasks, deadlines, taskNames, wcet);

    for (int i = 0; i < numTasks; ++i) {
        remainingTime[i] = wcet[i][0];
    }

    int lastScheduledTask = -1;
    int idleStartTime = -1;
    int idleDuration = 0;

    for (int time = 0; time < totalTime; ++time) {
        int earliestDeadlineTask = -1;
        int earliestDeadline = totalTime + 1;

        for (int i = 0; i < numTasks; ++i) {
            if (deadlines[i] < earliestDeadline && remainingTime[i] > 0) {
                earliestDeadline = deadlines[i];
                earliestDeadlineTask = i;
            }
        }

        if (earliestDeadlineTask != -1) {
            remainingTime[earliestDeadlineTask]--;

            if (earliestDeadlineTask != lastScheduledTask) {
                int frequency = cpuPower[0];
                double task_energy = frequency * wcet[earliestDeadlineTask][0] / 1000.0;
                printf("%d %s %d %d %.2lfJ\n", time, taskNames[earliestDeadlineTask], frequency, wcet[earliestDeadlineTask][0], task_energy);
                total_energy += task_energy;

                if (idleStartTime != -1) {
                    int idle_freq = cpuPower[4];
                    double idle_energy = idle_freq * idleDuration / 1000.0;
                    printf("%d IDLE %d %d %.2lfJ\n", idleStartTime, idle_freq, idleDuration, idle_energy);
                    idle_time += idleDuration;
                    total_energy += idle_energy;
                    idleStartTime = -1;
                    idleDuration = 0;
                }

                lastScheduledTask = earliestDeadlineTask;
            }

            if (remainingTime[earliestDeadlineTask] == 0) {
                deadlines[earliestDeadlineTask] = totalTime + 1;
                remainingTime[earliestDeadlineTask] = wcet[earliestDeadlineTask][0];
            }
        } else {
            if (idleStartTime == -1) {
                idleStartTime = time;
            }
            idleDuration++;
        }
    }

    if (idleStartTime != -1) {
        int idle_freq = cpuPower[4];
        double idle_energy = idle_freq * idleDuration / 1000.0;
        printf("%d IDLE %d %d %.2lfJ\n", idleStartTime, idle_freq, idleDuration, idle_energy);
        idle_time += idleDuration;
        total_energy += idle_energy;
    }

    // Print aggregate statistics
    print_aggregate_statistics(totalTime, total_energy, idle_time);
}

void schedule_RM(int numTasks, int totalTime, int cpuPower[], char taskNames[][MAX_TASK_NAME_LENGTH + 1], int deadlines[], int wcet[][4]) {
    // Implementation needed
}

void schedule_EE_EDF(int numTasks, int totalTime, int cpuPower[], char taskNames[][MAX_TASK_NAME_LENGTH + 1], int deadlines[], int wcet[][4]) {
    // Implementation needed
}

void schedule_EE_RM(int numTasks, int totalTime, int cpuPower[], char taskNames[][MAX_TASK_NAME_LENGTH + 1], int deadlines[], int wcet[][4]) {
    // Implementation needed
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        printf("Usage: %s <input_file_name> <EDF or RM> [EE]\n", argv[0]);
        return 1;
    }

    FILE *file;
    if (strcmp(argv[1], "input1.txt") == 0) {
        file = fopen(FILE_PATH1, "r");
    } else if (strcmp(argv[1], "input2.txt") == 0) {
        file = fopen(FILE_PATH2, "r");
    } else {
        printf("Invalid input file name. Supported files are input1.txt and input2.txt.\n");
        return 1;
    }

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int numTasks, totalTime, cpuPower[5];
    if (fscanf(file, "%d %d %d %d %d %d %d", &numTasks, &totalTime, &cpuPower[0], &cpuPower[1], &cpuPower[2], &cpuPower[3], &cpuPower[4]) != 7) {
        printf("Error reading values from the file\n");
        fclose(file);
        return 1;
    }

    char taskNames[MAX_TASKS][MAX_TASK_NAME_LENGTH + 1];
    int deadlines[MAX_TASKS];
    int wcet[MAX_TASKS][4];

    for (int i = 0; i < numTasks; ++i) {
        if (fscanf(file, "%s %d %d %d %d %d", taskNames[i], &deadlines[i], &wcet[i][0], &wcet[i][1], &wcet[i][2], &wcet[i][3]) != 6) {
            printf("Error reading values from the file\n");
            fclose(file);
            return 1;
        }
    }

    fclose(file);

    if (strcmp(argv[2], "EDF") == 0) {
        if (argc == 3) {
            schedule_EDF(numTasks, totalTime, cpuPower, taskNames, deadlines, wcet);
        } else if (argc == 4 && strcmp(argv[3], "EE") == 0) {
            schedule_EE_EDF(numTasks, totalTime, cpuPower, taskNames, deadlines, wcet);
        }
    } else if (strcmp(argv[2], "RM") == 0) {
        if (argc == 3) {
            schedule_RM(numTasks, totalTime, cpuPower, taskNames, deadlines, wcet);
        } else if (argc == 4 && strcmp(argv[3], "EE") == 0) {
            schedule_EE_RM(numTasks, totalTime, cpuPower, taskNames, deadlines, wcet);
        }
    } else {
        printf("Invalid scheduling strategy. Use 'EDF' or 'RM'.\n");
        return 1;
    }

    return 0;
}

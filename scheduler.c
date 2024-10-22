#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct job {
    int jobId;
    int jobLength;
    int comparisonKey;
    int timeRemaining;
    int executionCount;
    int jobPriority;
    int firstResponseTime;
    int totalTurnaroundTime;
    int totalWaitTime;
    struct job *nextJob;
} Job;

typedef struct jobList {
    int jobCount;
    Job *firstJob;
} JobList;

char inputBuffer[3];
char *schedulingPolicy;
char *jobFilePath;
int roundRobinTimeSlice;
int systemClock = 0;
int completedJobsCount = 0;

// Creates and returns an empty job list
JobList *createJobList();

// Adds a job to the job list
void insertJobToList(JobList *jobList, int jobId, int jobLength, int jobPriority);

// Loads jobs from the job trace file and inserts them into the job list
void loadJobsFromTraceFile(char *localJobFilePath, JobList *jobList, char policyType);

// Prints the execution order of jobs in the list
void printJobExecutionOrder(JobList *jobList);

// Executes the jobs in the Round Robin queue
void executeRoundRobinQueue(JobList *jobList, int timeSlice);

// Splits the job list into two halves for merge sort
void splitJobListForMergeSort(Job *jobHead, Job **frontRef, Job **backRef);

// Merges two sorted job lists
Job *mergeSortedJobLists(Job *firstJob, Job *secondJob);

// Performs a merge sort on the job list
void mergeSortJobList(Job **jobHeadRef);

// Analyzes the scheduling policy (FIFO, SJF, PRIO) and prints statistics
void analyzeSchedulingPolicy(JobList *jobList, char policyType);

// Analyzes the Round Robin scheduling and prints statistics
void analyzeRoundRobinPolicy(JobList *jobList, int timeSlice);

// Schedules jobs using the FIFO policy
void scheduleUsingFIFO(char *localJobFilePath);

// Schedules jobs using the SJF policy
void scheduleUsingSJF(char *localJobFilePath);

// Schedules jobs using the Round Robin policy
void scheduleUsingRoundRobin(char *localJobFilePath, int timeSlice);

// Parses command line arguments and sets global variables
void parseCommandLineArguments(char **arguments);

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Wrong number of arguments! Your input should be in the form of: ./scheduler policy_name job_trace time_slice.\n");
        return 0;
    }

    parseCommandLineArguments(argv);

    switch (schedulingPolicy[0]) {
        case 'F':
            scheduleUsingFIFO(jobFilePath);
            break;
        case 'S':
            scheduleUsingSJF(jobFilePath);
            break;
        case 'R':
            scheduleUsingRoundRobin(jobFilePath, roundRobinTimeSlice);
            break;
        case 'P':
            printf("Execution trace with PRIO:\n");
            JobList *jobList = createJobList();
            loadJobsFromTraceFile(jobFilePath, jobList, 'P');
            mergeSortJobList(&(jobList->firstJob));
            printJobExecutionOrder(jobList);
            printf("End of execution with PRIO.\n");
            printf("Begin analyzing PRIO:\n");
            analyzeSchedulingPolicy(jobList, 'P');
            printf("End analyzing PRIO.\n");
            break;
        default:
            printf("Incorrect policy name entered!\n");
            break;
    }

    return 0;
}

// Parses command line arguments and sets the scheduling policy, job file path, and Round Robin time slice
void parseCommandLineArguments(char **arguments) {
    schedulingPolicy = arguments[1];
    jobFilePath = arguments[2];
    roundRobinTimeSlice = atoi(arguments[3]);
}

// Creates and returns an empty job list
JobList *createJobList() {
    JobList *jobList = malloc(sizeof(JobList));
    jobList->jobCount = 0;
    jobList->firstJob = NULL;
    return jobList;
}

// Adds a new job to the job list with the given ID, length, and priority
void insertJobToList(JobList *jobList, int jobId, int jobLength, int jobPriority) {
    Job *newJob = malloc(sizeof(Job));
    newJob->jobId = jobId;
    newJob->jobLength = jobLength;
    newJob->jobPriority = jobPriority;
    newJob->timeRemaining = jobLength;
    newJob->executionCount = 0;
    newJob->nextJob = jobList->firstJob;
    jobList->firstJob = newJob;
    jobList->jobCount++;
}

// Reads job data from the job trace file and inserts the jobs into the list according to the scheduling policy
void loadJobsFromTraceFile(char *localJobFilePath, JobList *jobList, char policyType) {
    FILE *file = fopen(localJobFilePath, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    int jobIdCounter = 0;
    while (fscanf(file, "%s", inputBuffer) == 1) {
        int jobLength = atoi(inputBuffer);
        if (policyType == 'P') {
            if (fscanf(file, "%s", inputBuffer) != 1) {
                printf("Error reading priority for job %d.\n", jobIdCounter);
                fclose(file);
                return;
            }
            int jobPriority = atoi(inputBuffer);
            insertJobToList(jobList, jobIdCounter, jobLength, jobPriority);
        } else if (policyType == 'S') {
            insertJobToList(jobList, jobIdCounter, jobLength, jobLength);
        } else {
            insertJobToList(jobList, jobIdCounter, jobLength, jobIdCounter);
        }
        jobIdCounter++;
    }
    fclose(file);
}

// Prints the execution order of jobs in the list
void printJobExecutionOrder(JobList *jobList) {
    Job *currentJob = jobList->firstJob;
    while (currentJob != NULL) {
        printf("Job %d ran for: %d\n", currentJob->jobId, currentJob->jobLength);
        currentJob = currentJob->nextJob;
    }
}

// Executes the Round Robin scheduling algorithm on the jobs in the list
void executeRoundRobinQueue(JobList *jobList, int timeSlice) {
    Job *currentJob = jobList->firstJob;
    while (currentJob != NULL) {
        if (currentJob->executionCount == 0) {
            currentJob->firstResponseTime = systemClock;
            currentJob->totalWaitTime = systemClock;
        }

        if (currentJob->timeRemaining > 0) {
            if (currentJob->timeRemaining > timeSlice) {
                printf("Job %d ran for: %d\n", currentJob->jobId, timeSlice);
                systemClock += timeSlice;
                currentJob->executionCount++;
            } else {
                printf("Job %d ran for: %d\n", currentJob->jobId, currentJob->timeRemaining);
                if (currentJob->executionCount != 0) {
                    currentJob->totalWaitTime = systemClock - (currentJob->jobLength - currentJob->timeRemaining);
                }
                currentJob->executionCount++;
                systemClock += currentJob->timeRemaining;
                currentJob->totalTurnaroundTime = systemClock;
                jobList->jobCount--;
                completedJobsCount++;
            }
            currentJob->timeRemaining -= timeSlice;
        }
        currentJob = currentJob->nextJob;
    }
    if (jobList->jobCount > 0) {
        executeRoundRobinQueue(jobList, timeSlice);
    }
}

// Analyzes the results of the Round Robin scheduling algorithm and prints average statistics
void analyzeRoundRobinPolicy(JobList *jobList, int timeSlice) {
    Job *currentJob = jobList->firstJob;
    double totalResponseTime = 0;
    double totalTurnaroundTime = 0;
    double totalWaitTime = 0;
    int totalJobs = 0;

    while (currentJob != NULL) {
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
               currentJob->jobId, currentJob->firstResponseTime, currentJob->totalTurnaroundTime, currentJob->totalWaitTime);

        totalResponseTime += currentJob->firstResponseTime;
        totalTurnaroundTime += currentJob->totalTurnaroundTime;
        totalWaitTime += currentJob->totalWaitTime;
        currentJob = currentJob->nextJob;
        totalJobs++;
    }

    printf("Average -- Response: %0.2lf  Turnaround: %0.2lf  Wait: %0.2lf\n", 
           (totalResponseTime / totalJobs), 
           (totalTurnaroundTime / totalJobs), 
           (totalWaitTime / totalJobs));
}

// Splits a job list into two halves for merge sort
void splitJobListForMergeSort(Job *jobHead, Job **frontRef, Job **backRef) {
    Job *fastPointer = jobHead->nextJob;
    Job *slowPointer = jobHead;

    while (fastPointer != NULL) {
        fastPointer = fastPointer->nextJob;
        if (fastPointer != NULL) {
            slowPointer = slowPointer->nextJob;
            fastPointer = fastPointer->nextJob;
        }
    }

    *frontRef = jobHead;
    *backRef = slowPointer->nextJob;
    slowPointer->nextJob = NULL;
}

// Merges two sorted job lists into one sorted list
Job *mergeSortedJobLists(Job *firstJob, Job *secondJob) {
    Job *result = NULL;
    if (firstJob == NULL) return secondJob;
    if (secondJob == NULL) return firstJob;

    if (firstJob->jobPriority < secondJob->jobPriority || 
        (firstJob->jobPriority == secondJob->jobPriority && firstJob->jobId < secondJob->jobId)) {
        result = firstJob;
        result->nextJob = mergeSortedJobLists(firstJob->nextJob, secondJob);
    } else {
        result = secondJob;
        result->nextJob = mergeSortedJobLists(firstJob, secondJob->nextJob);
    }
    return result;
}

// Sorts a job list using the merge sort algorithm
void mergeSortJobList(Job **jobHeadRef) {
    Job *jobHead = *jobHeadRef;
    Job *firstHalf;
    Job *secondHalf;

    if ((jobHead == NULL) || (jobHead->nextJob == NULL)) {
        return;
    }

    splitJobListForMergeSort(jobHead, &firstHalf, &secondHalf);
    mergeSortJobList(&firstHalf);
    mergeSortJobList(&secondHalf);
    *jobHeadRef = mergeSortedJobLists(firstHalf, secondHalf);
}

// Analyzes the scheduling policy (FIFO, SJF, PRIO) and prints statistics for job execution times
void analyzeSchedulingPolicy(JobList *jobList, char policyType) {
    Job *currentJob = jobList->firstJob;
    int localClock = 0;
    double totalResponseTime = 0;
    double totalTurnaroundTime = 0;
    double totalWaitTime = 0;

    int priorityLevelsCount = 5;
    double priorityResponseTimes[priorityLevelsCount];
    double priorityTurnaroundTimes[priorityLevelsCount];
    double priorityWaitTimes[priorityLevelsCount];
    int priorityJobCounts[priorityLevelsCount];

    for (int i = 0; i < priorityLevelsCount; i++) {
        priorityResponseTimes[i] = 0;
        priorityTurnaroundTimes[i] = 0;
        priorityWaitTimes[i] = 0;
        priorityJobCounts[i] = 0;
    }

    while (currentJob != NULL) {
        currentJob->firstResponseTime = localClock;
        currentJob->totalTurnaroundTime = localClock + currentJob->jobLength;
        currentJob->totalWaitTime = currentJob->firstResponseTime;

        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
               currentJob->jobId, currentJob->firstResponseTime, currentJob->totalTurnaroundTime, currentJob->totalWaitTime);

        localClock += currentJob->jobLength;
        totalResponseTime += currentJob->firstResponseTime;
        totalTurnaroundTime += currentJob->totalTurnaroundTime;
        totalWaitTime += currentJob->totalWaitTime;

        if (policyType == 'P') {
            int priorityIndex = currentJob->jobPriority - 1;
            priorityResponseTimes[priorityIndex] += currentJob->firstResponseTime;
            priorityTurnaroundTimes[priorityIndex] += currentJob->totalTurnaroundTime;
            priorityWaitTimes[priorityIndex] += currentJob->totalWaitTime;
            priorityJobCounts[priorityIndex]++;
        }

        currentJob = currentJob->nextJob;
    }

    if (policyType != 'P') {
        printf("Average -- Response: %0.2lf  Turnaround: %0.2lf  Wait: %0.2lf\n",
               (totalResponseTime / jobList->jobCount),
               (totalTurnaroundTime / jobList->jobCount),
               (totalWaitTime / jobList->jobCount));
    }

    if (policyType == 'P') {
        for (int i = 0; i < priorityLevelsCount; i++) {
            if (priorityJobCounts[i] > 0) {
                printf("Priority %d: Average -- Response: %0.2lf  Turnaround: %0.2lf  Wait: %0.2lf\n",
                       i + 1,
                       priorityResponseTimes[i] / priorityJobCounts[i],
                       priorityTurnaroundTimes[i] / priorityJobCounts[i],
                       priorityWaitTimes[i] / priorityJobCounts[i]);
            }
        }
        printf("Average -- Response: %0.2lf  Turnaround: %0.2lf  Wait: %0.2lf\n",
               (totalResponseTime / jobList->jobCount),
               (totalTurnaroundTime / jobList->jobCount),
               (totalWaitTime / jobList->jobCount));
    }
}

// Schedules jobs using the FIFO scheduling algorithm
void scheduleUsingFIFO(char *localJobFilePath) {
    printf("Execution trace with FIFO:\n");
    JobList *jobList = createJobList();
    loadJobsFromTraceFile(localJobFilePath, jobList, 'F');
    mergeSortJobList(&(jobList->firstJob));
    printJobExecutionOrder(jobList);
    printf("End of execution with FIFO.\n");
    printf("Begin analyzing FIFO:\n");
    analyzeSchedulingPolicy(jobList, 'F');
    printf("End analyzing FIFO.\n");
}

// Schedules jobs using the SJF scheduling algorithm
void scheduleUsingSJF(char *localJobFilePath) {
    printf("Execution trace with SJF:\n");
    JobList *jobList = createJobList();
    loadJobsFromTraceFile(localJobFilePath, jobList, 'S');
    mergeSortJobList(&(jobList->firstJob));
    printJobExecutionOrder(jobList);
    printf("End of execution with SJF.\n");
    printf("Begin analyzing SJF:\n");
    analyzeSchedulingPolicy(jobList, 'S');
    printf("End analyzing SJF.\n");
}

// Schedules jobs using the Round Robin scheduling algorithm
void scheduleUsingRoundRobin(char *localJobFilePath, int timeSlice) {
    printf("Execution trace with RR:\n");
    JobList *jobList = createJobList();
    loadJobsFromTraceFile(localJobFilePath, jobList, 'R');
    mergeSortJobList(&(jobList->firstJob));
    executeRoundRobinQueue(jobList, timeSlice);
    printf("End of execution with RR.\n");
    printf("Begin analyzing RR:\n");
    analyzeRoundRobinPolicy(jobList, timeSlice);
    printf("End analyzing RR.\n");
}

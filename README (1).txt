This project 4 implements a scheduling system for different policies and priority-based scheduling.

The structure for a "Job" has several different fields. The two most important to the relevancy of this project are comparisonKey (which is used for sorting jobs specific to a policy type) and jobPriority (the priority of a Job for reference in priority scheduling).
A secondary structure is JobList.
JobList represents a linked lists of jobs, and contains jobCount (the total number of jobs) and firstJob (a pointer to the first job in the list).

Policies:
FIFO; Jobs are processes in the order they arrive in list, and are executed in the same first-come first-serve order.
The FIFO function traverses the job list and executes each job until the list is gone through.

SJF: Jobs are sorted by their length before execution, which makes the waiting time more efficient and optimized since the shortest jobs are prioritized.

RR: A time slice is assigned and jobs are executed for that slice before a new job is executed.
When a job completes before the time slice ends, it is removed from the queue.

PRIO: Jobs are sorted by "highest-priority". They default to FIFO if there is a "tie" of equal priorities.

Analysis:
The program analyzes the response time, turnaround time, and wait time.
The response time is the time from when a job arrives until it starts execution for the first time.
Turnaround time is the total time from a job's arrival to completion.
Finally, wait time is how long a job is in the queue before it is executed.

The file calculates these values for each policy. For priority scheduling, there is also averages for the priority level.

Jobs are sorted using merge sort. 
The advantage of merge sort is it has O(n log n) time complexity.
When it comes to job scheduling, using merge sort lets me avoid performance issues with large job lists because of its divide and conquer approach.

With the RR queue, recursion is used to ensure that a job is processed within the time slice, and the execution will continue until the entire list of jobs is executed.
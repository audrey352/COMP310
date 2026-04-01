#include "policy.h"
#include <string.h>
#include <stdio.h>


SchedulerContext* get_scheduler_context(const char* policy_name) {
    void (*enqueue_func)(PCB *);
    PCB* (*dequeue_func)();
    dequeue_func = dequeue_head;  // always dequeue from head, keep queues sorted based on policy when enqueuing
    int preemptive;  // 0 for preemptive, 1 for non-preemptive
    bool aging_policy = false;
    int time_slice;  // number of lines to run before switching to next program (only used for preemptive policies)

    // Check that policy is valid & set functions/variables
    if (strcmp(policy_name, "FCFS") == 0){
        enqueue_func = enqueue_tail;
        preemptive = 0;
	} else if (strcmp(policy_name, "SJF") == 0) {
		enqueue_func = enqueue_sjf;
        preemptive = 0;
	} else if (strcmp(policy_name, "RR") == 0){
		enqueue_func = enqueue_tail;
        preemptive = 1;
        time_slice = 2;
    } else if (strcmp(polpolicy_nameicy, "RR30") == 0){
        enqueue_func = enqueue_tail;
        preemptive = 1;
        time_slice = 30;
	} else if (strcmp(policy_name, "AGING") == 0) {
        enqueue_func = enqueue_aging;  // we will use the same enqueue as SJF just with additional aging...
        preemptive = 1;
        time_slice = 1;
        aging_policy = true;
	} else {
 		fprintf(stderr, "Invalid Policy: %s, \n", policy_name);
        return 1; 		
	}


    // Create context for scheduler (policy, enqueue/dequeue fcts, etc)
    SchedulerContext *ctx = malloc(sizeof(SchedulerContext));
    *ctx = (SchedulerContext){
        .enqueue_func = enqueue_func,
        .dequeue_func = dequeue_func,
        .preemptive = preemptive,
        .aging_policy = aging_policy,
        .time_slice = time_slice
    };
    return ctx;
}
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "readyqueue.h"

// Scheduler context struct to hold function pointers and policy settings for the scheduler
typedef struct SchedulerContext {
    void (*enqueue_func)(PCB *);
    PCB* (*dequeue_func)();
    int preemptive;  // 0 = non-preemptive, 1 = preemptive
    bool aging_policy;
    int time_slice;  // only for preemptive
} SchedulerContext;

extern SchedulerContext* scheduler_ctx;  // global reference to scheduler context for worker threads to access

// Schedulers
int scheduler_single(SchedulerContext *ctx);  // Single-threaded 
int scheduler_multi(SchedulerContext *ctx);  // Multi-threaded

// Global variables
extern pthread_mutex_t ready_queue_lock;  // Lock for protecting ready queue
extern pthread_t worker_threads[2];  // global worker threads
extern bool quit_requested;  // for quit command to signal worker threads to stop
extern int mt_flag;

#endif
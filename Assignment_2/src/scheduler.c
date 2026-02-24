#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h> 

#include "scheduler.h"
#include "readyqueue.h"
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"
#include <stdlib.h>

// Initialize mutex and quit_requested variable
pthread_mutex_t ready_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t worker_threads[2];  // global thread handles
bool quit_requested = false;  // signals worker threads to stop when quit is called
int mt_flag = 0;
static bool pool_intialized = false;
struct SchedulerContext *scheduler_ctx = NULL;

// Function to run the single threaded scheduler
int scheduler_single(SchedulerContext *ctx) {
    // Non-preemptive policies (FCFS and SJF)
    if (ctx->preemptive == 0) {
        while (ready_queue.head != NULL) {
            PCB *current_pcb = ctx->dequeue_func(); 
            // Run full program without preemption
            while (current_pcb->program_counter < current_pcb->start + current_pcb->program_length) {
                char *line = get_line(current_pcb->program_counter);
                parseInput(line);  // execute instruction
                current_pcb->program_counter++;  // go to next instruction
            }
            pcb_cleanup(current_pcb);  // remove program from storage and free PCB
        }
    }

    // Preemptive policies (RR, RR30, & AGING)
    else if (ctx->preemptive == 1) {
        while (ready_queue.head != NULL) {
            PCB *current_pcb = ctx->dequeue_func(); 
            int end_of_program = current_pcb->start + current_pcb->program_length;
            int lines_run = 0;  // track how many lines have been executed

            while (lines_run < ctx->time_slice && current_pcb->program_counter < end_of_program) {
                char *line = get_line(current_pcb->program_counter);
                parseInput(line);  // execute instruction
                current_pcb->program_counter++;  // go to next instruction
                lines_run++;
            }

            // AGING policy: Update scores of jobs left in queue
            // all decreasing by 1 so order shouldn't change, enqueue will add back the job that ran in the correct spot
            if (ctx->aging_policy) {
                PCB* queued_pcb = ready_queue.head;
                while (queued_pcb != NULL){
                    update_job_score(queued_pcb);
                    queued_pcb = queued_pcb->next;	
                }
            }

            // Add to queue if program not finished, otherwise clean up PCB and program storage
            if (current_pcb->program_counter < end_of_program) {
                ctx->enqueue_func(current_pcb);
            } else {
                pcb_cleanup(current_pcb);
            }
        }
    }

    return 0;
}


// Worker function for multi-threaded scheduler (only used with RR and RR30 policies)
void* worker_func(void* arg) {
    printf("[THREAD %lu] Started\n", pthread_self());
	SchedulerContext* ctx = (SchedulerContext*) arg;

    while (true) {
        // Lock + dequeue head PCB
        pthread_mutex_lock(&ready_queue_lock);
	printf("[THREAD %lu] Attempting dequeue\n", pthread_self());
        PCB* pcb = ctx->dequeue_func();
	printf("[THREAD %lu] Dequeued PCB %p (pc=%d start=%d len=%d)\n",
       pthread_self(), pcb,
       pcb ? pcb->program_counter : -1,
       pcb ? pcb->start : -1,
       pcb ? pcb->program_length : -1);

        // Check if queue was empty
        if (pcb == NULL) {
	      pthread_mutex_lock(&ready_queue_lock);
		PCB* pcb = ctx->dequeue_func();

            // check if quit was called 
            if (quit_requested) {
                pthread_mutex_unlock(&ready_queue_lock);  // unlock and exit thread
		for (int i = 0; i < 2 ; i++){
			pthread_join(worker_threads[i], NULL);
		}
		free(scheduler_ctx);
                break;  // quit was called, exit thread
            }
            // Otherwise, unlock and wait for queue to be non-empty
            pthread_mutex_unlock(&ready_queue_lock);
            usleep(1000); // small sleep
            continue;
        }
        // Unlock if queue was not empty and we have a PCB to run
        pthread_mutex_unlock(&ready_queue_lock);

        // Run the program for one time slice
        int end_of_program = pcb->start + pcb->program_length;
        int lines_run = 0;  // track how many lines have been executed in this time slice
	
        while (lines_run < ctx->time_slice && pcb->program_counter < end_of_program) {
            char* line = get_line(pcb->program_counter);
            printf("[THREAD %lu] Running line %d (addr=%p)\n",
       	    pthread_self(), pcb->program_counter,
            get_line(pcb->program_counter));
	    parseInput(line);
            pcb->program_counter++;
            lines_run++;
        }

        // Add PCB back to queue if program not finished, otherwise clean up PCB + program storage
        pthread_mutex_lock(&ready_queue_lock);
        if (pcb->program_counter < end_of_program) {
            ctx->enqueue_func(pcb);
        } else {
            pcb_cleanup(pcb);
        }
        pthread_mutex_unlock(&ready_queue_lock);
    }

    return NULL;
}

// Function to run the multi-threaded scheduler
int scheduler_multi(SchedulerContext* ctx) {
	//context will be constant so we want to always keep a global referece 
	//so stack doesn't get rid of it when we still need it.
	if (scheduler_ctx == NULL){
		scheduler_ctx = ctx;
	}
	
	// Check if the threads already exist (ctx is never going to change 
	// because once exec is called the policy never changes- so ctx will not either
	if (!pool_intialized){
    		// Create worker threads that run worker_func
		for (int i = 0; i < 2; i++){
        		pthread_create(&worker_threads[i], NULL, worker_func, scheduler_ctx);
    		}
		pool_intialized = true;
	}



	//Then block until the threads finish
	while (1) {
		pthread_mutex_lock(&ready_queue_lock);
		if (ready_queue.head == NULL){
			pthread_mutex_unlock(&ready_queue_lock);
			break;
		}
		pthread_mutex_unlock(&ready_queue_lock);
		usleep(1000);
	}
	printf("Queue head: %p \n", ready_queue.head);
 	

	
    return 0;
}

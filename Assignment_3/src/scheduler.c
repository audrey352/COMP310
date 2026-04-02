#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h> 
#include <stdlib.h>

#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "shell.h"


// Initialize mutex and quit_requested variable
struct SchedulerContext *scheduler_ctx = NULL;
pthread_mutex_t ready_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t worker_threads[2];  // global thread handles
bool quit_requested = false;  // signals worker threads to stop when quit is called
int mt_flag = 0;
static bool pool_initialized = false;


// Helper functions
// Returns 1 if page is in memory, 0 if not. Sets *frame_out if in memory
static int check_page_in_memory(PCB *pcb, int page_number, int *frame_out) {
    int frame = pcb->page_table[page_number];  // frame number
    if (frame == -1) return 0;  // not in memory
    *frame_out = frame;  // set frame_out if in memory
    return 1;
}

// Loads the page into memory, updates PCB page table
static int handle_page_fault(PCB *pcb, int page_number) {
    printf("Page fault! ");         
    // load page into memory (will evict a page if memory is full)
    int frame;
    if (load_page(pcb->prog_name, page_number, pcb->page_table) != 0) {
        return 1;  // error with file
    }
    printf("\n");
    return 0;
}

// Execute the line of the program given by it's program counter
// this function assumes the page is already in memory (should check before calling)
static void execute_current_line(PCB *pcb) {
    global_clock++;
    //printf("Global clock incremented at: %ld \n", global_clock);
    int pc = pcb->program_counter;
    int page = pc / FRAME_SIZE;  // page number in the program
    int offset = pc % FRAME_SIZE;  // line within page
    int frame = pcb->page_table[page];  // frame number in memory
    int mem_index = frame * FRAME_SIZE + offset;  // index of the line in program storage

    char *line = get_line(mem_index);
    if (line[0] != '\0') {  // skip padded lines (added when program doesn't fill a whole frame)
        parseInput(line);  // execute instruction
    }
    pcb->program_counter++;  // update program counter to next line
    all_frames[frame].time_stamp = global_clock; //update the time for when the frame was last accesed
    //printf("Frame timestamp updated to: %ld \n", all_frames[frame].time_stamp);
}

// Run the pcb until a page fault or we reach the max number of lines to run
// for non-preemptuve, max_mun_lines is the program length, for preemptive it's the time slice
static int run_pcb(PCB *pcb, int max_num_lines) {
    int lines_run = 0;  // to track how many lines have been executed in this time slice

    while (pcb->program_counter < pcb->program_length && lines_run < max_num_lines) {
        int pc = pcb->program_counter;
        int page = pc / FRAME_SIZE;  // page number in the program
        int frame; 

        if (!check_page_in_memory(pcb, page, &frame)) {  // page not in memory -> page fault
            int res = handle_page_fault(pcb, page);
            if (handle_page_fault(pcb, page) != 0) return -1;  // error with file
            return 1;  // stop running this program due to page fault (need to put pcb back in queue)
        }

        execute_current_line(pcb);  // execute the line and update program counter
        lines_run++;
    }
    return 0;  // finished running program (either fully or time slice up)
}


// Function to run the single threaded scheduler with demand paging
int scheduler_single(SchedulerContext *ctx) {
    // Non-preemptive policies (FCFS and SJF)
    if (ctx->preemptive == 0) {
        while (ready_queue.head != NULL) {
            // Get pcb to run
            PCB *current_pcb = ctx->dequeue_func(); 
            int end_of_program = current_pcb->program_length;

            int result = run_pcb(current_pcb, end_of_program);  // run until page fault or end of program
            if (result == 1) {
                ctx->enqueue_func(current_pcb);  // put back in queue if page fault
            } else if (result == -1) {
                return 1;  // error with file
            } // if result = 0, program finished so don't add back to queue
        }
    }

    // Preemptive policies (RR, RR30, & AGING)
    else if (ctx->preemptive == 1) {
        while (ready_queue.head != NULL) {
            // Get pcb to run
            PCB *current_pcb = ctx->dequeue_func();
            int end_of_program = current_pcb->program_length;
            int time_slice = ctx->time_slice;

            // run until page fault, end of program, or time slice is up
            int result = run_pcb(current_pcb, time_slice);  // run until page fault or time slice up
            if (result == -1) {
                return 1;  // error with file
            }
            else if (result == 0 && ctx->aging_policy) {  // slice done running -> update job scores for rest of queue if AGING policy
                // AGING policy: Update scores of jobs left in queue
                // all decreasing by 1 so order shouldn't change, enqueue will add back the job that ran in the correct spot
                PCB* queued_pcb = ready_queue.head;
                while (queued_pcb != NULL){
                    update_job_score(queued_pcb);
                    queued_pcb = queued_pcb->next;	
                }
            }  

            // Add back to queue if program not finished (after page fault/done time slice)
            if (current_pcb->program_counter < end_of_program) {
                ctx->enqueue_func(current_pcb);
            } 
        }
    }
    return 0;
}



// Worker function for multi-threaded scheduler (only used with RR and RR30 policies)
// NOT UPDATED FOR DEMAND PAGING 
void* worker_func(void* arg) {
	SchedulerContext* ctx = (SchedulerContext*) arg;

    while (true) {
        // Lock + dequeue head PCB
        pthread_mutex_lock(&ready_queue_lock);
        PCB* pcb = ctx->dequeue_func();

        // Check if queue was empty
        if (pcb == NULL) {
            // check if quit was called 
            if (quit_requested) {
                pthread_mutex_unlock(&ready_queue_lock);
                break;  // quit was called, exit thread (go to return NULL below)
            }
            // Otherwise, unlock and wait for queue to be non-empty
            pthread_mutex_unlock(&ready_queue_lock);
            usleep(1000); // small sleep to wait for queue to fill
            continue;
        }
        // Unlock if queue was not empty and we have a PCB to run
        pthread_mutex_unlock(&ready_queue_lock);

        // Run the program for one time slice
        int end_of_program = pcb->program_length;
        int lines_run = 0;  // track how many lines have been executed in this time slice
	
        while (lines_run < ctx->time_slice && pcb->program_counter < end_of_program) {
            char* line = get_line(pcb->program_counter);
	        parseInput(line);
            pcb->program_counter++;
            lines_run++;
        }

        // Add PCB back to queue if program not finished
        pthread_mutex_lock(&ready_queue_lock);
        if (pcb->program_counter < end_of_program) {
            ctx->enqueue_func(pcb);
        } 
        pthread_mutex_unlock(&ready_queue_lock);
    }
    return NULL;
}

// Function to run the multi-threaded scheduler
int scheduler_multi(SchedulerContext* ctx) {
	// Check if the threads already exist
	if (!pool_initialized){
        // set context only if not already initialized
        //context will be constant so we want to always keep a global reference 
	    //so stack doesn't get rid of it when we still need it.
        scheduler_ctx = ctx;

    	// Create worker threads that run worker_func
		for (int i = 0; i < 2; i++){
        	pthread_create(&worker_threads[i], NULL, worker_func, scheduler_ctx);
    	}
		pool_initialized = true;
	}
    return 0;
}

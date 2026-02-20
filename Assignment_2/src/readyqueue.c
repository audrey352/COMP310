#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "readyqueue.h"
#include "shellmemory.h"

// Function to initialize a new PCB
PCB* create_pcb(int program_start, int program_length) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->PID = next_pid++;
    pcb->start = program_start;
    pcb->program_counter = program_start;  // start executing at the first instruction
    pcb->program_length = program_length;
    pcb->next = NULL;
    pcb->job_score = program_length;
    return pcb;
}

// Function to clean up a PCB and free its resources
// NOT removing from queue (should be done beforehand)
int pcb_cleanup(PCB *pcb) {
    // free the lines of the program from storage
    for (int i = pcb->start; i < pcb->start + pcb->program_length; i++) {
        free(program_storage[i]);
        program_storage[i] = NULL;
    }
    free(pcb);
    return 0;
}


// Initializing global ready queue 
ReadyQueue ready_queue = {NULL, NULL};


// Enqueue functions
void enqueue_tail(PCB *pcb) {
    // if queue empty, head and tail both point to the new PCB
    if (ready_queue.tail == NULL && ready_queue.head == NULL) {
        ready_queue.head = pcb;
        ready_queue.tail = pcb;
    } 
    // otherwise, add to end of queue and update tail
    else {
        ready_queue.tail->next = pcb;
        ready_queue.tail = pcb;
    }
    return;
}

void enqueue_sjf(PCB *pcb) {
    int pcb_length = pcb->program_length;
    PCB *current = ready_queue.head;
    PCB *previous = NULL;

    // find the first PCB in the queue with a longer program length
    // if equal length, insert after
    while (current != NULL && current->program_length <= pcb_length) {
        previous = current;
        current = current->next;
    }

    // Insert new PCB between previous and current 
    pcb->next = current;
    if (previous == NULL) {
        ready_queue.head = pcb;  // inserting at head
    } 
    else {
        previous->next = pcb;  // link previous to new PCB
    }
    if (current == NULL) ready_queue.tail = pcb;  // inserting at tail

    return;
}



// Dequeue functions
PCB* dequeue_head() {
    // queue is empty --> error
    if (ready_queue.head == NULL) return NULL;
    
    // update head and next pointer
    PCB *pcb = ready_queue.head;
    ready_queue.head = pcb->next;
    pcb->next = NULL;  // disconnect dequeued PCB

    // if queue is now empty, update tail to NULL
    if (ready_queue.head == NULL) ready_queue.tail = NULL;

    return pcb;  // return dequeued PCB
}

//Helper function for updating the job score
int update_job_score(PCB *pcb){
	int err_code = 0;
	int prev_job_score = pcb->job_score;
	if (prev_job_score < 0){
	       err_code = -1; //It should never be less than 0 so return an error code if so
	} else if (prev_job_score > 0){
		pcb->job_score = prev_job_score - 1; //If it's greater than 0, subtract 1.
	}
	//Otherwise do nothing (keep at 0).
	return err_code;
}

		



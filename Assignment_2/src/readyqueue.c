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

    return pcb;
}

// Initializing global ready queue 
ReadyQueue ready_queue = {NULL, NULL};

// Enqueue and Dequeue functions for ready queue
void enqueue(PCB *pcb) {
    // if queue empty, head and tail both point to the new PCB
    if (ready_queue.tail == NULL) {
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

PCB* dequeue() {
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
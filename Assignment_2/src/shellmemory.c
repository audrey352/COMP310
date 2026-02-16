#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}

//Initaliaze a big array to store all programs
char *program_storage[MAX_STORAGE_LINES];
int program_index = 0;
int next_pid = 1;  // global counter for assigning unique PIDs

//Add line allows you to add lines to the program.
//Before adding the line, it ensures that the index does not exceed max 
//number of lines allowed (returns -1 for error if that is the case)
int add_line(char *line) {
	if (program_index ==  MAX_STORAGE_LINES){
		return -1;
	}
	program_storage[program_index] = strdup(line);
	return program_index++;
}


//get line simply returns the next line in the array
char* get_line(int index){
	return program_storage[index];
}

//load_program will attempt to write to the array until either there are 
//no more lines to write or the array runs out of space. It returns where
//the program started as well.  
int load_program(FILE* f, int* length_out){
	int program_start = program_index;
	int count = 0;
	int space = 1;
	char buffer[MAX_LINE_LENGTH];
	
	while (fgets(buffer, MAX_LINE_LENGTH, f) && space > 0){
		space = add_line(buffer);
		count++;
	}
	*length_out = count;

	return program_start;
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

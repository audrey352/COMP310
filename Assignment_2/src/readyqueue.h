#ifndef READYQUEUE_H
#define READYQUEUE_H

typedef struct pcb {
	int PID;
	int start;  // index of the first instruction of the program in program storage
	int program_counter;  // index of the next instruction to execute in program storage
	int program_length;  // number of lines in the program
    struct pcb* next;  // pointer to the next PCB in the ready queue
} PCB;

typedef struct ready_queue {
    PCB *head;  // pointer to first PCB in the queue
    PCB *tail;  // pointer to last PCB in the queue
} ReadyQueue;

extern ReadyQueue ready_queue;  // global ready queue

PCB* create_pcb(int program_start, int program_length);
void enqueue(PCB *pcb);
PCB* dequeue();


#endif
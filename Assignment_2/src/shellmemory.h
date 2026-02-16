#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#define MEM_SIZE 1000
#define MAX_STORAGE_LINES 1000
#define MAX_LINE_LENGTH 1000

extern char *program_storage[MAX_STORAGE_LINES];
extern int program_index;
extern int next_pid;

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
int load_program(FILE *f, int* length_out);

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

extern ReadyQueue ready_queue;

void enqueue(PCB *pcb);
PCB* dequeue();

#endif

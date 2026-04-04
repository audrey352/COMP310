#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
#include <stdbool.h>

// #define VAR_MEM_SIZE 1000  // max number of variables we can store in memory
// #define FRAME_STORE_SIZE 999 // total size of program storage 
// ^^ are defined in the makefile! 
#define FRAME_SIZE 3  // number of lines per frame
#define NUM_FRAMES (FRAME_STORE_SIZE / FRAME_SIZE)
#define MAX_LINE_LENGTH 1000  // max number of characters in a line

struct var_memory_struct {  // struct for variable memory
    char *var;
    char *value;
};

struct Frame {  // struct for metadata of each frame in program storage
    int valid;  // 0 = free, 1 = used, -1 = uninitialized
    int page_number;  // which page is stored in this frame? 
    char* prog_name;  // name of the program that owns this frame (used to open the correct file)
    int *page_table;   // pointer to PCB's page table
    long time_stamp; // stores the last time the frame was used (0 if never)
};

extern struct Frame all_frames[NUM_FRAMES];
extern char *program_storage[FRAME_STORE_SIZE];
extern int next_pid;
extern long global_clock; // keeps track of how long the program has been running

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
struct Frame create_frame(char* prog_name, int page_number, int* page_table);
int add_frame(char *lines[], char* prog_name, int page_number, int* page_table);
int replace_page(char* new_prog_name, int new_page_number, int* new_page_table);
int load_page(char* filename, int page_number, int* page_table);
int load_init(char* filename, int* page_table, int num_pages_total);
int compute_program_length(char* filename, int* length_out, int* num_pages_out);

#endif

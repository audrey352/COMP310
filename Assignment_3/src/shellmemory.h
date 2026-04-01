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
    char* prog_name;  // name of the program that owns this frame (used to open the correct file)
    int page_number;  // which page is stored in this frame? 
};

extern struct Frame all_frames[NUM_FRAMES];
extern char *program_storage[FRAME_STORE_SIZE];
extern int next_pid;

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
struct Frame create_frame(char* prog_name, int page_number);
int add_frame(char *lines[], char* prog_name, int page_number, int* frame_number_out);
int load_next_page(char* filename, int page_number, int* frame_number_out);
int load_init(char* filename, int* length_out, int* page_table);


// int clean_frames(int length, int* page_table);
// int load_program(char *script, int* length_out, int* page_table);
// int load_program_file(FILE *fp, int* length_out, int* page_table);

#endif

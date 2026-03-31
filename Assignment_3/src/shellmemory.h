#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
#include <stdbool.h>
#define MEM_SIZE 1000  // max number of variables we can store in memory
#define MAX_STORAGE_FRAMES 334  // max number of frames we can store in memory
#define FRAME_SIZE 3  // number of lines per frame
#define MAX_LINE_LENGTH 1000  // max number of characters in a line of a program

struct memory_struct {  // struct for variable memory
    char *var;
    char *value;
};

extern char *program_storage[MAX_STORAGE_FRAMES * FRAME_SIZE];
extern bool valid_store[MAX_STORAGE_FRAMES];
extern int storage_size;
extern int next_pid;

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
int clean_frames(int length, int* page_table);
int load_program(char *script, int* length_out, int* page_table);
int load_program_file(FILE *fp, int* length_out, int* page_table);

#endif

#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#define MEM_SIZE 1000
#define MAX_STORAGE_FRAMES 1002
#define FRAME_SIZE 3
#define MAX_LINE_LENGTH 1000

extern char *program_storage[MAX_STORAGE_FRAMES * FRAME_SIZE];
extern int program_index;
extern int next_pid;

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
int load_program(char *script, int* length_out, int* start_out);
int load_program_file(FILE *fp, int* length_out, int* start_out);
#endif

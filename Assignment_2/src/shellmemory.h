#define MEM_SIZE 1000
#define MAX_STORAGE_LINES 1000
#define MAX_LINE_LENGTH 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int add_line(char *line);
char* get_line(int index);
int load_program(FILE *f);

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

// Get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}

// Initialize a big array to store all programs
char *program_storage[MAX_STORAGE_FRAMES * FRAME_SIZE];
int program_index = 0;  // tracks where the next empty line is in program storage
int next_pid = 1;  // global counter for assigning unique PIDs


//REPLACED ADD LINE WITH ADD FRAME:
//Add line allows you to add lines to the program.
//Before adding the line, it ensures that the index does not exceed max 
//number of lines allowed (returns -1 for error if that is the case)
int add_frame(char *line[]) {
	if (program_index + 3 ==  MAX_STORAGE_FRAMES * FRAME_SIZE){
		return -1;
	}
	
	program_storage[program_index] = strdup(line[0]);
	program_storage[program_index+1] = strdup(line[1]);
	program_storage[program_index+2] = strdup(line[2]);
	program_index += 3;
	return program_index;
}

// Get line simply returns the next line in the array
char* get_line(int index){
	return program_storage[index];
}


//og function that was renamed to take input from stream
// Load program line by line into memory. 
// Returns 0 on success, and 1 if the file doesn't exist or the program is too long to fit in memory. 
int load_program_file(FILE* f, int* length_out, int* start_out) {
    // Make sure file exists
    if (f == NULL) return 1;

    // program info 
    *start_out = program_index;
	int count = 0;
	char buffer[MAX_LINE_LENGTH];
	
	char* newFrame[3]; 
	int line_in_frame = 0; 
    // read lines until end of file
	while (fgets(buffer, MAX_LINE_LENGTH, f) != NULL){
		
		newFrame[line_in_frame] = strdup(buffer);
		line_in_frame++;

		if (line_in_frame == 3){
			if (add_frame(newFrame) < 0){
				//out of space
				fclose(f);
				return 1;
			}
			line_in_frame = 0;
			count += 3;
		}
	}

	//If we are in the middle of frames
	if (line_in_frame > 0){
		for (int i = line_in_frame ; i < 3 ; i++){
		       newFrame[i] = strdup("");
		}
 		add_frame(newFrame);
		count += 3;		
	}

    *length_out = count;
	return 0;
}

// wrapper function that can load from stream (implemented for batch mode)
int load_program(char* filename, int* length_out, int* start_out){
	
	
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return 1;
	

	int result = load_program_file(fp, length_out, start_out);
	if (result == 1){
		fprintf(stderr, "Program too long to fit in memory: %s\n", filename);
	}

	fclose(fp);
	return result;
}

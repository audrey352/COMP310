#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
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
bool valid_store[MAX_STORAGE_FRAMES] = {false};
int program_index = 0;  // tracks where the next empty line is in program storage
int next_pid = 1;  // global counter for assigning unique PIDs


//REPLACED ADD LINE WITH ADD FRAME:
//Add line allows you to add lines to the program.
//Before adding the line, it ensures that the index does not exceed max 
//number of lines allowed (returns -1 for error if that is the case)
int add_frame(char *line[]) {
	int free_page = 0;
	for (free_page; free_page < MAX_STORAGE_FRAMES ; free_page++){
		if ( !valid_store[free_page] ){
			//insert lines
			program_storage[free_page*3] = strdup(line[0]);
			program_storage[free_page*3+1] = strdup(line[1]);
			program_storage[free_page*3+2] = strdup(line[2]);
			//set bits and return
			valid_store[free_page] = 1;
			printf("Found free page at: %d \n", free_page);
			return free_page;
		}
	}

	return -1;
}

// Get line simply returns the next line in the array
char* get_line(int index){
	return program_storage[index];
}

int clean_frames(int length, int* page_table){
	for (int i = 0 ; i < length; i++){
		int index = page_table[i];
		valid_store[index] = false;
	}
	return 0;
}

//og function that was renamed to take input from stream
// Load program line by line into memory. 
// Returns 0 on success, and 1 if the file doesn't exist or the program is too long to fit in memory. 
int load_program_file(FILE* f, int* length_out, int* start_out, int* page_table) {
    // Make sure file exists
    if (f == NULL) return 1;

    // program info 
    *start_out = program_index;
	int count = 0;
	char buffer[MAX_LINE_LENGTH];
	
	char* newFrame[3]; 
	int line_in_frame = 0;
       	int page_number = -1;	
    // read lines until end of file
	while (fgets(buffer, MAX_LINE_LENGTH, f) != NULL){
		
		newFrame[line_in_frame] = strdup(buffer);
		line_in_frame++;

		if (line_in_frame == 3){
			page_number = add_frame(newFrame);
			if (page_number < 0){
				//out of space
				fclose(f);
				return 1;
			}
			page_table[count/3] = page_number; //insert the page number in page table
			line_in_frame = 0;
			count += 3;
			printf("Added to page table: %d \n", page_number);
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
int load_program(char* filename, int* length_out, int* start_out, int* page_table){
	
	
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return 1;
	

	int result = load_program_file(fp, length_out, start_out, page_table);
	if (result == 1){
		fprintf(stderr, "Program too long to fit in memory: %s\n", filename);
	}

	fclose(fp);
	return result;
}

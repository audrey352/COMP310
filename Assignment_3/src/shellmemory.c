#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "shellmemory.h"


// Initialize variable memory
struct memory_struct shellmemory[VAR_MEM_SIZE];  // this is our variables store

// Initialize program storage (single array to store all frames)
// ** FRAME_STORE_SIZE is the total number of lines in prog storage. it's defined in the makefile!
char *program_storage[FRAME_STORE_SIZE];  // this is our frame store (frame0 = lines 0-FRAME_SIZE, etc)
bool valid_store[NUM_FRAMES] = {false};  // array to keep track of which frames are in use (1 if frame is in use, 0 if free))
int next_pid = 1;  // global counter for assigning unique PIDs


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
    for (i = 0; i < VAR_MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < VAR_MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < VAR_MEM_SIZE; i++) {
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

    for (i = 0; i < VAR_MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}


// Find first free frame in memory and add the lines of the program to that frame.
// Returns -1 if memory is full, otherwise returns the frame number where the program was added.
int add_frame(char *line[]) {
	int free_frame_nb = 0;
	// printf("looking for a free frame...\n");
	for (free_frame_nb; free_frame_nb < NUM_FRAMES ; free_frame_nb++){
		// find first free frame in memory
		if (!valid_store[free_frame_nb]){
			// printf("Found free frame %d \n", free_frame_nb);
			// printf("Adding lines in storage at indices %d, %d, and %d \n", free_frame_nb*FRAME_SIZE, free_frame_nb*FRAME_SIZE+1, free_frame_nb*FRAME_SIZE+2);
			// insert lines
			program_storage[free_frame_nb*FRAME_SIZE] = strdup(line[0]);
			program_storage[free_frame_nb*FRAME_SIZE+1] = strdup(line[1]);
			program_storage[free_frame_nb*FRAME_SIZE+2] = strdup(line[2]);
			// for (int i = 0; i < FRAME_SIZE; i++){
			// 	printf("Added to storage: %s", program_storage[free_frame_nb*FRAME_SIZE+i]);
			// }
			// if (line[2][strlen(line[2])-1] != '\n') {
			// 	printf("\n");
			// }

			// set bit to valid & return frame number 
			valid_store[free_frame_nb] = 1;
			return free_frame_nb;
		}
	}
	return -1;
}

// Get line simply returns the next line in the array
char* get_line(int index){
	return program_storage[index];
}

// Clean frames used by a program (used when program finishes or is killed)
int clean_frames(int length, int* page_table){
	for (int i = 0 ; i < length; i++){
		int index = page_table[i];
		valid_store[index] = false;
	}
	return 0;
}

// Load program line by line into memory using frames. 
// Returns 0 on success, and 1 if the file doesn't exist or the program is too long to fit in memory. 
int load_program_file(FILE* f, int* length_out, int* page_table) {
    // Make sure file exists
    if (f == NULL) return 1;

    // program info 
	int line_count = 0;  // total number of lines read in so far
	char buffer[MAX_LINE_LENGTH];
	
	// frame info
	char* newFrame[FRAME_SIZE];  // 3 lines per frame
	int line_in_frame = 0;  // number of lines read into the current frame so far
    int frame_number = -1;	// frame number in program storage 

	// printf("Loading program into memory... \n");

    // read lines until end of file
	while (fgets(buffer, MAX_LINE_LENGTH, f) != NULL){
		
		newFrame[line_in_frame] = strdup(buffer);  // copy line into frame
		// printf("Reading line: %s", buffer);
		// if (buffer[strlen(buffer)-1] != '\n') {
		// 	printf("\n");
		// }
		line_in_frame++;

		// if frame is full, add to memory and update page table
		if (line_in_frame == FRAME_SIZE){
			frame_number = add_frame(newFrame);
			// printf("Lines successfully added to frame %d \n", frame_number);
			if (frame_number < 0){  // out of space
				fprintf(stderr, "Program too long to fit in memory\n");
				fclose(f);
				return 1;
			}
			// set page_table entry for this frame (eg. page0 = frame8 in memory)
			int page_number = line_count/FRAME_SIZE;
			page_table[page_number] = frame_number; // insert the frame number in page table (of this program)
			line_in_frame = 0;
			line_count += FRAME_SIZE;
			// printf("Done adding to page table. Mapping page %d to frame %d \n", page_number, frame_number);
		}
	}

	// If we were in the middle of a frame when we hit the end of the file...
	if (line_in_frame > 0){
		// printf("End of file reached, adding empty lines to frame... \n");

		// fill rest of frame with empty strings
		for (int i = line_in_frame ; i < FRAME_SIZE ; i++){
		    newFrame[i] = strdup("");
		}
		// add to memory and update page table
		frame_number = add_frame(newFrame);
		if (frame_number < 0){  // out of space
			fclose(f);
			return 1;
		}
		int page_number = line_count/FRAME_SIZE;
		page_table[page_number] = frame_number;  
		line_count += line_in_frame;  // only count the actual code lines, not the padded empty lines
		// printf("Added padded frame to page table: %d \n", frame_number);
	}

    *length_out = line_count;
	return 0;
}

// wrapper function that can load from stream (implemented for batch mode)
int load_program(char* filename, int* length_out, int* page_table){
	
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return 1;
	
	// printf("(load_program) Loading program: %s \n", filename);
	int result = load_program_file(fp, length_out, page_table);
	if (result == 1){
		fprintf(stderr, "Program too long to fit in memory: %s\n", filename);
	}
	// printf("Finished loading program (return %d): %s \n", result, filename);

	fclose(fp);
	return result;
}

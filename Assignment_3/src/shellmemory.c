#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "shellmemory.h"


// Initialize variable memory
struct var_memory_struct shellmemory[VAR_MEM_SIZE];  // this is our variables store

// Initialize program storage (single array to store all frames)
// ** FRAME_STORE_SIZE is the total number of lines in prog storage. it's defined in the makefile!
char *program_storage[FRAME_STORE_SIZE];  // this is our frame store (frame0 = lines 0-FRAME_SIZE, etc)
struct Frame all_frames[NUM_FRAMES] = {-1};  // array containing all frame structs (keeps track of which process and page is stored in each frame)
int next_pid = 1;  // global counter for assigning unique PIDs (used when creating pcbs)


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

// Get line simply returns the next line in the array
char* get_line(int index){
	return program_storage[index];
}


// Function to initialize a new Frame struct
struct Frame create_frame(int pids[], int num_pids, int page_number) {
	struct Frame frame;
	frame.valid = 1;  // mark frame as used
	frame.num_pids = num_pids;

	// copy PIDs into the frame
	for (int i = 0; i < num_pids; i++)
        frame.process_pid[i] = pids[i];
	// fill remaining slots
	for (int i = num_pids; i < 3; i++)
		frame.process_pid[i] = -1;

	frame.page_number = page_number;
	return frame;
}


// Find first free frame in memory and add the lines to it.
// Returns -1 if memory is full, otherwise returns the frame number where the program was added.
int add_frame(char *lines[], int pid, int page_number, int* frame_number_out) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (all_frames[i].valid != 1) {  // found free frame
            // copy lines into frame storage
			for (int j = 0; j < FRAME_SIZE; j++) {
				if (lines[j] != NULL) {
					program_storage[i * FRAME_SIZE + j] = strdup(lines[j]);
				} else {
					program_storage[i * FRAME_SIZE + j] = NULL; // pad remaining slots in case end of file
				}
			}

			// Create or update frame struct for this frame
            if (all_frames[i].valid == -1) {  // frame has not been initialized
                int pids[1] = { pid };  // use array to pass to create frame
                all_frames[i] = create_frame(pids, 1, page_number);
            } else {  // frame already exists -> update fields
                all_frames[i].valid = 1;
                all_frames[i].process_pid[0] = pid;
                all_frames[i].num_pids = 1;
                all_frames[i].page_number = page_number;  // unique
            }

            *frame_number_out = i;  // output the frame number where the page was added
            return 0;
        }
    }
    return 1; // memory full IMPLEMENT REPLACING POLICY LATER
}


// Function to load a single page from a file into memory
int load_next_page(FILE* f, int pid, int page_number, int* frame_number_out){
	// Make sure file exists
    if (f == NULL) return 1;

	// program & frame info 
	char buffer[MAX_LINE_LENGTH];  // buffer to read lines into
	char* lines[FRAME_SIZE];  // to hold the lines we want to load in memory

	// Read the next 3 lines into buffer (starts at current position of f)
	for (int i = 0; i < FRAME_SIZE; i++){
		if (fgets(buffer, MAX_LINE_LENGTH, f) != NULL){
			lines[i] = strdup(buffer);  // copy line into frame
		} else {
			lines[i] = NULL;  // pad remaining slots in case end of file
		}
	}

	// Add lines to memory
	int result = add_frame(lines, pid, page_number, frame_number_out);  // if successful, result = frame number
	if (result == 1){
		fprintf(stderr, "Memory full\n");
		return 1;
	}
	return 0;
} 


// Function to load the first 2 pages of a program into memory 
int load_init(FILE* f, int pid, int* length_out, int* page_table){
	// check if file exists
	if (f == NULL) return 1;

	// Count total lines in the file (for program length)
    int total_lines = 0;
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, f) != NULL) {
        total_lines++;
    }
    int total_pages = (total_lines + FRAME_SIZE - 1) / FRAME_SIZE;  // round up
	*length_out = total_lines;
    rewind(f);  // go back to start of file for loading

	// Load first 2 pages (or fewer if program is shorter) into memory
	int frame_number;
	for (int page_number = 0; page_number < 2 && page_number < total_pages; page_number++) {
		if (load_next_page(f, pid, page_number, &frame_number) != 0) {
			fprintf(stderr, "Memory full or failed to load page %d\n", page_number);
			return 1;
		}
		page_table[page_number] = frame_number;  // store frame number in page table
	}

	return 0;
}




/*
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

// wrapper function that can load from stream 
int load_program(char* filename, int* length_out, int* page_table){
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return 1;
	
	// printf("(load_program) Loading program: %s \n", filename);
	int result = load_program_file(fp, length_out, page_table);  // loads 2 pages
	if (result == 1){
		fprintf(stderr, "Program too long to fit in memory: %s\n", filename);
	}
	// printf("Finished loading program (return %d): %s \n", result, filename);

	fclose(fp);
	return result;
}

*/
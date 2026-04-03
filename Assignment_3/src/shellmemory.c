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

//Frames keep track of all the frame in storage
struct Frame all_frames[NUM_FRAMES];
//Initialized all frames to -1 for frame not initialized

int next_pid = 1;  // global counter for assigning unique PIDs (used when creating pcbs)

//Initialize the global clock
long global_clock = 0;

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
    for (i = 0 ; i < NUM_FRAMES ; i++){
	    all_frames[i].valid = -1;
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
struct Frame create_frame(char* prog_name, int page_number, int* page_table) {
	struct Frame frame;
	frame.valid = 1;  // mark frame as used
	frame.prog_name = strdup(prog_name);  // store program name
	frame.page_number = page_number;  // store new page number
	frame.page_table = page_table;  // store pointer to PCB's page table
	frame.time_stamp = 0;
	return frame;
}


void free_frame_storage(int frame_index) {
    int start = frame_index * FRAME_SIZE;
	// free the lines in the frame storage and set to NULL
    for (int i = 0; i < FRAME_SIZE; i++) {
        if (program_storage[start + i] != NULL) {
            free(program_storage[start + i]);
            program_storage[start + i] = NULL;
        }
    }
	// free the prog_name in the frame struct and set to NULL
    if (all_frames[frame_index].prog_name != NULL) {
        free(all_frames[frame_index].prog_name);
        all_frames[frame_index].prog_name = NULL;
    }
}

//Helper function to find LRU page
int find_least_recently_used(void){
	int LRU = 0;
	for (int i = 0; i < NUM_FRAMES; i++){
		if (all_frames[i].time_stamp < all_frames[LRU].time_stamp){
			LRU = i;
		}
	}
	return LRU;
}

// Function to evict a page from memory and replace it with a new page
int replace_page(char* new_prog_name, int new_page_number, int* new_page_table) {
	// pick a victim page to evict (randomly pick a frame that is currently in use)
	//int victim_frame = rand() % NUM_FRAMES;  // NEED TO CHANGE BASED ON EVICTION POLICY
	//Based on evicition policy
	
	int victim_frame = find_least_recently_used();
	//printf("Choose page: %d \n", victim_frame);	

	// print the contents of the victim page
	printf("Victim page contents: \n\n");
	int victim_start = victim_frame * FRAME_SIZE;
	for (int i = 0; i < FRAME_SIZE; i++) {
		char *line = program_storage[victim_start + i];
		if (line != NULL) {
			printf("%s", line);
		}
	}
	printf("\nEnd of victim page contents.");

	// update old page table & free frame memory
	int old_page_number = all_frames[victim_frame].page_number;
	int* old_page_table = all_frames[victim_frame].page_table;
	old_page_table[old_page_number] = -1;  // mark page as not in memory
	free_frame_storage(victim_frame);

	// update new page table and frame metadata with new page info
	new_page_table[new_page_number] = victim_frame;
	all_frames[victim_frame].prog_name = strdup(new_prog_name);
	all_frames[victim_frame].page_number = new_page_number;
	all_frames[victim_frame].page_table = new_page_table;
	all_frames[victim_frame].time_stamp = global_clock;

	return victim_frame;
}


// Find first free frame in memory and add the lines to it, otherwise evicts a page.
// Also updates page table with the new frame number where the page is stored in memory.
// returns 0 if found free space, 1 if memory was full and a page had to be evicted to make space.
int add_frame(char *lines[], char* prog_name, int page_number, int* page_table) {
	//printf("Number of frames is: %d \n", NUM_FRAMES);   
	for (int i = 0; i < NUM_FRAMES; i++) {
	//printf("Frame at: %d is %d \n", i, all_frames[i].valid);
        if (all_frames[i].valid != 1) {  // found free frame
            // copy lines into frame storage
	    printf("Found a free frame at: %d \n", i);
			for (int j = 0; j < FRAME_SIZE; j++) {
				if (lines[j] != NULL) {
					program_storage[i * FRAME_SIZE + j] = strdup(lines[j]);
				} else {
					program_storage[i * FRAME_SIZE + j] = NULL; // pad remaining slots in case end of file
				}
			}
			// Create or update frame struct
            if (all_frames[i].valid == -1) {  // frame has not been initialized
                all_frames[i] = create_frame(prog_name, page_number, page_table);
            } else {  // frame already exists -> update fields
                all_frames[i].valid = 1;
                all_frames[i].prog_name = strdup(prog_name);
                all_frames[i].page_number = page_number;
		all_frames[i].time_stamp = global_clock;
		all_frames[i].page_table = page_table;
            }
	page_table[page_number] = i;  // update page table with frame number where page is stored    
       	return 0;
        }
    }
	printf("Finding a victim frame \n");
	// memory is full, need to evict a page to make space
	int victime_frame = replace_page(prog_name, page_number, page_table);
	// add to storage after evicting
	for (int j = 0; j < FRAME_SIZE; j++) {
		if (lines[j] != NULL) {
			program_storage[victime_frame * FRAME_SIZE + j] = strdup(lines[j]);
		} else {
			program_storage[victime_frame * FRAME_SIZE + j] = NULL; // pad remaining slots in case end of file
		}
	}
    return 1;
}


// Function to load a single page from a file into memory.
// returns 1 if error with file, 0 if successful (had space / evicted page to make space)
int load_page(char* filename, int page_number, int* page_table) {
	// Open file & make sure it exists
	FILE* f = fopen(filename, "r");
	if (f == NULL) return 1;

	// program & frame info 
	char buffer[MAX_LINE_LENGTH];  // buffer to read lines into
	char* lines[FRAME_SIZE];  // to hold the lines we want to load in memory

	// go to the start of the page we want to load
	int start_line = page_number * FRAME_SIZE;
	for (int i = 0; i < start_line; i++){
		if (fgets(buffer, MAX_LINE_LENGTH, f) == NULL){  // if we hit end of file before reaching the start of the page, return error
			fprintf(stderr, "Error: page number %d is out of bounds for program %s\n", page_number, filename);
			fclose(f);
			return 1;
		}
	}
	// Read the next 3 lines into buffer
	for (int i = 0; i < FRAME_SIZE; i++){
		if (fgets(buffer, MAX_LINE_LENGTH, f) != NULL){
			lines[i] = strdup(buffer);  // copy line into frame
		} else {
			lines[i] = NULL;  // pad remaining slots in case end of file
		}
	}
	fclose(f);

	// Add lines to memory
	add_frame(lines, filename, page_number, page_table);
	
	return 0;
} 


// Function to load the first 2 pages of a program into memory 
int load_init(char* filename, int* length_out, int* page_table){
	// Open file & make sure it exists
	FILE* f = fopen(filename, "r");
	if (f == NULL) return 1;

	// Count total lines in the file (for program length)
    int total_lines = 0;
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, f) != NULL) {
        total_lines++;
    }
    int total_pages = (total_lines + FRAME_SIZE - 1) / FRAME_SIZE;  // round up
	*length_out = total_lines;
    fclose(f);

	// Load first 2 pages (or fewer if program is shorter) into memory
	for (int page_number = 0; page_number < 2 && page_number < total_pages; page_number++) {
		if (load_page(filename, page_number, page_table) != 0) return 1;
		//printf("Loaded page %d to %d \n", page_number, page_table[page_number]);
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

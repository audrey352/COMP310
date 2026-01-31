#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <ctype.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
//new commands:
int echo(char *var);
int my_ls();
int my_mkdir(char *dirname); 
int my_touch(char *filename);
int my_cd(char *dirname);
int run(char *command_args[]);


// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }


    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0){
	    if (args_size != 2)
		    return badcommand();
	    return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0){
        if (args_size != 1)
		    return badcommand();
	    return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0){
        if (args_size != 2) 
		    return badcommand();
	    return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0){
        if (args_size != 2) 
		    return badcommand();
	    return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0){
        if (args_size != 2) 
		    return badcommand();
	    return my_cd(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0){
        if (args_size < 2)  // at least one argument needed
            return badcommand();
        return run(command_args);
    } else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}

int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *string){
	// Initialize eval to the string, which we will return
	char *eval = malloc(strlen(string) + 1);
	strcpy(eval, string);
	
    // Initialize value for the character without the first character
	char *value = string+1;

	//Step 1: Check if string is preceded by $
	if (string[0] == '$'){
		char *found = mem_get_value(value);
		
		if (strcmp(found, "Variable does not exist") == 0){
			eval = ""; //replace with an empty string
		}
		else{
			strcpy(eval, found); //replace with found value
		}
	}

	printf("%s\n", eval); //print and return
	return 0;
}

int my_ls(){
    struct dirent **namelist;
    int n;

    // get list of files and directories in current directory + sort them
    n = scandir(".", &namelist, NULL, alphasort); // # of entries (-1 if error)

    // go through each entry and print its name
    for (int i = 0; i < n; i++) {
        printf("%s\n", namelist[i]->d_name);
        free(namelist[i]);  // free each entry
    }
    free(namelist);   // free the list pointer
    return 0;
}

int my_mkdir(char *dirname){
    // Initialize eval which we'll use to create the directory
    char *eval = malloc(strlen(dirname) + 1);  // +1 for \0
    strcpy(eval, dirname);

    // check for $ 
    if (dirname[0] == '$'){
        char *value = dirname + 1; //get variable name without $
		char *found = mem_get_value(value); // look up value in shell memory
		
		if (strcmp(found, "Variable does not exist") == 0){
			printf("Bad command: my_mkdir\n");
            return 1;  // variable not found
		}
		else{
			strcpy(eval, found);;  // replace with value found
		}
	}

    // check that characters are alphanumeric
    for (int i = 0; eval[i] != '\0'; i++) {
        if (!isalnum(eval[i])) {
            printf("Bad command: my_mkdir\n");
            return 1;  // non-alphanumeric character found
        }
    }

    mkdir(eval, 0755);  // create directory with rwxr-xr-x permissions
    free(eval);
    return 0;
}

int my_touch(char *filename){
    // check if alphanumeric
    for (int i = 0; filename[i] != '\0'; i++) {
        if (!isalnum(filename[i])) {
            printf("Bad command: my_touch\n");
            return 1;  // non-alphanumeric character found
        }
    }

    // create new empty file
    FILE *fp = fopen(filename, "w");
    fclose(fp);

    return 0;
}

int my_cd(char *dirname){
    // check if alphanumeric, allow . and ..
    if (!(strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0)) {
        for (int i = 0; dirname[i] != '\0'; i++) {
            if (!isalnum(dirname[i])) {
                printf("Bad command: my_cd\n");
                return 1;  // non-alphanumeric character found
            }
        }
    }
    
    // Change directory to dirname
    if (chdir(dirname) != 0) {
        printf("Bad command: my_cd\n");
        return 1;  // error changing directory
    }

    return 0;
}

int run(char *command_args[]){
    // fork a new process
    pid_t pid = fork();

    // check if fork fails
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    // child process
    if (pid == 0) { 
        execvp(command_args[1], command_args + 1); // execute command after run command (in regular shell)
        perror("exec failed"); // check if exec fails
        exit(0); // exit child process
    }
    // parent process 
    else {
        wait(NULL); // wait for child to finish
    }
    return 0;
}
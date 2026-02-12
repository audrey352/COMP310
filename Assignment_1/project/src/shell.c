#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.5 created Dec 2025\n");
    fflush(stdout); //Flush after printing so that the banner will always print first

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    //init shell memory
    mem_init();
    while(1) {
	    //isatty is part of unistd: returns 1 if file is the 
	    // terminal and 0 if from something else...
        if (isatty(STDIN_FILENO)){	    
                printf("%c ", prompt);
        }
        
        // terminating if EOF (NULL) is reached
        if (fgets(userInput, MAX_USER_INPUT-1, stdin)==NULL){
            return 0;
        }
	
	//stores and parses user input
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' ;
}

int parseInput(char inp[]) {
    //Using strtok command to split into different commands that we can parse and pass
    const char *delim = ";"; 
    char *command; //current command we will work with
    command  = strtok(inp, delim); //split into first

    int errorCode = 0;
    int counter = 0; 

    while (command != NULL && counter < 10){
    char tmp[200], *words[100];                            
    int ix = 0, w = 0;
    int wordlen;
    //parse edited to be only for specific command (replaced inp with command)
    for (ix = 0; command[ix] == ' ' && ix < 1000; ix++); // skip white spaces
    while (command[ix] != '\n' && command[ix] != '\0' && ix < 1000) {
        // extract a word
        for (wordlen = 0; !wordEnding(command[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = command[ix];                        
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (command[ix] == '\0') break;
        ix++; 
    }

    words[w] = NULL; //Making sure that it ends after w words
    
    errorCode = interpreter(words, w); //send command to the interpreter
    //if things went wrong, return immediately
    if (errorCode != 0){
	    return errorCode;
    }
    command = strtok(NULL, delim); //set to next word
    counter++; 
    }
    return errorCode;
}

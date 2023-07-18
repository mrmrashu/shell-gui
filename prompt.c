#include <stdio.h>
#include "shell.h"

//The first function prints the first prompt string, or PS1,
// which you usually see when the shell is waiting for you to enter a command.
void print_prompt1(void){
	fprintf(stderr,"(myShell) $ ");
}

//The second function prints the second prompt string, or PS2,
// which is printed by the shell when you enter a multi-line command 
void print_prompt2(void){
	fprintf(stderr,">>> ");
}
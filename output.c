#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#include "output.h"

#define BUFFER_SIZE 1024

char* get_output_cmd(int argc,char **argv,char *output_o) {
     

     // Calculate the new size of the argv array
    int newArgc = argc + 2;

    // Allocate memory for the new argv array
    char  **newArgv = (char **)malloc(newArgc * sizeof(char *));

    // Check if memory allocation was successful
    if (newArgv == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    // Copy the original arguments to the new argv array, starting from index 2
    for (int i = 2; i < newArgc; i++) {
        newArgv[i] = argv[i - 2];
    }

    // Insert new elements at positions 0 and 1
    newArgv[0] = "/";
    newArgv[1] = "-c";
    newArgv[newArgc] = NULL;
     

    // ---------------------------------------------
    int pipefd[2];
    char buffer[BUFFER_SIZE];
    char* output = NULL;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
    } else if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close the read end of the pipe

        // Redirect stdout to the write end of the pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            close(pipefd[1]);
            exit(1);
        }

        // Execute the command
        // char* const arguments[] = { "/bin/sh", "-c", command, NULL };
        // execv("/bin/sh", arguments);
        //  if (strchr(argv[0],'/')) //searches for the first occurrence of the character '/' (an unsigned char) in the string (argv[0]) and return 1 if found.
        //     {
        //         execv(argv[0],newArgv);
        //     }
        // else
        //     {
                char *path = search_path(argv[0]);
                if(!path)
                {
                    return NULL;
                }

                strcpy(newArgv[0],path);
                execv(path,newArgv);
            // }

        // If execv returns, it means there was an error executing the command
        perror("execv");
        close(pipefd[1]);
        exit(1);
    } else {
        // Parent process
        close(pipefd[1]);  // Close the write end of the pipe

        // Read the output of the command from the read end of the pipe
        ssize_t bytesRead;
        size_t totalBytesRead = 0;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output = realloc(output, totalBytesRead + bytesRead + 1);
            if (output == NULL) {
                perror("realloc");
                close(pipefd[0]);
                return NULL;
            }
            memcpy(output + totalBytesRead, buffer, bytesRead);
            totalBytesRead += bytesRead;
        }
        output[totalBytesRead] = '\0';  // Add null terminator to the output

        close(pipefd[0]);  // Close the read end of the pipe

        // Wait for the child process to exit
        int status;
        waitpid(pid, &status, 0);
    }

   // return output;
    
}

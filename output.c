#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "output.h"
#include "shell.h"
#include "node.h"
#include <errno.h>



#define BUFFER_SIZE 1024

static inline void free_argv(int argc , char **argv)
{
    if(!argc)
    {
        return;
    }
    while(argc--)
    {
        free(argv[argc]);
    }
}

char *get_output_cmd(int argc, char **argv)
{
    
    // printf("test 1\n");
    int flag =0;
    
    // ---------------------------------------------
    int pipefd[2];
    char buffer[BUFFER_SIZE];
    char *output = NULL;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return NULL;
    }
    else if (pid == 0)
    {
        // printf("test 2\n");

        // Child process
        close(pipefd[0]); // Close the read end of the pipe

        // Redirect stdout to the write end of the pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(pipefd[1]);
            exit(1);
        }

        // printf("test 3\n");
        flag = do_exec_cmd(argc,argv);

        fprintf(stderr,"error: failed to execute <<>> command !!: %s\n",strerror(errno));

        if (errno == ENOEXEC)
        {
            exit(126);
        }
        else if (errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
        // // If execv returns, it means there was an error executing the command
        // perror("execv");

        // char *err = "error: failed to execute command !! :";
        // strcat(err,strerror(errno));
        close(pipefd[1]);
        // exit(1);
        // return err;
    }
    if(flag == 0) {
   
        int status;
        waitpid(pid, &status, 0);
        // printf("test 4\n");

        // Parent process
        close(pipefd[1]); // Close the write end of the pipe

        // Read the output of the command from the read end of the pipe
        ssize_t bytesRead;
        size_t totalBytesRead = 0;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            char *temp = realloc(output, totalBytesRead + bytesRead + 1);
            if (temp == NULL)
            {
                perror("realloc");
                free(output);
                close(pipefd[0]);
                return NULL;
            }
            output = temp;

            memcpy(output + totalBytesRead, buffer, bytesRead);
            totalBytesRead += bytesRead;
        }
        output[totalBytesRead] = '\0'; // Add null terminator to the output

        close(pipefd[0]); // Close the read end of the pipe

        // Wait for the child process to exit
        

        return output;
    }
    int status = 0;
    waitpid(pid,&status,0);

    free_argv(argc,argv);
     

}

// int main(int argc, char** argv) {
//     char* output = get_output_cmd(argc, argv);
//     if (output != NULL) {
//         printf("Output: %s\n", output);
//         free(output);  // Free the dynamically allocated memory
//     }
//     return 0;
// }

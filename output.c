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
#include <signal.h>


#define SIG_ERROR_SIGNAL SIGUSR1

#define BUFFER_SIZE 1024

void handle_error(int signum, siginfo_t *info, void *context) {
    // Get the error message from the signal info
    const char *error_message = (const char *)info->si_ptr;
    // Send the error message to main.c for printing in the GUI
}

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

        // Install the signal handler for the error signal
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handle_error;
        sigemptyset(&sa.sa_mask);
        sigaction(SIG_ERROR_SIGNAL, &sa, NULL);

        // Trigger an error and send the error signal
        // Remove the previous declaration of `error_message` if exists
        char* error_message = NULL;
        
        // Get the length of the error message
        int error_message_length = snprintf(NULL, 0, "error: failed to execute <<>> command !!: %s\n", strerror(errno));

        // Allocate memory for `error_message`
        error_message = malloc((error_message_length + 1) * sizeof(char));

        // Check if memory allocation was successful
        if (error_message != NULL) {
            // Use `sprintf` to print the error message into `error_message`
            sprintf(error_message, "error: failed to execute <<>> command !!: %s\n", strerror(errno));
        }
        

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

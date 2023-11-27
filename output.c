#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <errno.h>
#include <string.h> 
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"
#include "output.h"


#define BUFFER_SIZE 1024

static inline void free_argv(int argc, char **argv)
{
    if (!argc)
    {
        return;
    }
    while (argc--)
    {
        free(argv[argc]);
    }
}
int parse_and_execute(struct source_s *src)
{
    skip_white_spaces(src);
    struct token_s *tok = tokenize(src);
    if(tok == &eof_token)
    {
        return 0;
    }
    while(tok && tok != &eof_token) 
    {
        struct node_s *cmd = parse_simple_command(tok);
        if(!cmd)
        {
            break;
        }  
       

        int status = do_simple_command(cmd);


        free_node_tree(cmd);
        tok = tokenize(src);
    }
    return 1;
}

void read_cmd_gui(char *input){


    const size_t len_input = strlen(input)+1;
    char* cmd = malloc(len_input); // A variable to store commands
        // print_prompt1();

        // cmd = read_cmd();

        // For GUI
        strncpy(cmd,input,len_input);

        // If there's an error reading the command, we exit the shell
        if(!cmd){
            exit(EXIT_SUCCESS);
        }

        // If the command is empty (i.e. the user pressed ENTER without writing anything, we skip this input and continue with the loop.
        if (cmd[0]=='\0' || strcmp(cmd,"\n")==0)
        {
            free(cmd);
            return;
        }

        //  If the command is exit, we exit the shell.
        if(strcmp(cmd, "exit\n") == 0)
        {
            free(cmd);
            exit(EXIT_SUCCESS);
            
        }

        //Otherwise, we echo back the command, free the memory we used to store the command, and continue with the loop. 
        // printf("%s\n",cmd);
        struct source_s src;
        src.buffer   = cmd;
        src.bufsize  = strlen(cmd);
        src.curpos   = INIT_SRC_POS;
        parse_and_execute(&src);

    
        free(cmd);
    // exit(EXIT_SUCCESS);
}


char *get_output_cmd(int argc, char **argv) {
    int stdin_pipe[2];
    int pipe_stdout[2];
    int pipe_stderr[2];
    char buffer[BUFFER_SIZE];
    char *output = NULL;

    if (pipe(stdin_pipe) == -1 || pipe(pipe_stdout) == -1 || pipe(pipe_stderr) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return NULL;

    } else if (pid == 0) {
        // Child process
        close(stdin_pipe[1]);
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);

        // Redirect stdin to the write end of the pipe
        if (dup2(stdin_pipe[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        // Redirect stdout to the write end of the pipe
        if (dup2(pipe_stdout[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // Redirect stderr to the write end of the pipe
        if (dup2(pipe_stderr[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        do_exec_cmd(argc, argv);
    }

    // Parent process
    close(stdin_pipe[0]);
    close(pipe_stderr[1]);
    close(pipe_stdout[1]);
    char *command = " ";
    write(stdin_pipe[1], command, strlen(command));
    close(stdin_pipe[1]);
    int status;

    fd_set read_set;
    struct timeval timeout;
    timeout.tv_sec = 5; // 5 seconds
    timeout.tv_usec = 0;

    FD_ZERO(&read_set);
    FD_SET(pipe_stdout[0], &read_set);
    FD_SET(pipe_stderr[0], &read_set);

    int max_fd = pipe_stderr[0] > pipe_stdout[0] ? pipe_stderr[0] : pipe_stdout[0];
    int ready = select(max_fd + 1, &read_set, NULL, NULL, &timeout);

    if (ready == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    } else if (ready == 0) {
        printf("No data available within the timeout.\n");
    } else {
        if (FD_ISSET(pipe_stderr[0], &read_set) || FD_ISSET(pipe_stdout[0], &read_set)) {
            ssize_t bytesRead;
            size_t totalBytesRead = 0;

            while ((bytesRead = read(pipe_stderr[0], buffer, sizeof(buffer))) > 0 ||
                   (bytesRead = read(pipe_stdout[0], buffer, sizeof(buffer))) > 0) {
                char *temp = realloc(output, totalBytesRead + bytesRead + 1);
                if (temp == NULL) {
                    perror("realloc");
                    free(output);
                    close(pipe_stdout[0]);
                    close(pipe_stderr[0]);
                    return NULL;
                }
                output = temp;

                memcpy(output + totalBytesRead, buffer, bytesRead);
                totalBytesRead += bytesRead;
            }

            output[totalBytesRead] = '\0'; // Add null terminator to the output
        }
    }

    close(pipe_stdout[0]);
    close(pipe_stderr[0]);
    waitpid(pid, &status, 0);

    free_argv(argc, argv);

    return output;
}

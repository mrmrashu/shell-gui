#include <stdio.h>
#include "executor.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "shell.h"
#include "node.h"
#include "output.h"
#define BUFFER_SIZE 1024



/* The search_path() function takes the name of a command, then searches the directories listed in the $PATH variable to try and find the command's executable file.
   The $PATH variable contains a comma-separated list of directories, such as /bin:/usr/bin. For each directory, we create a pathname by appending the command name to the directory name,
   then we call stat() to see if a file that exists with the given pathname (for simplicity, we don't check whether the file is actually executable, or whether we have enough permissions to execute it).
   If the file exists, we assume it contains the command we want to execute, and we return the full pathname of that command. If we don't find the file in the first $PATH directory, we search the second,
   third, and the rest of the $PATH directories, until we find our executable file. If we fail in finding the command by searching all the directories in the $PATH, we return NULL (this usually means the user typed an invalid command name).
*/
char *search_path(char * file) // here file is executable file we will search this exe in PATH of enviornment if it is found then return full path otherwise NULL. 
{
	char *PATH = getenv("PATH");
	// getenv("PATH") will return like - /naman/home/bin:/ashu/home/bin:/bin:/user/local/sbin:snap/bin
	char *p_Path = PATH;
	char *p2_p ;

	while(p_Path && *p_Path)
	{
		p2_p = p_Path;

		while(*p2_p && *p2_p != ':')
		{
			p2_p++;
		}
		int p_Path_len = p2_p - p_Path;
		// p_Path_len = length of like /user/local/sbin

		if(!p_Path_len){

		 	p_Path_len = 1;

		}
		 int alen = strlen(file);
		 char path[p_Path_len+1+alen+1];

		strncpy(path,p_Path,p2_p-p_Path);
		path[p2_p-p_Path] = '\0';

		if (p2_p[-1] != '/')
		{
		 	strcat(path,"/");
		}
		strcat(path,file);

		struct stat st; // stat structure store all the information aboud a file. 
		 //so we check if file we foud is Regular file or not.
		if (stat(path,&st)==0) // On success the function returns 0 and fills the buf structure(st) with file properties. On error the function return -1 and sets error code
		{
		 	if (!S_ISREG(st.st_mode)) //S_ISREG, true when the file is a plain ol’ “regular” file
		 	{
		 		errno = ENOENT;
		 		p_Path = p2_p; // update the pointer to next searching path dir..
		 		if (*p2_p == ':') // do not want to include ":" so increment pointer by one.
		 		{
		 			p_Path++;
		 		}
		 		continue; // go to next itration
		 	}
		
		 p_Path = malloc(strlen(path)+1);
		 if(!p_Path)
		 {
		 	return NULL;

		 }
		 strcpy(p_Path,path);
		 return p_Path;

		}
		else // file not found
		{
			p_Path = p2_p;
			if (*p2_p == ':')
			{
				p_Path++;
			}
		}

	}
	errno = ENOENT;
	return NULL;
}
/* The do_exec_cmd() function executes a command by calling execv() to replace the current process image with the new command executable. If the command name contains any slash characters,
   we treat it as a pathname and we directly call execv(). Otherwise, we try to locate the command by calling search_path(), which should return the full pathname that we will pass on to execv().
 */
// Execute command for GUI
int do_exec_cmd(int argc,char **argv,char *output_o)
{
	if (strchr(argv[0],'/')) //searches for the first occurrence of the character '/' (an unsigned char) in the string (argv[0]) and return 1 if found.
	{
		// get_output_cmd(argc,argv,output_o);
		execv(argv[0],argv);
	}
	else
	{
		char *path = search_path(argv[0]);
		if(!path)
		{
			return 0;
		}
		// get_output_cmd(argc,argv,output_o);
		execv(path,argv);
		free(path);
	}
	return 0;
}

// The free_argv() function frees the memory we used to store the arguments list of the last command executed.
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
/*
The do_simple_command() function is the main function in our executor. It takes the command's AST and converts it to an arguments list (Remember that the zeroth argument, or argv[0], contains the name of the command we want to execute).

The function then forks a new child process. In the child process, we execute the command by calling do_exec_cmd(). If the command is executed successfully, this call shouldn't return. If it does return, it means an error happened (e.g., the command wasn't found, the file wasn't executable, insufficient memory, ...). In this case, we print a suitable error message and exit with a non-zero exit status.

In the parent process, we call waitpid() to wait for our child process to finish execution. We then free the memory we used to store the arguments list and return.
*/
int do_simple_command(struct node_s *node,char *output_o){

	if(!node){
		return 0;
	}

	struct node_s *child = node->first_child;
	if (!child)
	{
		return 0;
	}

	int argc = 0;
	long max_args = 255;
	char *argv[max_args+1]; // keep 1 for terminating NULL arg..
	char *str;

	while(child)
	{
		str = child->val.str;
		argv[argc] = malloc(strlen(str)+1);

		if (!argv[argc])
		{
			free_argv(argc,argv);
			return 0;
		}

		strcpy(argv[argc],str);
		if (++argc >= max_args)
		{
			break;
		}
		child = child->next_sibling;
	}
	argv[argc]= NULL;

	pid_t child_pid = 0;
	if ((child_pid = fork()) == 0)
	{
		do_exec_cmd(argc,argv,output_o);
		// printf("%s",src->output);

		fprintf(stderr,"error: failed to execute command !!: %s\n",strerror(errno));

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
	}
	else if (child_pid < 0)
	{
		fprintf(stderr,"error: failed to fork command: %s\n",strerror(errno));
		return 0;
	}
	int status = 0;
	waitpid(child_pid,&status,0);
	/*
	The value of pid can be:

<-1 : Wait for any child process whose process group ID is equal to the absolute value of pid.
-1  : Wait for any child process.
0   : Wait for any child process whose process group ID is equal to that of the calling process.
>0  : Wait for the child whose process ID is equal to the value of pid.
	*/
	free_argv(argc,argv);


	return 1;
}
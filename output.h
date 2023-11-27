#ifndef OUTPUT_H
#define OUTPUT_H

#define BUFFER_SIZE 1024

#include "source.h"
#include "executor.h"

char *get_output_cmd(int argc,char **argv);
void read_cmd_gui(char *input);


#endif

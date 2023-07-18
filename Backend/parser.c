#include <unistd.h>
#include "shell.h"
#include "parser.h"
#include "scanner.h"
#include "node.h"
#include "source.h"
// Creates a AST(Abstract Syntax Tree),  which is a tree-like structure that contains information about the components of a command.
struct node_s *parse_simple_command(struct token_s *tok)
{
	if (!tok)
	{
		return NULL;
	}
	struct node_s *cmd = new_node(NODE_COMMAND);
	if (!cmd)
	{
		free_token(tok);
		return NULL;
	}
	struct source_s *src = tok->src;

	do
	{
		//until we get a newline token (which we test for in the line that reads: if(tok->text[0] == '\n'))
		if (tok->text[0]=='\n')
		{
			free_token(tok);
			break;
		}
		struct node_s *word = new_node(NODE_VAR);
		if (!word)
		{
			free_node_tree(cmd);
			free_token(tok);
			return NULL;
		}
		set_node_val_str(word,tok->text);
		add_child_node(cmd,word);

		free_token(tok);


	}while((tok = tokenize(src)) != &eof_token);
	//To parse a simple command, we only need to call tokenize() to retrieve input tokens, one by one
    // or we reach the end of our input (we know this happened when we get an eof_token token. 
	return cmd;
}
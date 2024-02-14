#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "scanner.h"
#include "source.h"
/*tok_buf is a pointer to the buffer in which we'll store the current token.
 */
char *tok_buf = NULL;
/*tok_bufsize is the number of bytes we allocate to the buffer
 */
int tok_bufsize = 0;
/*tok_bufindex is the current buffer index (i.e. it tells us where to add the next input character in the buffer)
 */
int tok_bufindex = -1;
/* special token to indicate end of input */
struct token_s eof_token =
    {
        .text_len = 0,
};
/*The add_to_buf() function adds a single character to the token buffer. If the buffer is full, the function takes care of extending it
 */
void add_to_buf(char c)
{
    tok_buf[tok_bufindex++] = c;
    if (tok_bufindex >= tok_bufsize)
    {
        char *tmp = realloc(tok_buf, tok_bufsize * 2);
        if (!tmp)
        {
            errno = ENOMEM;
            return;
        }
        tok_buf = tmp;
        tok_bufsize *= 2;
    }
}
/*The create_token() function takes a string and converts it to a struct token_s structure. It takes care of allocating memory for the token's structure and text, and fills in the structure's member fields.
 */
struct token_s *create_token(char *str)
{
    struct token_s *tok = malloc(sizeof(struct token_s));

    if (!tok)
    {
        return NULL;
    }
    memset(tok, 0, sizeof(struct token_s));
    tok->text_len = strlen(str);

    char *nstr = malloc(tok->text_len + 1);

    if (!nstr)
    {
        free(tok);
        return NULL;
    }

    strcpy(nstr, str);
    tok->text = nstr;

    return tok;
}
/*The free_token() function frees the memory used by a token structure, as well as the memory used to store the token's text.
 */
void free_token(struct token_s *tok)
{
    if (tok->text)
    {
        free(tok->text);
    }
    free(tok);
}
/*The tokenize() function is the heart and soul of our lexical scanner, and the function's code is fairly straight-forward. It starts by allocating memory for our token buffer (if not already done), then initializes our token buffer and source pointers. It then calls next_char() to retrieve the next input character. When we reach the end of input, tokenize() returns the special eof_token, which marks the end of input.
 */
struct token_s *tokenize(struct source_s *src)
{
    int endloop = 0;
    if (!src || !src->buffer || !src->bufsize)
    {
        errno = ENODATA;
        return &eof_token;
    }

    if (!tok_buf)
    {
        tok_bufsize = 1024;
        tok_buf = malloc(tok_bufsize);
        if (!tok_buf)
        {
            errno = ENOMEM;
            return &eof_token;
        }
    }
    tok_bufindex = 0;
    tok_buf[0] = '\0';
    char nc = next_char(src);
    if (nc == ERRCHAR || nc == EOF)
    {
        return &eof_token;
    }
    /*The function then loops to read input characters, one at a time. If it encounters a whitespace character, it checks the token buffer to see if it’s empty or not. If the buffer is not empty, we delimit the current token and break out of the loop. Otherwise, we skip the whitespace characters and move along to the beginning of the next token.
     */
    do
    {
        switch (nc)
        {
        case ' ':
        case '\t':
            if (tok_bufindex > 0)
            {
                endloop = 1;
            }
            break;

        case '\n':
            if (tok_bufindex > 0)
            {
                unget_char(src);
            }
            else
            {
                add_to_buf(nc);
            }
            endloop = 1;
            break;

        default:
            add_to_buf(nc);
            break;
        }
        if (endloop)
        {
            break;
        }
    } while ((nc = next_char(src)) != EOF);
    if (tok_bufindex == 0)
    {
        return &eof_token;
    }

    if (tok_bufindex >= tok_bufsize)
    {
        tok_bufindex--;
    }
    tok_buf[tok_bufindex] = '\0';
    /*After we’ve got our token, tokenize() calls create_token(), passing it the token text (which we stored in the buffer). The token text is converted to a token structure, which tokenize() then returns to the caller.
     */
    struct token_s *tok = create_token(tok_buf);
    if (!tok)
    {
        fprintf(stderr, "error: failed to alloc buffer: %s\n",
                strerror(errno));
        return &eof_token;
    }
    tok->src = src;
    return tok;
}
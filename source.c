#include <errno.h>
#include "shell.h"
#include "source.h"
/*
The unget_char() function returns (or ungets) the last character we've retrieved from input, back to the input source. It does this by simply manipulating the source structure's pointers.
*/
void unget_char(struct source_s *src)
{
    if (src->curpos < 0)
    {
        return;
    }
    src->curpos--;
}
/*The next_char() function returns the next character of input and updates the source pointer, so that the next call to next_char() returns the following input character. When we reach the last character in input, the function returns the special character EOF, which we defined as -1 in source.h above.
 */
char next_char(struct source_s *src)
{
    if (!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }
    char c1 = 0;
    if (src->curpos == INIT_SRC_POS)
    {
        src->curpos = -1;
    }
    else
    {
        c1 = src->buffer[src->curpos];
    }
    if (++src->curpos >= src->bufsize)
    {
        src->curpos = src->bufsize;
        return EOF;
    }
    return src->buffer[src->curpos];
}
/*The peek_char() function is similar to next_char() in that it returns the next character of input. The only difference is that peek_char() doesn't update the source pointer, so that the next call to next_char() returns the same input character we've just peeked at. You'll see the benefit of input peeking later in this series.
 */
char peek_char(struct source_s *src)
{
    if (!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }
    long pos = src->curpos;
    if (pos == INIT_SRC_POS)
    {
        pos++;
    }
    pos++;
    if (pos >= src->bufsize)
    {
        return EOF;
    }
    return src->buffer[pos];
}
/*the skip_white_spaces() function skips all whitespace characters. This will help us when we've finished reading a token, and we want to skip delimiter whitespaces before we read the next token.
 */
void skip_white_spaces(struct source_s *src)
{
    char c;
    if (!src || !src->buffer)
    {
        return;
    }
    while (((c = peek_char(src)) != EOF) && (c == ' ' || c == '\t'))
    {
        next_char(src);
    }
}
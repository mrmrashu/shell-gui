#ifndef SOURCE_H
#define SOURCE_H

#define EOF (-1)
#define ERRCHAR (0)

#define INIT_SRC_POS (-2)
/*
To make the scanner’s job easy,
we’ll abstract our input by passing the input string as part of a struct source_s structure
*/
struct source_s
{
  char *buffer; /* the input text */
  long bufsize; /* size of the input text */
  long curpos;  /* absolute char position in source */
};
/*
Focusing on the structure’s definition,
 you can see that our struct source_s contains a pointer to the input string,
 in addition to a two long fields that hold information about the string's length and
  our current position in the string (where we'll get the next character from).
*/
char next_char(struct source_s *src);
void unget_char(struct source_s *src);
char peek_char(struct source_s *src);
void skip_white_spaces(struct source_s *src);

#endif
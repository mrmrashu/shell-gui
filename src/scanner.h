#ifndef SCANNER_H
#define SCANNER_H
struct token_s
{
    struct source_s *src; /* source of input */
    int text_len;         /* length of token text */
    char *text;           /* token text */
};
/* the special EOF token, which indicates the end of input */
extern struct token_s eof_token;
struct token_s *tokenize(struct source_s *src);
void free_token(struct token_s *tok);
#endif

/*Focusing on the structure definition, our struct token_s contains a pointer to the struct source_s that holds our input. The structure also contains a pointer to the token's text, and a field that tells us the length of that text (so that we don't need to repeatedly call strlen() on the token's text).
 */
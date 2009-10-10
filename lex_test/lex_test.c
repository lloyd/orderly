#include "../src/orderly_lex.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX_INPUT_TEXT (1 << 20)

int main(int argc, char ** argv) 
{
    /* this is not a stream lexer, let's read as much as we can up to a
     * reasonable limit  (1 meg) */
    static char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
        tot += rd;

    {
        orderly_tok t;
        unsigned int off = 0;
        orderly_lexer lexer = orderly_lex_alloc(NULL);
        const unsigned char * outBuf = NULL;
        unsigned int outLen = 0;

        do {
            t = orderly_lex_lex(lexer, (const unsigned char *) inbuf,
                                tot, &off, &outBuf, &outLen);
            printf("(%3u,%3u): %d '",
                   orderly_lex_current_line(lexer),
                   orderly_lex_current_char(lexer),
                   t);
            fwrite(outBuf, sizeof(char), outLen, stdout);
            printf("'\n");
            
        } while (t != orderly_tok_error && t != orderly_tok_eof);
    }
    
    return 0;
}

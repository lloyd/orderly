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
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT))) {
        tot += rd;
    }

    printf("%s\n", inbuf);

    return 0;
}

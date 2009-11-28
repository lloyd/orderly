#include "../../../src/orderly_parse.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX_INPUT_TEXT (1 << 20)

static const char * nodeToStr(orderly_node * n)
{
    if (n) {
        switch (n->t) {
            case orderly_node_empty: return "empty";
            case orderly_node_error: return "error";
        }
    }
    return "unknown";
}

int
main(int argc, char ** argv) 
{
    /* this is not a stream lexer, let's read as much as we can up to a
     * reasonable limit  (1 meg) */
    static unsigned char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
        tot += rd;

    {
        orderly_node * n = orderly_parse(NULL, inbuf, tot);
        printf("%s\n", nodeToStr(n));
        orderly_free_node(&n);
    }
    
    return 0;
}

#include "../../../src/orderly_parse.h"
#include "../../../src/orderly_alloc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX_INPUT_TEXT (1 << 20)

static void dumpNode(orderly_node * n, unsigned int indent)
{
    if (n) {
        const char * type = "unknown";
        switch (n->t) {
            case orderly_node_empty: type = "empty"; break;
            case orderly_node_null: type = "null"; break;
        }
        printf("(%p) %s [%s]\n", (void *) n, n->name, type);        
    } else {
        printf("(null)\n");
    }
    
}


static const char * statusToStr(orderly_parse_status s)
{
    switch (s) {
        case orderly_parse_s_ok: return "ok";
        case orderly_parse_s_not_implemented: return "not_implemented";
        case orderly_parse_s_prop_name_expected: return "prop_name_expected";
        case orderly_parse_s_gt_expected: return "gt_expected";
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
        orderly_alloc_funcs oaf;
        orderly_node * n;
        orderly_parse_status s;

        orderly_set_default_alloc_funcs(&oaf);

        s = orderly_parse(&oaf, inbuf, tot, &n);
        printf("VVVVVVVV (%s)\n", statusToStr(s));
        dumpNode(n, 0); /* here's where we'll map over and output the returned tree */ 
        printf("^^^^^^^^\n");
        /* TODO: ugly alloc routine crap here, perhaps we should give the
         *       ultimate client a parse handle and make the ownership of
         *       the parse tree held by the parse handle? */
        orderly_free_node(&oaf, &n);
    }
    
    return 0;
}

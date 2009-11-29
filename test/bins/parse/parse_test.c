#include "../../../src/orderly_parse.h"
#include "../../../src/orderly_alloc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_TEXT (1 << 20)

static void dumpNode(orderly_node * n, unsigned int indent)
{
    char * indentStr = (char *) malloc(indent * 4 + 1);
    memset((void *) indentStr, ' ', indent);
    indentStr[indent] = 0;

    if (n) {
        const char * type = "unknown";
        switch (n->t) {
            case orderly_node_empty: type = "empty"; break;
            case orderly_node_null: type = "null"; break;
            case orderly_node_string: type = "string"; break;
            case orderly_node_boolean: type = "boolean"; break;
            case orderly_node_any: type = "any"; break;
        }
        printf("%s%s [%s] %s\n", indentStr, n->name, type,
               n->optional ? "OPTIONAL" : "");        
        if (n->default_value) printf("%s--> default: %s\n",
                                        indentStr, n->default_value);        
        if (n->values) printf("%s--> enum: %s\n", indentStr, n->values);        
        if (n->requires) printf("%s--> requires: %s\n", indentStr, n->requires);        
    } else {
        printf("%s(null)\n", indentStr);
    }
    free(indentStr);
}

static const char * statusToStr(orderly_parse_status s)
{
    switch (s) {
        case orderly_parse_s_ok: return "ok";
        case orderly_parse_s_not_implemented: return "not_implemented";
        case orderly_parse_s_prop_name_expected: return "prop_name_expected";
        case orderly_parse_s_gt_expected: return "gt_expected";
        case orderly_parse_s_prop_name_syntax_error: return "prop_name_syntax_error";
        case orderly_parse_s_expected_schema_entry: return "expected_schema_entry";
        case orderly_parse_s_junk_at_end_of_input: return "junk_at_end_of_input";
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
        printf("parse complete (%s):\n", statusToStr(s));
        dumpNode(n, 0); /* here's where we'll map over and output the returned tree */ 
        /* TODO: ugly alloc routine crap here, perhaps we should give the
         *       ultimate client a parse handle and make the ownership of
         *       the parse tree held by the parse handle? */
        orderly_free_node(&oaf, &n);
    }
    
    return 0;
}

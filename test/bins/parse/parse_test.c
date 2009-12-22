/*
 * Copyright 2009, Lloyd Hilaiel.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "../../../src/orderly_parse.h"
#include "../../../src/orderly_alloc.h"
#include "../../../src/orderly_buf.h"
#include "../../../src/orderly_json.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_TEXT (1 << 20)

static void dumpNode(orderly_alloc_funcs * oaf, orderly_node * n, unsigned int indent)
{
    orderly_buf b = orderly_buf_alloc(oaf);
    char * indentStr = (char *) malloc(indent * 4 + 1);
    memset((void *) indentStr, ' ', indent * 4);
    indentStr[indent*4] = 0;

    if (n) {
        const char * type = orderly_node_type_to_string(n->t);
        if (NULL == type) type = "unknown";

        printf("%s%s [%s]%s%s\n", indentStr, n->name ? n->name : "", type,
               n->optional ? " OPTIONAL" : "",
               n->additional_properties ? " OPEN" : "");
        if (n->default_value) {
            orderly_buf_clear(b);
            orderly_write_json(oaf, n->default_value, b, 0);
            printf("%s--> default: %s\n", indentStr, orderly_buf_data(b));
        }
        
        if (n->values) {
            orderly_buf_clear(b);
            orderly_write_json(oaf, n->values, b, 0);
            printf("%s--> enum: %s\n", indentStr, orderly_buf_data(b));
        }
        if (n->requires) {
            const char ** p = n->requires;
            printf("%s--> requires: ", indentStr);
            while (p && *p) {
                if (p != n->requires) printf(", ");
                printf("%s", *p);
                p++;
            }
            printf("\n");
        }
        
        if (n->regex) printf("%s--> regex: %s\n", indentStr, n->regex);        
        if (ORDERLY_RANGE_SPECIFIED(n->range)) {
            printf("%s--> range: {", indentStr);
            if (ORDERLY_RANGE_LHS_DOUBLE & n->range.info)
                printf("%g", n->range.lhs.d);
            else if (ORDERLY_RANGE_LHS_INT & n->range.info)            
                printf("%ld", n->range.lhs.i);
            printf(",");
            if (ORDERLY_RANGE_RHS_DOUBLE & n->range.info)
                printf("%g", n->range.rhs.d);
            else if (ORDERLY_RANGE_RHS_INT & n->range.info)            
                printf("%ld", n->range.rhs.i);
            printf("}\n");
        }
        if (n->child) {
            printf("%schildren:\n", indentStr);
            dumpNode(oaf, n->child, indent + 1);
        }
        if (n->sibling) {
            dumpNode(oaf, n->sibling, indent);
        }
    } else {
        printf("%s(null)\n", indentStr);
    }
    free(indentStr);
    orderly_buf_free(b);
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
        case orderly_parse_s_malformed_range: return "malformed_range";
        case orderly_parse_s_integer_overflow: return "integer_overflow";
        case orderly_parse_s_numeric_parse_error: return "numeric_parse_error";
        case orderly_parse_s_left_curly_expected: return "left_curly_expected";
        case orderly_parse_s_right_curly_expected: return "right_curly_expected";
        case orderly_parse_s_lex_error: return "lex_error";
        case orderly_parse_s_jsonschema_error: return "jsonschema_parse_error";
        case orderly_parse_s_right_bracket_expected: return "right_bracket_expected";
        case orderly_parse_s_invalid_json: return "invalid_json";
        case orderly_parse_s_backtick_expected: return "backtick_expected";
    }
    return "unknown";
}

#include "../debug_alloc_routines.h"

int
main(int argc, char ** argv) 
{
    /* this is not a stream parser, let's read as much as we can up to a
     * reasonable limit  (1 meg) */
    static unsigned char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
        tot += rd;

    {
        orderly_node * n;
        orderlyTestMemoryContext memStats;
        orderly_parse_status s;
        orderly_alloc_funcs oaf = {
            orderlyTestMalloc,
            orderlyTestRealloc,
            orderlyTestFree,
            &memStats
        };
        memset((void *) &memStats, 0, sizeof(memStats));
        
        s = orderly_parse(&oaf, inbuf, tot, &n, NULL);
        printf("parse complete (%s):\n", statusToStr(s));
        dumpNode(&oaf, n, 0); /* here's where we'll map over and output the returned tree */ 
        /* TODO: ugly alloc routine crap here, perhaps we should give the
         *       ultimate client a parse handle and make the ownership of
         *       the parse tree held by the parse handle? */
        orderly_free_node(&oaf, &n);

        /* now memstats! */
        if (memStats.numFrees < memStats.numMallocs)
        {
            printf("MEMORY LEAK: %d allocations/%d frees\n",
                   memStats.numMallocs, memStats.numFrees);
        }
    }
    
    return 0;
}

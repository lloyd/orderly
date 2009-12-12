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

#include "api/writer.h"
#include "orderly_buf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct orderly_writer_t
{
    struct orderly_writer_config cfg;
    orderly_buf b;
};

orderly_writer
orderly_writer_new(const struct orderly_writer_config * cfg)
{
    orderly_writer w;
    static struct orderly_writer_config s_cfg;
    static orderly_alloc_funcs s_alloc;
    static int initd;
    
    if (!initd) {
        orderly_set_default_alloc_funcs(&s_alloc);
        s_cfg.alloc = &s_alloc;
        s_cfg.pretty = 1;
        initd = 1;
    }        

    /* if !cfg we'll use defaults */
    if (!cfg) cfg = &s_cfg;
    
    w = OR_MALLOC(cfg->alloc, sizeof(struct orderly_writer_t));        
    w->b = orderly_buf_alloc(cfg->alloc);
    memcpy((void *) &(w->cfg), (void *) cfg,
           sizeof(struct orderly_writer_config));

    if (!w->cfg.alloc) w->cfg.alloc = &s_alloc;

    return w;
}

void
orderly_writer_free(orderly_writer *w)
{
}

static int
dumpNode(orderly_writer w, const orderly_node * n, unsigned int indent)
{
    static const char * indentStr = "    ";
    char buf[128];

#define INDENT_IF_DESIRED                                    \
        if (w->cfg.pretty) {                                 \
            unsigned int i = 0;                              \
            for (i = 0; i < indent; i++) {                   \
                orderly_buf_append_string(w->b, indentStr);  \
            }                                                \
        }


    if (n) {
        const char * type = NULL;
        switch (n->t) {
            case orderly_node_empty: type = "empty"; break;
            case orderly_node_null: type = "null"; break;
            case orderly_node_string: type = "string"; break;
            case orderly_node_boolean: type = "boolean"; break;
            case orderly_node_any: type = "any"; break;
            case orderly_node_integer: type = "integer"; break;
            case orderly_node_number: type = "number"; break;
            case orderly_node_object: type = "object"; break;
            case orderly_node_array: type = "array"; break;
            case orderly_node_union: type = "union"; break;
        }
        if (!type) return 0;

        INDENT_IF_DESIRED;
        orderly_buf_append_string(w->b, type);

        /* children! */
        if (n->child) {
            orderly_buf_append_string(w->b, " {");
            if (w->cfg.pretty) orderly_buf_append_string(w->b, "\n");
            dumpNode(w, n->child, indent + 1);
            INDENT_IF_DESIRED;
            orderly_buf_append_string(w->b, "}");
        }

        /* optional range */
        if (ORDERLY_RANGE_SPECIFIED(n->range)) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "{");
            buf[0] = 0;
            if (ORDERLY_RANGE_LHS_DOUBLE & n->range.info)
                sprintf(buf, "%g", n->range.lhs.d);
            else if (ORDERLY_RANGE_LHS_INT & n->range.info)
                sprintf(buf, "%ld", n->range.lhs.i);
            if (buf[0]) orderly_buf_append_string(w->b, buf);
            orderly_buf_append_string(w->b, ",");
            buf[0] = 0;
            if (ORDERLY_RANGE_RHS_DOUBLE & n->range.info)
                sprintf(buf, "%g", n->range.rhs.d);
            else if (ORDERLY_RANGE_RHS_INT & n->range.info)
                sprintf(buf, "%ld", n->range.rhs.i);
            if (buf[0]) orderly_buf_append_string(w->b, buf);
            orderly_buf_append_string(w->b, "}");
        }

        /* name time */
        if (n->name) {
            /* XXX: names with spaces an' shit? */
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, n->name);
        }

        /* optional regex */
        if (n->regex) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, n->regex);
        }

        /* enumerated possible values */
        if (n->values) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, n->values);
        }
        
        /* default value */
        if (n->default_value) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "=");
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, n->default_value);
        }

        /* requires value */
        if (n->requires) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "<");
            orderly_buf_append_string(w->b, n->requires);
            orderly_buf_append_string(w->b, ">");
        }

        if (n->optional) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "?");
        }

        orderly_buf_append_string(w->b, ";");        
        if (w->cfg.pretty) orderly_buf_append_string(w->b, "\n");        

        if (n->sibling) {
            dumpNode(w, n->sibling, indent);
        }
    }
    return 1;
}


const char *
orderly_write(orderly_writer w, orderly_format fmt,
              const orderly_node * node)
{
    if (!w) return NULL;
    orderly_buf_clear(w->b);
    /** XXX: respect the fmt */
    if (!dumpNode(w, node, 0)) return NULL;
    return (const char *) orderly_buf_data(w->b);
}

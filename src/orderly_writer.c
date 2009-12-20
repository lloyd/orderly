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
#include "orderly_lex.h"
#include "orderly_json.h"

#include <yajl/yajl_gen.h>

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
dumpNodeAsOrderly(orderly_writer w, const orderly_node * n, unsigned int indent)
{
    static const char * indentStr = "  ";
    char buf[128];

#define INDENT_IF_DESIRED                                    \
        if (w->cfg.pretty) {                                 \
            unsigned int i = 0;                              \
            for (i = 0; i < indent; i++) {                   \
                orderly_buf_append_string(w->b, indentStr);  \
            }                                                \
        }


    if (n) {
        const char * type = orderly_node_type_to_string(n->t);
        if (!type) return 0;

        INDENT_IF_DESIRED;
        orderly_buf_append_string(w->b, type);

        /*  children!  */
        if (n->t == orderly_node_array ||
            n->t == orderly_node_object ||
            n->t == orderly_node_union)
        {
            orderly_buf_append_string(w->b, " {");            
            if (n->child) {
                if (w->cfg.pretty) orderly_buf_append_string(w->b, "\n");
                dumpNodeAsOrderly(w, n->child, indent + 1);
                INDENT_IF_DESIRED;
            } else {
                orderly_buf_append_string(w->b, " ");
            }
            orderly_buf_append_string(w->b, "}");
        }
        
        if ((n->t == orderly_node_array ||
             n->t == orderly_node_object) &&
            n->additionalProperties)
        {
            orderly_buf_append_string(w->b, "*");
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
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            /* keywords or names with certain chars must be quoted */
            if (orderly_lex_keyword_check((unsigned char *) n->name, strlen(n->name))
                != orderly_tok_property_name ||
                strspn(n->name, "abcdefghijklmnopqrstuvwxyz"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ-_") != strlen(n->name))
            {
                unsigned int i = 0;
                orderly_buf_append_string(w->b, "\"");                
                for (i=0; i < strlen(n->name); i++) {
                    if (n->name[i] == '"') orderly_buf_append_string(w->b , "\\\"");
                    else orderly_buf_append(w->b, n->name + i, 1);                                    
                }
                orderly_buf_append_string(w->b, "\"");                
            } else {
                orderly_buf_append_string(w->b, n->name);
            }
            
        }

        /* optional regex */
        if (n->regex) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "/");
            orderly_buf_append_string(w->b, n->regex);
            orderly_buf_append_string(w->b, "/");
        }

        /* enumerated possible values */
        if (n->values) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_write_json(w->cfg.alloc, n->values, w->b);
        }
        
        /* default value */
        if (n->default_value) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "=");
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_write_json(w->cfg.alloc, n->default_value, w->b);
        }

        /* requires value */
        if (n->requires) {
            if (w->cfg.pretty) orderly_buf_append_string(w->b, " ");
            orderly_buf_append_string(w->b, "<");
            orderly_buf_append_string(w->b, n->requires);
            orderly_buf_append_string(w->b, ">");
        }

        if (n->optional) {
            orderly_buf_append_string(w->b, "?");
        }

        orderly_buf_append_string(w->b, ";");        
        if (w->cfg.pretty) orderly_buf_append_string(w->b, "\n");        

        if (n->sibling) {
            dumpNodeAsOrderly(w, n->sibling, indent);
        }
    }
    return 1;
}

#define YAJL_GEN_STRING_WLEN(yg, s) \
    yajl_gen_string((yg), (const unsigned char *) (s), strlen(s));

static int
dumpNodeAsJSONSchema(orderly_writer w, const orderly_node * n, yajl_gen yg)
{
    if (n) {
        const char * type = orderly_node_type_to_string(n->t);
        if (!type) return 0;
        
        /* open up this entry */
        yajl_gen_map_open(yg);
        
        /* dump the type */
        YAJL_GEN_STRING_WLEN(yg, "type");
        YAJL_GEN_STRING_WLEN(yg, type);        

        if (n->child) {
            if (n->t == orderly_node_array) {
                YAJL_GEN_STRING_WLEN(yg, "items");
                if (n->child && n->child->sibling) {
                    const orderly_node * k = NULL;
                    yajl_gen_array_open(yg);
                    for (k = n->child; k; k = k->sibling) {
                        dumpNodeAsJSONSchema(w, k, yg);
                    }
                    yajl_gen_array_close(yg);
                } else {
                    dumpNodeAsJSONSchema(w, n->child, yg);                
                }
            } else if (n->t == orderly_node_object) {
                const orderly_node * kid = n->child;
                YAJL_GEN_STRING_WLEN(yg, "properties");
                yajl_gen_map_open(yg);            
                for (kid = n->child; kid != NULL; kid = kid->sibling) {
                    if (!kid->name) return 0;
                    YAJL_GEN_STRING_WLEN(yg, kid->name);
                    dumpNodeAsJSONSchema(w, kid, yg);
                }
                yajl_gen_map_close(yg);
            } else if (n->t == orderly_node_union) {
                /* XXX */ 
            }
        }

        /* optional range */
        if (ORDERLY_RANGE_SPECIFIED(n->range)) { 
            const char * minword, * maxword;

            switch (n->t) {
                case orderly_node_integer:
                case orderly_node_number:
                    minword = "minimum";
                    maxword = "maximum";
                    break;
                case orderly_node_array:
                    minword = "minItems";
                    maxword = "maxItems";
                    break;
                case orderly_node_string:
                    minword = "minLength";
                    maxword = "maxLength";
                    break;
                default:
                    return 0; /* XXX: error code!  cannot include min/max params on something other
                               * than the types enumerated above */ 
            }
            
            if (ORDERLY_RANGE_HAS_LHS(n->range)) {
                YAJL_GEN_STRING_WLEN(yg, minword);
                if (ORDERLY_RANGE_LHS_DOUBLE & n->range.info)
                    yajl_gen_double(yg, n->range.lhs.d);
                else if (ORDERLY_RANGE_LHS_INT & n->range.info)
                    yajl_gen_integer(yg, n->range.lhs.i);
            }

            if (ORDERLY_RANGE_HAS_RHS(n->range)) {
                YAJL_GEN_STRING_WLEN(yg, maxword);
                if (ORDERLY_RANGE_RHS_DOUBLE & n->range.info)
                    yajl_gen_double(yg, n->range.rhs.d);
                else if (ORDERLY_RANGE_RHS_INT & n->range.info)
                    yajl_gen_integer(yg, n->range.rhs.i);
            }
        }
        if (n->optional) {
            YAJL_GEN_STRING_WLEN(yg, "optional");
            yajl_gen_bool(yg, 1);
        }

        /* optional regex */
        if (n->regex) { 
            YAJL_GEN_STRING_WLEN(yg, "pattern");
            YAJL_GEN_STRING_WLEN(yg, n->regex);
        } 

        /* default value */
        if (n->default_value) { 
            YAJL_GEN_STRING_WLEN(yg, "default");
            orderly_write_json2(yg, n->default_value);
        }

        /* enumerated possible values */
        if (n->values) { 
            YAJL_GEN_STRING_WLEN(yg, "enum");
            orderly_write_json2(yg, n->values);
        } 

        /* additionalProperties */
        if (n->additionalProperties) { 
            YAJL_GEN_STRING_WLEN(yg, "additionalProperties");
            yajl_gen_bool(yg, 1);
        } 
        
/*         /\* requires value *\/ */
/*         if (n->requires) { */
/*             if (w->cfg.pretty) orderly_buf_append_string(w->b, " "); */
/*             orderly_buf_append_string(w->b, "<"); */
/*             orderly_buf_append_string(w->b, n->requires); */
/*             orderly_buf_append_string(w->b, ">"); */
/*         } */

        yajl_gen_map_close(yg);

    }
    return 1;
}


static void
bufAppendCallback(void * ctx, const char * str, unsigned int len)
{
    orderly_buf_append((orderly_buf) ctx, str, len);
}


const char *
orderly_write(orderly_writer w, orderly_format fmt,
              const orderly_node * node)
{
    if (!w) return NULL;
    orderly_buf_clear(w->b);
    /** respect the fmt */
    if (fmt == ORDERLY_JSONSCHEMA) {
        yajl_gen_config cfg = { 1, NULL };
        yajl_gen g = yajl_gen_alloc2(bufAppendCallback, &cfg,
                                     (const yajl_alloc_funcs *) w->cfg.alloc,
                                     (void *) w->b);
        int rv = dumpNodeAsJSONSchema(w, node, g);
        yajl_gen_free(g);
        if (!rv) return NULL;
    } else {
        if (!dumpNodeAsOrderly(w, node, 0)) return NULL;    
    }
    
    return (const char *) orderly_buf_data(w->b);
}

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

#include "orderly_json_parse.h"
#include "orderly_ptrstack.h"
#include "orderly_alloc.h"

#include <yajl/yajl_parse.h>
#include <string.h>


#define BUF_STRDUP(dst, a, ob, ol)               \
    (dst) = OR_MALLOC((a), (ol) + 1);            \
    memcpy((void *)(dst), (void *) (ob), (ol));  \
    ((char *) (dst))[(ol)] = 0;

typedef enum {
    OPS_Init = 0,
    OPS_ParseNode,
    OPS_HandleType,
    OPS_HandleProperties0,
    OPS_HandleProperties1,
    OPS_HandleMinimum,
    OPS_HandleMaximum,
    OPS_HandleItems,
    OPS_EndOfStates
} orderly_parse_state;

/* #define ORDERLY_DEBUG_PARSER 1 */
#if defined(ORDERLY_DEBUG_PARSER)
#include <stdio.h>

static const char *
psToString(orderly_parse_state ps)
{
    static const char * states[] = {
        "init", "pnode", "htype",  "hp0", "hp1", "min", "max", "item"
    };
    if (ps >= OPS_EndOfStates) return "?????";
    return states[ps];
}

#define DUMP_PARSER_STATE(__s, __str)                                         \
    printf("%20s [%5s] cur:%10p val:%s\n", (__s), psToString(pc->state), (void *) pc->current, __str);

#define DUMP_PARSER_STATE_STR(__s, __str, __len)   \
{                                                  \
  char buf[64];                                    \
  unsigned int i = __len;                          \
  if (63 < i) i = 63;                              \
  snprintf(buf, i+1, "%s", __str);               \
  buf[i] = 0;                                      \
  DUMP_PARSER_STATE((__s), buf);                   \
}

#define DUMP_PARSER_STATE_DOUBLE(__s, __d)         \
{                                                  \
  char buf[64];                                    \
  sprintf(buf, "%g", (__d));                       \
  DUMP_PARSER_STATE((__s), buf);                   \
}
#define DUMP_PARSER_STATE_LONG(__s, __l)         \
{                                                  \
  char buf[64];                                    \
  sprintf(buf, "%ld", (__l));                       \
  DUMP_PARSER_STATE((__s), buf);                   \
}
#else
#define DUMP_PARSER_STATE(x,y);
#define DUMP_PARSER_STATE_DOUBLE(x,y);
#define DUMP_PARSER_STATE_LONG(x,y);
#define DUMP_PARSER_STATE_STR(x,y,z);
#endif


typedef struct
{
    orderly_alloc_funcs * alloc;
    orderly_ptrstack keys;
    orderly_ptrstack nodes;
    orderly_ptrstack states;
    orderly_parse_state state;
    orderly_node * current;
    orderly_json_parse_status error;
} orderly_parse_context;

static
int js_parse_null(void * ctx)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE("js_parse_null", "null");
    return 1;
}


static int js_parse_boolean(void * ctx, int boolean)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE("js_parse_boolean", (boolean ? "true" : "false"));
    return 1;
}


static int js_parse_double(void * ctx, double d)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE_DOUBLE("js_parse_double", d);
    if (pc->state == OPS_HandleMinimum) {
        pc->current->range.info |= ORDERLY_RANGE_LHS_DOUBLE;
        pc->current->range.lhs.d = d;
        pc->state = OPS_ParseNode;
    } else if (pc->state == OPS_HandleMaximum) {
        pc->current->range.info |= ORDERLY_RANGE_RHS_DOUBLE;
        pc->current->range.rhs.d = d;
        pc->state = OPS_ParseNode;
    } else {
        pc->error = orderly_json_parse_s_unexpected_number;
    }
    return (pc->error == orderly_json_parse_s_ok);
}

static int js_parse_integer(void * ctx, long l)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE_LONG("js_parse_integer", l);
    if (pc->state == OPS_HandleMinimum) {
        pc->current->range.info |= ORDERLY_RANGE_LHS_INT;
        pc->current->range.lhs.i = l;
        pc->state = OPS_ParseNode;
    } else if (pc->state == OPS_HandleMaximum) {
        pc->current->range.info |= ORDERLY_RANGE_RHS_INT;
        pc->current->range.rhs.i = l;
        pc->state = OPS_ParseNode;
    } else {
        pc->error = orderly_json_parse_s_unexpected_number;
    }
    return (pc->error == orderly_json_parse_s_ok);
}


static int js_parse_string(void * ctx, const unsigned char * v, unsigned int l)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE_STR("js_parse_string", v, l);
    if (pc->state == OPS_HandleType) {
        pc->state = OPS_ParseNode;
        pc->current->t = orderly_string_to_node_type((const char*)v, l);
        if (pc->current->t == orderly_node_empty) {
            pc->error = orderly_json_parse_s_unrecognized_node_type;
        }
    } else {
        pc->error = orderly_json_parse_s_unexpected_json_string;
    }

    return (pc->error == orderly_json_parse_s_ok);
}


static int js_parse_map_key(void * ctx, const unsigned char * v,
                            unsigned int l)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE_STR("js_parse_map_key", v, l);

    if (pc->state == OPS_ParseNode)
    {
        /* great! let's see what kind of element this is */
        if (v && !strncmp((const char *) v, "type", l)) {
            pc->state = OPS_HandleType;
        } else if (v && !strncmp((const char *) v, "properties", l)) {
            pc->state = OPS_HandleProperties0;
        } else if (v && !strncmp((const char *) v, "items", l)) {
            /* we need a distinct state (distinct from from handleProperties) 
             * because items may have EITHER a schema or an array of schemas */
            pc->state = OPS_HandleItems;
        } else if (v && !strncmp((const char *) v, "minimum", l)) {
            pc->state = OPS_HandleMinimum;
        } else if (v && !strncmp((const char *) v, "maximum", l)) {
            pc->state = OPS_HandleMaximum;
        } else {
            pc->error = orderly_json_parse_s_unexpected_property_name;
        }
    } else if (pc->state == OPS_HandleProperties1) {
        /* push this key onto the keystack */
        char * key;
        BUF_STRDUP(key, pc->alloc, v, l);
        orderly_ps_push(pc->alloc, pc->keys, key);
        pc->state = OPS_Init;
    } else {
        pc->error = orderly_json_parse_s_unexpected_json_property;
    }

    return (pc->error == orderly_json_parse_s_ok);
}


static int js_parse_start_map(void * ctx)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE("js_parse_start_map", "{");
    if (pc->state == OPS_Init) {
        orderly_node * old = pc->current;
        pc->state = OPS_ParseNode;
        pc->current = orderly_alloc_node(pc->alloc, orderly_node_empty);
        pc->current->sibling = old;
        orderly_ps_push(pc->alloc, pc->states, (void *) OPS_HandleProperties1);        
    } else if (pc->state == OPS_HandleProperties0) {
        pc->state = OPS_HandleProperties1;
        orderly_ps_push(pc->alloc, pc->nodes, pc->current);
        pc->current = NULL;
    } else if (pc->state == OPS_HandleItems) {
        pc->state = OPS_ParseNode;
        orderly_ps_push(pc->alloc, pc->nodes, pc->current);
        pc->current = orderly_alloc_node(pc->alloc, orderly_node_empty);
        orderly_ps_push(pc->alloc, pc->states, (void *) OPS_ParseNode);
    } else {
        pc->error = orderly_json_parse_s_unexpected_json_map;
        return 0;
    }
    return 1;
}


static int js_parse_end_map(void * ctx)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;

    DUMP_PARSER_STATE("js_parse_end_map", "}");

    if (pc->state == OPS_ParseNode) {    
        pc->state = (orderly_parse_state) orderly_ps_current(pc->states);
        orderly_ps_pop(pc->states);
        if (pc->state == OPS_HandleProperties1) {
            /* last name on the stack belongs to us */
            if (orderly_ps_length(pc->keys)) {
                pc->current->name = orderly_ps_current(pc->keys);
                orderly_ps_pop(pc->keys);                            
            }
        } else if (pc->state == OPS_ParseNode) {    
            /* we just parsed an *items* entry, current becomes child of top of stack */
            orderly_node * p = pc->current; 
            pc->current = orderly_ps_current(pc->nodes);
            orderly_ps_pop(pc->nodes);
            pc->current->child = p;
        } else {
            /* internal error */
            return 0;
        }
    } else if (pc->state == OPS_HandleProperties1) {
        /* current becomes child of first node on the stack, also
         * *reverse* sibling order.  given the way the parser works, otherwise
         * a straight forward traversal of the parse tree would yield the
         * opposite order expected.  slightly wasteful. */

        if (orderly_ps_length(pc->nodes)) {
            orderly_node * p = pc->current;
            pc->current = orderly_ps_current(pc->nodes);
            orderly_ps_pop(pc->nodes);            
            while (p) {
                orderly_node * n = p->sibling;
                p->sibling = pc->current->child;
                pc->current->child = p;
                p = n;
            }
        }
        
        pc->state = OPS_ParseNode;        
    }
    return 1;
}


static int js_parse_start_array(void * ctx)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE("js_parse_start_array", "[");
    return 1;
}


static int js_parse_end_array(void * ctx)
{
    orderly_parse_context * pc = (orderly_parse_context *) ctx;
    DUMP_PARSER_STATE("js_parse_end_array", "]");
    return 1;
}


orderly_json_parse_status
orderly_json_parse(orderly_alloc_funcs * alloc,
                   const unsigned char * schemaText,
                   const unsigned int schemaTextLen,
                   orderly_node ** n,
                   unsigned int * final_offset)
{
    static yajl_callbacks callbacks = {
        js_parse_null,
        js_parse_boolean,
        js_parse_integer,
        js_parse_double,
        NULL,
        js_parse_string,
        js_parse_start_map,
        js_parse_map_key,
        js_parse_end_map,
        js_parse_start_array,
        js_parse_end_array
    };


    yajl_handle hand;
    yajl_status stat;
    /* allow comments! */
    yajl_parser_config cfg = { 1, 1 };
    orderly_parse_context pc;

    memset((void *) &pc, 0, sizeof(pc));
    pc.alloc = alloc;

    /* allocate a parser */
    hand = yajl_alloc(&callbacks, &cfg,
                      (const yajl_alloc_funcs *) alloc,
                      (void *) &pc);

    /* read file data, pass to parser */
    stat = yajl_parse(hand, schemaText, schemaTextLen);

    if (stat == yajl_status_insufficient_data)
    {
    }
    else if (stat != yajl_status_ok)
    {
/*        unsigned char * str = yajl_get_error(hand, 1, schemaText, schemaTextLen);
          fprintf(stderr, (const char *) str);
          yajl_free_error(hand, str); */
        if (final_offset) *final_offset = yajl_get_error_offset(hand);
    }
    else
    {
        /* we're ok! */
        *n = pc.current;
    }

    return pc.error;
}

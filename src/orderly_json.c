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

/* a lightweight C representation of json data */

#include "orderly_json.h"

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include <stdlib.h>
#include <string.h>


void orderly_free_json(orderly_alloc_funcs * alloc,
                       orderly_json ** node)
{
    /* XXX */
}


orderly_json * orderly_alloc_json(orderly_alloc_funcs * alloc,
                                  orderly_json_type t)
{
    orderly_json * n = (orderly_json *) OR_MALLOC(alloc, sizeof(orderly_json));
    memset((void *) n, 0, sizeof(orderly_json));
    n->t = t;
    return n;
}

// push an element to the correct place
#define PUSH_NODE(pc, __n)                                                      \
  if (orderly_ps_length((pc)->nodeStack) == 0) {                                \
      orderly_ps_push((pc)->alloc, (pc)->nodeStack, __n);                       \
  } else {                                                                      \
      orderly_json * top = (orderly_json *) orderly_ps_current((pc)->nodeStack);\
      if (top->t == orderly_json_array) {                                       \
          if (top->v.children.last) top->v.children.last->next = (__n);         \
          top->v.children.last = (__n);                                         \
          if (!top->v.children.first) top->v.children.first = (__n);            \
      } else if (top->t == orderly_json_object) {                               \
          if (top->v.children.last) top->v.children.last->next = (__n);         \
          top->v.children.last = (__n);                                         \
          if (!top->v.children.first) top->v.children.first = (__n);            \
          (__n)->k = orderly_ps_current((pc)->keyStack);                        \
          orderly_ps_pop((pc)->keyStack);                                       \
      }                                                                         \
  }


#define BUF_STRDUP(dst, a, ob, ol)               \
    (dst) = OR_MALLOC((a), (ol) + 1);            \
    memcpy((void *)(dst), (void *) (ob), (ol));  \
    ((char *) (dst))[(ol)] = 0;


int o_json_parse_start_array(void * ctx)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_array);
    orderly_ps_push(pc->alloc, pc->nodeStack, n);
    return 1;
}

int o_json_parse_end_array(void * ctx)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_ps_current(pc->nodeStack);
    orderly_ps_pop(pc->nodeStack);
    PUSH_NODE(pc, n);
    return 1;
}

int o_json_parse_start_map(void * ctx)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_object);
    orderly_ps_push(pc->alloc, pc->nodeStack, n);
    return 1;
}

int o_json_parse_map_key(void * ctx, const unsigned char * v,
                         unsigned int l)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    char * k = NULL;
    BUF_STRDUP(k, pc->alloc, v, l);
    orderly_ps_push(pc->alloc, pc->keyStack, k);
    return 1;
}

int o_json_parse_end_map(void * ctx)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_ps_current(pc->nodeStack);
    orderly_ps_pop(pc->nodeStack);
    PUSH_NODE(pc, n);
    return 1;
}

int o_json_parse_string(void * ctx, const unsigned char * v, unsigned int l)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_string);
    BUF_STRDUP(n->v.s, pc->alloc, v, l);
    PUSH_NODE(pc, n);
    return 1;
}

int o_json_parse_integer(void * ctx, long l)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_integer);
    n->v.i = l;
    PUSH_NODE(pc, n);
    return 1;
}

int o_json_parse_double(void * ctx, double d)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_number);
    n->v.n = d;
    PUSH_NODE(pc, n);
    return 1;
}

int o_json_parse_null(void * ctx)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_null);
    PUSH_NODE(pc, n);
    return 1;
}


int o_json_parse_boolean(void * ctx, int val)
{
    o_json_parse_context * pc = (o_json_parse_context *) ctx;
    orderly_json * n = orderly_alloc_json(pc->alloc, orderly_json_boolean);
    n->v.b = val;
    PUSH_NODE(pc, n);
    return 1;
}


#include <stdio.h>


orderly_json *
orderly_read_json(orderly_alloc_funcs * alloc,
                  const char * jsonText,
                  unsigned int * len)
{
    static yajl_callbacks callbacks = {
        o_json_parse_null,
        o_json_parse_boolean,
        o_json_parse_integer,
        o_json_parse_double,
        NULL,
        o_json_parse_string,
        o_json_parse_start_map,
        o_json_parse_map_key,
        o_json_parse_end_map,
        o_json_parse_start_array,
        o_json_parse_end_array
    };

    yajl_handle hand;
    yajl_status stat;
    /* allow comments! */
    yajl_parser_config cfg = { 1, 1 };
    o_json_parse_context pc;
    orderly_json * j = NULL;

    memset((void *) &pc, 0, sizeof(pc));
    pc.alloc = alloc;

    /* allocate a parser */
    hand = yajl_alloc(&callbacks, &cfg,
                      (const yajl_alloc_funcs *) alloc,
                      (void *) &pc);

    /* read file data, pass to parser */
    stat = yajl_parse(hand, (const unsigned char *) jsonText, *len);
    if (stat == yajl_status_insufficient_data || stat == yajl_status_ok)
    {
        stat = yajl_parse_complete(hand);
    }

    if (stat != yajl_status_ok)
    {
        unsigned char * str = yajl_get_error(hand, 1, (const unsigned char *) jsonText, *len);
        fprintf(stderr, (const char *) str);
        yajl_free_error(hand, str);
        /* if (final_offset) *final_offset = yajl_get_error_offset(hand); */
    }
    else if (!orderly_ps_length(pc.nodeStack))
    {
        /* XXX: ERROR! */
    }
    else 
    {
        /* we're ok! */
        j = orderly_ps_current(pc.nodeStack);
    }

    return j;
}


static int writeJson(yajl_gen g, const orderly_json * j)
{
    yajl_gen_status s;
    int rv = 1;

    if (j) {
        if (j->k) yajl_gen_string(g, (const unsigned char *) j->k, strlen(j->k));

        switch (j->t) {
            case orderly_json_none:
                return 0;
            case orderly_json_null:
                s = yajl_gen_null(g);
                break;
            case orderly_json_string:
                s = yajl_gen_string(g, (const unsigned char *) j->v.s, strlen(j->v.s));
                break;
            case orderly_json_boolean:
                s = yajl_gen_bool(g, j->v.b);
                break;
            case orderly_json_integer:
                s = yajl_gen_integer(g, j->v.i);
                break;
            case orderly_json_number:
                s = yajl_gen_double(g, j->v.n);
                break;
            case orderly_json_object:
                s = yajl_gen_map_open(g);
                rv = writeJson(g, j->v.children.first);
                s = yajl_gen_map_close(g);
                break;
            case orderly_json_array:
                s = yajl_gen_array_open(g);
                rv = writeJson(g, j->v.children.first);
                s = yajl_gen_array_close(g);
                break;
        }

        if (rv && j->next) rv = writeJson(g, j->next);
    }

    return rv;
}

static void
bufAppendCallback(void * ctx, const char * str, unsigned int len)
{
    orderly_buf_append((orderly_buf) ctx, str, len);
}


void
orderly_write_json(const orderly_alloc_funcs * alloc,
                   const orderly_json * json,
                   orderly_buf b)
{
    yajl_gen_config cfg = { 0, NULL };
    yajl_gen g = yajl_gen_alloc2(bufAppendCallback, &cfg,
                                 (const yajl_alloc_funcs *) alloc,
                                 (void *) b);
    int rv = writeJson(g, json);
    yajl_gen_free(g);
}

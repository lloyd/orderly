/*
 * Copyright 2009, 2010, LLoyd Hilaiel.
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

#include "ajv_state.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
static int ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen);
static int ajv_start_map (void * ctx);
static int ajv_end_map(void * ctx);
static int ajv_number(void * ctx, const char * numberVal,
                      unsigned int numberLen);
static int ajv_string(void * ctx, const unsigned char * stringVal,
               unsigned int stringLen);
static int ajv_start_array(void * ctx);
static int ajv_end_array(void * ctx);


const static yajl_callbacks ajv_callbacks = {
  NULL, /* yajl_null */
  NULL, /* XXX ? boolean */
  NULL, /* integer */
  NULL, /* double */
  ajv_number, /* number */
  ajv_string,
  ajv_start_map,
  ajv_map_key,
  ajv_end_map,
  ajv_start_array,
  ajv_end_array
};

YAJL_API yajl_handle ajv_alloc(const yajl_callbacks * callbacks,
                               const yajl_parser_config * config,
                               const yajl_alloc_funcs * allocFuncs,
                               void * ctx,
                               const char *schema) {

  orderly_reader r = orderly_reader_new(NULL);
  const orderly_node *n;
  n = orderly_read(r, ORDERLY_UNKNOWN, schema, sizeof(schema));
  struct ajv_state_t *ajv_state = 
    (struct ajv_state_t *)
    OR_MALLOC(allocFuncs, sizeof(struct ajv_state_t));

  ajv_state->cb = callbacks;
  ajv_state->cbctx = ctx;
  return yajl_alloc(&ajv_callbacks,
                    config,
                    allocFuncs,
                    (void *)ajv_state);
}




static int ajv_start_map (void * ctx) {
  Ajv_STATE(ctx);
  if (typecheck(state,orderly_node_map,state->on.t)) {  
    return 1;
  }
  push_state(state);
  
}

static int ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen) {
  YALJV_STATE(ctx);
  ajv_node *cur;
  const char *name;

  ASSERT(orderly_node_map == ORDERLY_NODE(state->node->parent)->t);

  for (cur = state->node->parent->child; cur; cur = cur->sibling) {
    ASSERT(ORDERLY_NODE(cur)->name);
    if (!strncmp(ORDERLY_NODE(cur)->name,key,stringLen)) {
      cur->seen = 1;
      /* XXX: can we have multiple schemas for a key ?! */
    }
  }
  return *(ctx->cb->map_key)(key, stringLen);
}


static int ajv_end_map(void * ctx) {
  AJV_STATE(ctx);
  find_node(ctx,key,len) 

  POP_STATE(st);
}
        

#if 0
int ajv_null(void * ctx) {
  AJV_STATE(ctx);
  
}
#endif

/** A callback which passes the string representation of the number
 *  back to the client.  Will be used for all numbers when present */


static int ajv_number(void * ctx, const char * numberVal,
                 unsigned int numberLen) {
  AJV_STATE(ctx);
  if (typecheck(state,orderly_node_number,state->on.t)) {
    return 1;
  }
  
}


/** strings are returned as pointers into the JSON text when,
 * possible, as a result, they are _not_ null padded */
static int ajv_string(void * ctx, const unsigned char * stringVal,
                unsigned int stringLen) {
  AJV_STATE(ctx);
  orderly_node *on = state->node->node;
  if (on->t != orderly_node_string) {
    VALIDATE_FAILED(wrong_type,orderly_node_string,on->t);
  }
  if (ORDERLY_RANGE_SPECIFIED(on->range)) {
    if (ORDERLY_RANGE_HAS_LHS(on->range)) {
      if (on->range.lhs.i > stringLen) {
  
      VALIDATE_FAILED(out_of_range,on->range.lhs.i,stringLen);
      }
    }
    if (ORDERLY_RANGE_HAS_RHS(on->range)) {
      if (on->range.rhs.i < stringLen) {

        VALIDATE_FAILED(out_of_range,on->range.rhs.i,stringLen);
      }
    }
  }

  if (on->values) {
    ASSERT(on->values->t == orderly_json_array); /* docs say so */
    /* XXX  validate this is a value in the array */
  }

  if (on->regex) {
    /* XXX: validate regex */
  }

  return state->cb->yajl_string(ctx,stringVal,stringLen);
}



static int ajv_start_array(void * ctx) {
  AJV_STATE(ctx);
  if (typecheck(state,orderly_node_array,state->on.t)) {  
    return 1;
  }
  push_state(state);
  return state->cb->yajl_start_array(ctx);
}


static int ajv_end_array(void * ctx) {
  AJV_STATE(ctx);
  push_state(state);
  return state->cb->yajl_end_array(ctx);
}

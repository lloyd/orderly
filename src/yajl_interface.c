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
#include "api/ajv_parse.h"
#include "api/reader.h"
#include "api/node.h"
#include "api/json.h"
#include <yajl/yajl_parse.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

static int ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen);
static int ajv_start_map (void * ctx);
static int ajv_end_map(void * ctx);
static int ajv_integer(void * ctx, long integerValue);
static int ajv_double(void * ctx, double value);

static int ajv_string(void * ctx, const unsigned char * stringVal,
               unsigned int stringLen);
static int ajv_start_array(void * ctx);
static int ajv_end_array(void * ctx);
static int ajv_null(void * ctx);
static int ajv_boolean(void * ctx, int booleanValue);
static const ajv_node * orderly_subsumed_by (const orderly_node_type a, 
                                             const ajv_node *b);




/* is type a subsumed by type b? (everything is subsumed by any) */
static const ajv_node * orderly_subsumed_by (const orderly_node_type a, 
                                             const ajv_node *b) {
  const ajv_node *cur;
  switch (b->node->t) {
  case orderly_node_union:   
    for (cur = b->child; cur; cur = cur->sibling) {
      const ajv_node *ret = orderly_subsumed_by(a,cur);
      if (ret) return ret;
    }
    return NULL;

  case orderly_node_empty:   return a == b->node->t ? b : NULL; 
  case orderly_node_null:    return a == b->node->t ? b : NULL; 
  case orderly_node_string:  return a == b->node->t ? b : NULL; 
  case orderly_node_integer: return a == b->node->t ? b : NULL; 
  case orderly_node_boolean: return a == b->node->t ? b : NULL; 
  case orderly_node_number:  
    return a == b->node->t || a == orderly_node_integer ? b : NULL;
  case orderly_node_object:  return a == b->node->t ? b : NULL; 
  case orderly_node_array:   return a == b->node->t ? b : NULL;
  case orderly_node_any:     return b;
  }  
  return NULL;
}



int ick_strcmp(const char *a, const char *b, unsigned int blen);

#define AJV_STATE(x)                    \
  struct ajv_state_t *state = (struct ajv_state_t *) x;


#define FAIL_TYPE_MISMATCH(s, node, type) do {                  \
    ajv_set_error(s,ajv_e_type_mismatch,                        \
                  node, orderly_node_type_to_string(type));     \
    return 0; } while (0);                                      \


#define FAIL_TRAILING_INPUT(s) do {                             \
    ajv_set_error(s, ajv_e_trailing_input,                      \
                  NULL,NULL);                                   \
    return 0; } while (0);

#define FAIL_NOT_IN_LIST(s,n,k) do {                              \
    ajv_set_error(s, ajv_e_illegal_value, n, k);                  \
      return 0;} while (0);

#define FAIL_OUT_OF_RANGE(s,n) do {        \
    ajv_set_error(s, ajv_e_out_of_range, n, NULL);  \
    return 0;} while (0);

#define FAIL_UNEXPECTED_KEY(s,n,k) do {                       \
    ajv_set_error(s,ajv_e_unexpected_key,n,(const char *)k);  \
    return 0;} while (0);

#define FAIL_MISSING_ELEMENT(s,n,k) do {             \
    ajv_set_error(s,ajv_e_incomplete_container,n,k); \
    return 0;} while (0);
  
#define DO_TYPECHECK(s, t, n) do {                                      \
    if (s->finished) { FAIL_TRAILING_INPUT(s) };                        \
    const ajv_node * typecheck = n;                                     \
    typecheck = orderly_subsumed_by(t, typecheck);                      \
    if (! typecheck ) FAIL_TYPE_MISMATCH(s, s->node, t); } while (0);


static const yajl_callbacks ajv_callbacks = {
  ajv_null,
  ajv_boolean, 
  ajv_integer, 
  ajv_double, 
  NULL,
  ajv_string,
  ajv_start_map,
  ajv_map_key,
  ajv_end_map,
  ajv_start_array,
  ajv_end_array
};

static void push_state(ajv_state state) {
  /* only maps and array have children */
  assert(state->node->node->t == orderly_node_object
         || state->node->node->t == orderly_node_array);
  state->node->seen = 1;
  state->node = state->node->child;
}

static void pop_state(ajv_state state) {
  /* silly corner case : we're popping a toplevel 'any' objects */
  if (state->node->parent == NULL) { 
    return;
  }
  state->node = state->node->parent;
  return;
}

YAJL_API yajl_handle ajv_alloc(const yajl_callbacks * callbacks,
                               const yajl_parser_config * config,
                               const yajl_alloc_funcs * allocFuncs,
                               void * ctx,
                               const char *schema) {
  const orderly_alloc_funcs * AF = (const orderly_alloc_funcs *) allocFuncs;
  
  orderly_reader r = orderly_reader_new(NULL);
  const orderly_node *n;
  n = orderly_read(r, ORDERLY_UNKNOWN, schema, strlen(schema));
  if (!n) {
    fprintf(stderr, "Schema is invalid: %s\n%s\n", orderly_get_error(r),
            orderly_get_error_context(r, schema, strlen(schema)));
  }


  {
    static orderly_alloc_funcs orderlyAllocFuncBuffer;
    static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;
    
    if (orderlyAllocFuncBufferPtr == NULL) {
            orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
            orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
    }
    AF = orderlyAllocFuncBufferPtr;

    }

  
  struct ajv_state_t *ajv_state = 
    (struct ajv_state_t *)
    OR_MALLOC(AF, sizeof(struct ajv_state_t));
  memset((void *) ajv_state, 0, sizeof(struct ajv_state_t));
  ajv_state->AF = AF;
  ajv_state->node = 
    ajv_alloc_tree((const orderly_alloc_funcs *)AF, n, NULL);

  ajv_state->any.parent = ajv_state->any.child = ajv_state->any.sibling = NULL;
  ajv_state->any.node = orderly_alloc_node((orderly_alloc_funcs *)AF, 
                                           orderly_node_any);
  ajv_state->cb = callbacks;
  ajv_state->cbctx = ctx;

  return yajl_alloc(&ajv_callbacks,
                    config,
                    allocFuncs,
                    (void *)ajv_state);
}




static int ajv_start_map (void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on;
  DO_TYPECHECK(state, orderly_node_object, state->node);

  on = state->node->node;

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    state->depth++;
  }  else {
    push_state(state);
  }

  if (state->cb && state->cb->yajl_start_map) {
    return state->cb->yajl_start_map(state->cbctx);
  } else {
    return 1;
  }

}

static int ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen) {
  AJV_STATE(ctx);
  ajv_node *cur;
  if (state->node->node->t == orderly_node_any) {

  } else {
    const char *this_is_utf8_dont_do_math = (const char *)key;
    assert(orderly_node_object == state->node->parent->node->t);

    for (cur = state->node->parent->child; cur; cur = cur->sibling) {
      assert(cur->node->name);
      if (!strncmp(cur->node->name,this_is_utf8_dont_do_math,stringLen)) {
        state->node = cur; 
        /* point at the schema for this key, as the next callback will needs it */
        break;
      }
    }
  
    if ( !cur ) {
      /* we found a key which we don't have an associated schema for
       * if the object forbids this, throw an error */
      if ( state->node->parent->node->additional_properties )  {
        FAIL_UNEXPECTED_KEY(state, state->node->parent, key);
      }
      /* otherwise, put us into schemaless mode */
      
      assert(state->depth == 0);
      state->any.parent = state->node;
      state->node = &(state->any);
    }
  }

  if (state->cb && state->cb->yajl_map_key) {
    return state->cb->yajl_map_key(state->cbctx, key, stringLen);
  } else {
    return 1;
  }
}

/*
 *
 */
static int synthesize_callbacks (struct ajv_state_t *state, orderly_json *value) {
  int ret = 1; 

  /* No callbacks, no work */
  if (!state->cb) {
    return 1;
  }
    
  switch (value->t) {

    /*
     * NULL
     */
  case orderly_json_null:
    if (state->cb->yajl_null) {
      ret = state->cb->yajl_null(state->cbctx);
    }
    break;

    /*
     * boolean
     */
  case orderly_json_boolean:
    if (state->cb->yajl_boolean) {
      ret = state->cb->yajl_boolean(state->cbctx,value->v.b);
    }
    break;
    
    /*
     * string
     */
  case orderly_json_string:

    if (state->cb->yajl_string) {
      ret = state->cb->yajl_string(state->cbctx,
                                   (unsigned char *)value->v.s,
                                   strlen(value->v.s));
    }
    break;

    /*
     * object / map
     */
  case orderly_json_object:
    /* call the start callback */
    if (state->cb->yajl_start_map) {
      ret = state->cb->yajl_start_map(state->cbctx);
      if (ret == 0) {
        return 0;
      }
    }
    /* recurse - XXX: stack depth limit */
    {
      orderly_json *kiditr;
      for (kiditr = value->v.children.first; kiditr; kiditr = kiditr->next) {
        ret = synthesize_callbacks(state, kiditr);
        if (ret == 0) {
          return 0;
        }
      }
    }
    /* call end callback */
    if (state->cb->yajl_end_map) {
      ret = state->cb->yajl_end_map(state->cbctx);
      if (ret == 0) {
        return 0;
      }
    }
    break;


    /*
     * array
     */
  case orderly_json_array:
    /* call start callback */
    if (state->cb->yajl_start_array) {
      ret = state->cb->yajl_start_array(state->cbctx);
      if (ret == 0) break;
    }
    /* recurse */
    {
      orderly_json *kiditr;
      for (kiditr = value->v.children.first; kiditr; kiditr = kiditr->next) {
        ret = synthesize_callbacks(state, kiditr);
        if (ret == 0) break;
      }
    }
    /* call end callback */
    if (state->cb->yajl_end_array) {
      ret = state->cb->yajl_end_array(state->cbctx);
    }

    break;

    /* 
     * integer
     */
  case orderly_json_integer:
    if (state->cb->yajl_number) {
      assert("unimplemented" == 0);
    } else if (state->cb->yajl_integer) {
      ret = state->cb->yajl_integer(state->cbctx, value->v.i);
    }

    break;

    /* 
     * "number" -- float
     */
  case orderly_json_number:
    if (state->cb->yajl_number) {
      assert("unimplemented" == 0);
    } else if (state->cb->yajl_double) {
      ret = state->cb->yajl_double(state->cbctx, value->v.n);
    }

    break;



    /* XXX: orderly_json_none appears to be for internal accounting
       only, should never find a default of this type */
  case orderly_json_none:
  default:
    assert("unreachable"==0);
  }


  return ret;
}


static int ajv_end_map(void * ctx) {
  AJV_STATE(ctx);
  ajv_node *cur;
  if (state->node->node->t == orderly_node_any) {
    state->depth--;
    if (state->depth == 0) { pop_state(state); }
  } else {
    /* this initialization assume pop_state hasn't happened yet */
    for (cur = state->node->parent->child; cur; cur = cur->sibling) {
      if (cur->required || !(cur->node->optional)) {
        if (!cur->seen) {
          if (cur->node->default_value) {
            int ret;
            ret = synthesize_callbacks(state, cur->node->default_value);
            if (ret == 0) {
              return 0;
            }
          } else {
            FAIL_MISSING_ELEMENT(state,cur,NULL);
          }
        }
      }
    }
    /* i.e. careful with this */
    pop_state(state);
  }

  if (state->cb && state->cb->yajl_end_map) {
    return state->cb->yajl_end_map(state->cbctx);
  } else {
    return 1;
  }

}
        


static int ajv_integer(void * ctx, long integerValue) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_integer, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    if (state->depth == 0) pop_state(ctx);
  } else {
    if (ORDERLY_RANGE_SPECIFIED(on->range)) {
      if (ORDERLY_RANGE_HAS_LHS(on->range)) {
        if (on->range.lhs.i > integerValue) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
      if (ORDERLY_RANGE_HAS_RHS(on->range)) {
        if (on->range.rhs.i < integerValue) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
    }

    if (on->values) {
      orderly_json *cur;
      int found = 0;
      assert(on->values->t == orderly_json_array); /* docs say so */
      for (cur = on->values->v.children.first; cur ; cur = cur->next) {
        assert(cur->t == orderly_json_integer);
        if (integerValue == cur->v.i) {
          found = 1;
        }
      }
      if (found == 0) {
        FAIL_NOT_IN_LIST(state,state->node,NULL); /* XXX: deparse int ? */
      }
    }
  }

  if (state->cb && state->cb->yajl_integer) {
    return state->cb->yajl_integer(state->cbctx,integerValue);
  } else {
    return 1;
  }
  
}

static int ajv_null(void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_null, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    if (state->depth == 0) pop_state(ctx);
  }   

  if (state->cb && state->cb->yajl_null) {
    return state->cb->yajl_null(state->cbctx);
  } else {
    return 1;
  }
}


static int ajv_boolean(void * ctx, int booleanValue) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_boolean, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    if (state->depth == 0) pop_state(ctx);
  }

  if (state->cb && state->cb->yajl_boolean) {
    return state->cb->yajl_boolean(state->cbctx, booleanValue);
  } else {
    return 1;
  }

}

static int ajv_double(void * ctx, double doubleval) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state, orderly_node_number, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    if (state->depth == 0) pop_state(ctx);
  } else {
    const orderly_node *on = state->node->node;
  
    if (on->t != orderly_node_number) {
      FAIL_TYPE_MISMATCH(state,state->node, orderly_node_number);
    }
    if (ORDERLY_RANGE_SPECIFIED(on->range)) {
      if (ORDERLY_RANGE_HAS_LHS(on->range)) {
        /* Strictly greater than, orderly spec is vague,
         * json-schema.org is source */
        if (on->range.lhs.i > doubleval) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
      if (ORDERLY_RANGE_HAS_RHS(on->range)) {
        if (on->range.rhs.i < doubleval) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
    }
    state->node->seen = 1;
    
    if (on->values) {
      orderly_json *cur;
      int found = 0;
      assert(on->values->t == orderly_json_array); /* docs say so */
      for (cur = on->values->v.children.first; cur ; cur = cur->next) {
        assert(cur->t == orderly_json_number);
        if (doubleval == cur->v.n) {
          found = 1;
        }
      }
      if (found == 0) {
        FAIL_NOT_IN_LIST(state,state->node,NULL); /* XXX: deparse number? */
      }
    }
  }

  if (state->cb && state->cb->yajl_double) {
    return state->cb->yajl_double(state->cbctx,doubleval);
  } else {
    return 1;
  }
  
}

int ick_strcmp(const char *a, const char *b, unsigned int blen) {
  while (*a) {
    if (blen == 0) { break; }
    if (*a != *b) { return 1; }
    a++; b++; blen--;
  }
  if (*a != '\0') { return 1; }
  return 0;
}

/** strings are returned as pointers into the JSON text when,
 * possible, as a result, they are _not_ null padded */
static int ajv_string(void * ctx, const unsigned char * stringVal,
                unsigned int stringLen) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_string, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    if (state->depth == 0) pop_state(ctx);
  } else {

    if (on->t != orderly_node_string) {
      FAIL_TYPE_MISMATCH(state,state->node,orderly_node_string);
    }
    if (ORDERLY_RANGE_SPECIFIED(on->range)) {
      if (ORDERLY_RANGE_HAS_LHS(on->range)) {
        if (on->range.lhs.i > stringLen) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
      if (ORDERLY_RANGE_HAS_RHS(on->range)) {
        if (on->range.rhs.i < stringLen) {
          FAIL_OUT_OF_RANGE(state,state->node);
        }
      }
    }

    if (on->values) {
      orderly_json *cur;
      int found = 0;
      assert(on->values->t == orderly_json_array); /* docs say so */
      for (cur = on->values->v.children.first; cur ; cur = cur->next) {
        assert(cur->t == orderly_json_string);
        if (!ick_strcmp(cur->v.s, (const char *)stringVal, stringLen)) {
          found = 1;
        }
      }
      if (found == 0) {
        FAIL_NOT_IN_LIST(state,state->node, NULL);
      }
    }

    if (on->regex) {
      /* TODO: validate regex */
    }
  
  }

  if (state->cb && state->cb->yajl_string) {
    return state->cb->yajl_string(state->cbctx,stringVal,stringLen);
  } else {
    return 1;
  }
  
}


static int ajv_start_array(void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_array, state->node);

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    state->depth++;
  } else {
    push_state(state);
  }

  if (state->cb && state->cb->yajl_start_array) {
    return state->cb->yajl_start_array(state->cbctx);
  } else {
    return 1;
  }
}


static int ajv_end_array(void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;

  state->node->seen = 1;

  if (on->t == orderly_node_any) {
    state->depth--;
    if (state->depth == 0) pop_state(ctx);
  /* with tuple typed nodes, we need to check that we've seen things */
  } else {

    if (state->node->parent->node->tuple_typed) {
      ajv_node *cur;
      for (cur = state->node->parent->child; cur; cur = cur->sibling) {
        if (!cur->seen) {
          if (cur->node->default_value) {
            int ret;
            ret = synthesize_callbacks(state, cur->node->default_value);
            if (ret == 0) { /*parse was cancelled */
              return 0;
            }
          } else { 
            FAIL_MISSING_ELEMENT(state,cur,NULL);
          }
        }
      }
    }

    pop_state(state);
  
  }

  
  if (state->cb && state->cb->yajl_end_array) {
    return state->cb->yajl_end_array(state->cbctx);
  } else {
    return 1;
  }
}




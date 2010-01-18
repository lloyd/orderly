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
int ick_strcmp(const char *a, const char *b, unsigned int blen);

#define AJV_STATE(x)                    \
  struct ajv_state_t *state = (struct ajv_state_t *) x;

/* XXX: takes args, do something here */
#define VALIDATE_FAILED(x,y,z) { fprintf(stderr,"VALIDATE FAILED: %s\n",x);\
                                 return 0;}

static const yajl_callbacks ajv_callbacks = {
  ajv_null, /* yajl_null */
  ajv_boolean, /* XXX ? boolean */
  ajv_integer, /* integer */
  ajv_double, /* double */
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
  /* only maps and array have children */
  assert(state->node->parent != NULL);
  state->node = state->node->parent;
}

YAJL_API yajl_handle ajv_alloc(const yajl_callbacks * callbacks,
                               const yajl_parser_config * config,
                               const yajl_alloc_funcs * allocFuncs,
                               void * ctx,
                               const char *schema) {
  const orderly_alloc_funcs *AF = (const orderly_alloc_funcs *) allocFuncs;

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
    if (AF == NULL) {
      AF = 
        (const orderly_alloc_funcs *)orderlyAllocFuncBufferPtr;
    }
  }

  struct ajv_state_t *ajv_state = 
    (struct ajv_state_t *)
    OR_MALLOC(AF, sizeof(struct ajv_state_t));

  ajv_state->node = 
    ajv_alloc_node_recursive((const orderly_alloc_funcs *)AF, n, NULL);

  ajv_state->cb = callbacks;
  ajv_state->cbctx = ctx;
  return yajl_alloc(&ajv_callbacks,
                    config,
                    allocFuncs,
                    (void *)ajv_state);
}




static int ajv_start_map (void * ctx) {
  AJV_STATE(ctx);

  if (state->node) {
    push_state(state);
  }  else {
    state->depth++;
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
  if (state->node) {
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
      if ( ! state->node->parent->node->additional_properties )  {
        VALIDATE_FAILED("found unknown key in map", key,stringLen);
      }
      /* otherwise, put us into schemaless mode */
      assert(state->depth == 0);
      state->valid_node = state->node;
      state->node = NULL;
    }
  } else {
    /* NOP, not at toplevel by definition */
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
    /* TODO FIGURE THESE out */
  case orderly_json_none:
  case orderly_json_null:
  case orderly_json_boolean:
    /* TODO: */
    assert(0==1);
    break;
    
  case orderly_json_string:
    /* call the callback if need be */
    if (state->cb->yajl_string) {
      ret = state->cb->yajl_string(state->cbctx,
                                   (unsigned char *)value->v.s,
                                   strlen(value->v.s));
    }
    break;

  case orderly_json_object:
    /* call the start callback */
    if (state->cb->yajl_start_map) {
      ret = state->cb->yajl_start_map(state->cbctx);
      if (ret == 0) {
        return 0;
      }
    }
    /* recurse */
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

  case orderly_json_array:
    /* call start callback */
    if (state->cb->yajl_start_array) {
      ret = state->cb->yajl_start_array(state->cbctx);
      if (ret == 0) {
        return 0;
      }
    }
    /* recurse */
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
    if (state->cb->yajl_end_array) {
      ret = state->cb->yajl_end_array(state->cbctx);
      if (ret == 0) {
        return 0;
      }
    }

    break;


  case orderly_json_integer:
    
    if (state->cb->yajl_number) {
      assert("unimplemented" == 0);

    } else if (state->cb->yajl_integer) {

      ret = state->cb->yajl_integer(state->cbctx, value->v.i);

      if (ret == 0) {
        return 0;
      }

    }

    break;

  case orderly_json_number:

    if (state->cb->yajl_number) {
      assert("unimplemented" == 0);

    } else if (state->cb->yajl_double) {

      ret = state->cb->yajl_double(state->cbctx, value->v.n);

      if (ret == 0) {
        return 0;
      }

    }

    break;

  default:
    assert("unreachable"==0);
  }


  return ret;
}


static int ajv_end_map(void * ctx) {
  AJV_STATE(ctx);
  ajv_node *cur;
  if (state->node) {

    for (cur = state->node->parent->child; cur; cur = cur->sibling) {
      if (cur->required || !(cur->node->optional)) {
        if (!cur->seen) {
          if (cur->node->default_value) {
            int ret;
            ret = synthesize_callbacks(state, cur->node->default_value);
            if (ret == 0) { /*parse was cancelled */
              return 0;
            }
          } else {
            VALIDATE_FAILED("missing required map element",
                          "didn't see", "This guy");
          }
        }
      }
    }
    pop_state(state);

  } else {
    /* we're in a schemaless map, restore the schema pointer */
    state->depth--;
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  }

  
  if (state->cb && state->cb->yajl_end_map) {
    return state->cb->yajl_end_map(state->cbctx);
  } else {
    return 1;
  }

}
        

static int ajv_integer(void * ctx, long integerValue) {
  AJV_STATE(ctx);
  if (!state->node) {
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  } else {
    const orderly_node *on = state->node->node;
  
    if (on->t != orderly_node_integer) {
      VALIDATE_FAILED("wrong_type",orderly_node_number,on->t);
    }
    /* TODO validate range */
    state->node->seen = 1;
  }



  if (state->cb && state->cb->yajl_integer) {
    return state->cb->yajl_integer(state->cbctx,integerValue);
  } else {
    return 1;
  }
  
}

static int ajv_null(void * ctx) {
  AJV_STATE(ctx);
  if (!state->node) {
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  } else {
    state->node->seen = 1;
  }

  if (state->cb && state->cb->yajl_null) {
    return state->cb->yajl_null(state->cbctx);
  } else {
    return 1;
  }

}


static int ajv_boolean(void * ctx, int booleanValue) {
  AJV_STATE(ctx);
  if (!state->node) {
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  } else {
    state->node->seen = 1;
  }

  if (state->cb && state->cb->yajl_boolean) {
    return state->cb->yajl_boolean(state->cbctx, booleanValue);
  } else {
    return 1;
  }

}

static int ajv_double(void * ctx, double doubleval) {
  AJV_STATE(ctx);
  if (!state->node) {
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  } else {
    const orderly_node *on = state->node->node;
  
    if (on->t != orderly_node_number) {
      VALIDATE_FAILED("wrong_type",orderly_node_number,on->t);
    }
    /* TODO validate range */
    state->node->seen = 1;
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

  if (!state->node)  {
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
  } else {
    const orderly_node *on = state->node->node;

    if (on->t != orderly_node_string) {
      VALIDATE_FAILED("wrong_type",orderly_node_string,on->t);
    }
    if (ORDERLY_RANGE_SPECIFIED(on->range)) {
      if (ORDERLY_RANGE_HAS_LHS(on->range)) {
        if (on->range.lhs.i > stringLen) {
  
          VALIDATE_FAILED("out_of_range",on->range.lhs.i,stringLen);
        }
      }
      if (ORDERLY_RANGE_HAS_RHS(on->range)) {
        if (on->range.rhs.i < stringLen) {

          VALIDATE_FAILED("out_of_range",on->range.rhs.i,stringLen);
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
        VALIDATE_FAILED("invalid_value",ook,ook);
      }
    }

    if (on->regex) {
      /* TODO: validate regex */
    }
  
    state->node->seen = 1;
  }

  if (state->cb && state->cb->yajl_string) {
    return state->cb->yajl_string(state->cbctx,stringVal,stringLen);
  } else {
    return 1;
  }
  
}


static int ajv_start_array(void * ctx) {
  AJV_STATE(ctx);
  if (!state->node)  {

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

  if (!state->node) {

    state->depth--;
    if (state->depth == 0) {
      state->node = state->valid_node;
    }
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
            VALIDATE_FAILED("missing tuple elements", "didn't see", "This guy");
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

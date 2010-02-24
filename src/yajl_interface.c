/*
 * Copyright 2010, LLoyd Hilaiel.
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
 *  3. Neither the name of Greg Olszewski and Lloyd Hilaiel nor the names of its
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
#include "ajv_schema.h"
#include "yajl_interface.h"
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
static int pass_ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen);
static int pass_ajv_start_map (void * ctx);
static int pass_ajv_end_map(void * ctx);
static int pass_ajv_integer(void * ctx, long integerValue);
static int pass_ajv_double(void * ctx, double value);

static int pass_ajv_string(void * ctx, const unsigned char * stringVal,
               unsigned int stringLen);
static int pass_ajv_start_array(void * ctx);
static int pass_ajv_end_array(void * ctx);
static int pass_ajv_null(void * ctx);
static int pass_ajv_boolean(void * ctx, int booleanValue);
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
                  node, orderly_node_type_to_string(type),      \
                  strlen(orderly_node_type_to_string(type)));   \
                  return 0; } while (0);                        \


#define FAIL_REGEX_NOMATCH(s, node,regex) do {                   \
    ajv_set_error(s,ajv_e_regex_failed,                          \
                  node, regex, strlen(regex));                   \
    return 0; } while (0);                                       \


#define FAIL_NOT_IN_LIST(s,n,k,kl) do {                                 \
    ajv_set_error(s, ajv_e_illegal_value, n, k, kl);                    \
      return 0;} while (0);

#define FAIL_OUT_OF_RANGE(s,n,d) do {                           \
    char buf[128];                                              \
    snprintf(buf,128,"%99g",d);                                 \
    ajv_set_error(s, ajv_e_out_of_range, n, buf,strlen(buf));   \
    return 0;} while (0);

#define FAIL_UNEXPECTED_KEY(s,n,k,kl) do {                      \
    ajv_set_error(s,ajv_e_unexpected_key,n,(const char *)k,kl); \
    return 0;} while (0);

#define FAIL_MISSING_ELEMENT(s,n,k) do {             \
    ajv_set_error(s,ajv_e_incomplete_container,n,k,strlen(k));  \
    return 0;} while (0);
  
#define DO_TYPECHECK(st, t, n) do {                                     \
    const ajv_node *tcn;                                                      \
    if (!(tcn = ajv_do_typecheck(st,t,n))) return 0;                    \
    n = tcn;                                                            \
    on = tcn->node;                                                     \
  } while(0);                                                           \

#define AJV_SUFFIX(type,...)                                    \
  if (state->cb && state->cb->yajl_##type) {                    \
    return state->cb->yajl_##type(state->cbctx, __VA_ARGS__);   \
  } else {                                                      \
    return 1;                                                   \
  }                                                             \

#define AJV_SUFFIX_NOARGS(type)                                 \
  if (state->cb && state->cb->yajl_##type) {                    \
    return state->cb->yajl_##type(state->cbctx);                \
  } else {                                                      \
    return 1;                                                   \
  }                                                             \


static const ajv_node *
ajv_do_typecheck(ajv_state state, orderly_node_type t, const ajv_node *node) {
  const ajv_node * typecheck = node;                       
  if (state->error.code != ajv_e_no_error) {
    assert("got a yajl callback while in an error state" == 0);
    /* NORETURN */
  }
  
  if (ajv_state_finished(state)) { 
    ajv_set_error(state, ajv_e_trailing_input, NULL, NULL, 0);
  }
  
  typecheck = orderly_subsumed_by(t, typecheck);
  
  if (! typecheck ) { 
    ajv_set_error(state,ajv_e_type_mismatch,node->node->name ? node : NULL, 
                  orderly_node_type_to_string(t),      
                  strlen(orderly_node_type_to_string(t)));   
    return NULL;
  }

  return typecheck;
}

const yajl_callbacks ajv_callbacks = {
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


const yajl_callbacks ajv_passthrough = {
  pass_ajv_null,
  pass_ajv_boolean, 
  pass_ajv_integer, 
  pass_ajv_double, 
  NULL,
  pass_ajv_string,
  pass_ajv_start_map,
  pass_ajv_map_key,
  pass_ajv_end_map,
  pass_ajv_start_array,
  pass_ajv_end_array
};

static int pass_ajv_null(void * ctx) {
  AJV_STATE(ctx);
  AJV_SUFFIX_NOARGS(null);
}
static int pass_ajv_boolean(void * ctx, int booleanValue) {
  AJV_STATE(ctx);
  AJV_SUFFIX(boolean,booleanValue);
}
static int pass_ajv_double(void * ctx, double doubleval) {
  AJV_STATE(ctx);
  AJV_SUFFIX(double,doubleval);
}

static int pass_ajv_integer(void * ctx, long integerValue) {
  AJV_STATE(ctx);
  AJV_SUFFIX(integer,integerValue);
}
static int pass_ajv_string(void * ctx, const unsigned char * stringVal,
                           unsigned int stringLen) {
  AJV_STATE(ctx);
  AJV_SUFFIX(string,stringVal,stringLen);
}
static int pass_ajv_start_array(void * ctx) {
   AJV_STATE(ctx);
   AJV_SUFFIX_NOARGS(start_array);
}
static int pass_ajv_end_array(void * ctx) {
   AJV_STATE(ctx);
   AJV_SUFFIX_NOARGS(end_array);
}
static int pass_ajv_start_map(void * ctx) {
   AJV_STATE(ctx);
   AJV_SUFFIX_NOARGS(start_map);
}
static int pass_ajv_end_map(void * ctx) {
   AJV_STATE(ctx);
   AJV_SUFFIX_NOARGS(end_map);
}
static int pass_ajv_map_key(void * ctx, const unsigned char * key, 
                            unsigned int stringLen) {
  AJV_STATE(ctx);
  AJV_SUFFIX(map_key,key,stringLen);
}


static int ajv_null(void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on;
  DO_TYPECHECK(state,orderly_node_null, state->node);


  if (on->t == orderly_node_any) {
    if (state->depth == 0) { ajv_state_mark_seen(state, state->node); }
  } else {
    ajv_state_mark_seen(state, state->node);
  }

  AJV_SUFFIX_NOARGS(null);
}


static int ajv_boolean(void * ctx, int booleanValue) {
  AJV_STATE(ctx);
  const orderly_node *on;
  DO_TYPECHECK(state,orderly_node_boolean, state->node);

  if (on->t == orderly_node_any) {
    if (state->depth == 0) {  ajv_state_mark_seen(state, state->node); }
  } else {
    ajv_state_mark_seen(state, state->node);
  }
  if (on->values) {
    orderly_json *cur;
    int found = 0;
    assert(on->values->t == orderly_json_array); /* docs say so */
    for (cur = on->values->v.children.first; cur ; cur = cur->next) {
      if (cur->t == orderly_json_boolean) {
        if (cur->v.b == booleanValue) {
          found = 1;
        }
      }
    }
    if (found == 0) {
      FAIL_NOT_IN_LIST(state,state->node,
                       booleanValue ? "true" : "false",
                       booleanValue ? 4 : 5);
    }
  }

  AJV_SUFFIX(boolean,booleanValue);
}

static int ajv_double(void * ctx, double doubleval) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state, orderly_node_number, state->node);

  if (on->t == orderly_node_any) {
    if (state->depth == 0) { ajv_state_mark_seen(state, state->node); }
  } else {
  
    if (on->t != orderly_node_number) {
      FAIL_TYPE_MISMATCH(state,state->node, orderly_node_number);
    }
    if (ORDERLY_RANGE_SPECIFIED(on->range)) {
      orderly_range r = on->range;
      if (ORDERLY_RANGE_HAS_LHS(on->range)) {
        /* Strictly greater than, orderly spec is vague,
         * json-schema.org is source */
        if (((ORDERLY_RANGE_RHS_DOUBLE & r.info) 
             ? r.lhs.d : (double)r.lhs.i) > doubleval) { 
          FAIL_OUT_OF_RANGE(state,state->node,doubleval);
        }
      }
      if (ORDERLY_RANGE_HAS_RHS(on->range)) {
        if (((ORDERLY_RANGE_RHS_DOUBLE & r.info) 
             ? r.rhs.d : (double)r.rhs.i) < doubleval) { 
          FAIL_OUT_OF_RANGE(state,state->node,doubleval);
        }
      }
    }
    ajv_state_mark_seen(state, state->node);
    
  }
  if (on->values) {
    orderly_json *cur;
    int found = 0;
    assert(on->values->t == orderly_json_array); /* docs say so */
    for (cur = on->values->v.children.first; cur ; cur = cur->next) {
      if (cur->t == orderly_json_number) {
        if (doubleval == cur->v.n) {
          found = 1;
        }
      } else if (cur->t == orderly_json_integer) {
        if (doubleval == (double)cur->v.i) {
          found = 1;
        }
      }
    }
    if (found == 0) {
      char doublestr[128];
      snprintf(doublestr,128,"%99g",doubleval);
      FAIL_NOT_IN_LIST(state,state->node,doublestr,strlen(doublestr)); 
    }
  }
  
  AJV_SUFFIX(double,doubleval);
}

static int ajv_integer(void * ctx, long integerValue) {
  AJV_STATE(ctx);
  const orderly_node *on = state->node->node;
  DO_TYPECHECK(state,orderly_node_integer, state->node);

  if (on->t == orderly_node_any) {
    if (state->depth == 0) {  ajv_state_mark_seen(state, state->node); }
  } else {
    ajv_state_mark_seen(state, state->node);
    if (!ajv_check_integer_range(state,state->node,integerValue)) {
      return 0;
    }
  }
  if (on->values) {
    orderly_json *cur;
    int found = 0;
    assert(on->values->t == orderly_json_array); /* docs say so */
    for (cur = on->values->v.children.first; cur ; cur = cur->next) {
      if (cur->t == orderly_json_integer) {
        if (integerValue == cur->v.i) {
          found = 1;
          break;
        }
      } else {
        if ((double)integerValue == cur->v.n) {
          found = 1;
          break;
        }
      }
    }
    if (found == 0) {
      char buf[128];
      snprintf(buf,128,"%ld",integerValue);
      FAIL_NOT_IN_LIST(state,state->node,buf,strlen(buf));
    }
  }

  AJV_SUFFIX(integer,integerValue);
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


  if (on->t == orderly_node_any) {
    if (state->depth == 0)  { ajv_state_mark_seen(state, state->node); }
  } else {
    ajv_state_mark_seen(state, state->node);
    if (on->t != orderly_node_string) {
      FAIL_TYPE_MISMATCH(state,state->node,orderly_node_string);
    }
    if (!ajv_check_integer_range(state,state->node,stringLen)) {
      return 0;
    }
    
    if (state->node->regcomp) {
      int pcrecode;
      pcrecode = pcre_exec(state->node->regcomp,NULL,
                           (char *)stringVal,stringLen,0,0,NULL,0);
      if (pcrecode < 0) {
        if (pcrecode == PCRE_ERROR_NOMATCH) {
          FAIL_REGEX_NOMATCH(state,state->node,on->regex);
        }
      }
    }
    if (state->node->checker) {
      if (!state->node->checker((const char *)stringVal,stringLen)) {
        ajv_set_error(state, ajv_e_invalid_format, state->node, 
                      (char *)stringVal, stringLen);
        return 0;
      }
    }

  }
  
  if (on->values) {
    orderly_json *cur;
    int found = 0;
    assert(on->values->t == orderly_json_array); /* docs say so */
    for (cur = on->values->v.children.first; cur ; cur = cur->next) {
      if (cur->t == orderly_json_string) {
        if (!ick_strcmp(cur->v.s, (const char *)stringVal, stringLen)) {
          found = 1;
        }
      }
    }
    if (found == 0) {
      FAIL_NOT_IN_LIST(state,state->node, (const char *)stringVal,stringLen);
    }
  }


  AJV_SUFFIX(string,stringVal,stringLen);
}


static int ajv_start_array(void * ctx) {
   AJV_STATE(ctx);
   const orderly_node *on = state->node->node;
   DO_TYPECHECK(state,orderly_node_array, state->node);

   if (on->t == orderly_node_any) {
     state->depth++;
   } else {
     ajv_state_push(state,state->node);
   }

   AJV_SUFFIX_NOARGS(start_array);
 }


 static int ajv_end_array(void * ctx) {
   AJV_STATE(ctx);
   const orderly_node *on = state->node ? state->node->node : NULL;
   if (on && on->t == orderly_node_any && state->depth > 0) {
     state->depth--;
     if (state->depth == 0 ) { ajv_state_mark_seen(state, state->node); }
   } else {
     if (!ajv_state_array_complete(state)) {
       return 0;
     }
     ajv_state_mark_seen(state, state->node);
   }   
   AJV_SUFFIX_NOARGS(end_array);
 }


static int ajv_start_map (void * ctx) {
  AJV_STATE(ctx);
  const orderly_node *on;
  DO_TYPECHECK(state, orderly_node_object, state->node);

  on = state->node->node;

  if (on->t == orderly_node_any) {
    state->depth++;
  }  else {
    ajv_state_push(state,state->node);
  }

  AJV_SUFFIX_NOARGS(start_map);
}


static int ajv_map_key(void * ctx, const unsigned char * key, 
                       unsigned int stringLen) {
  AJV_STATE(ctx);
  ajv_node *cur;
  if (state->node && state->node->node->t == orderly_node_any
      && state->depth != 0) {
    /* skip straight to return section */ 
  } else {
    const char *this_is_utf8_dont_do_math = (const char *)key;
    cur = ajv_find_key(ajv_state_parent(state), this_is_utf8_dont_do_math,
                       stringLen);

    if ( cur ){
      state->node = cur;
      if (cur->node->requires) {
        const char **n;
        for (n = cur->node->requires; *n; n++) {
          ajv_node *req = ajv_find_key(ajv_state_parent(state), 
                                       *n,
                                       strlen(*n));
          ajv_state_require(state,req);
        }
      }
    } else {
      if ( ajv_state_parent(state)->node->additional_properties == orderly_node_empty )  {
        /* we found a key which we don't have an associated schema for
         * if the object forbids this, throw an error */
        FAIL_UNEXPECTED_KEY(state, ajv_state_parent(state), key,stringLen);
      } else {  
        /* otherwise, put us into schemaless mode */
        ((orderly_node *)(state->any.node))->t = ajv_state_parent(state)->node->additional_properties; 
        state->depth = 0;
        state->any.parent = ajv_state_parent(state);
        state->node = &(state->any);
      }
    }
  }

  AJV_SUFFIX(map_key,key,stringLen);
}

static int ajv_end_map(void * ctx) {
  AJV_STATE(ctx);

  if (state->node && state->node->node->t == orderly_node_any
      && state->depth > 0) {
    state->depth--;
    if (state->depth == 0) { ajv_state_mark_seen(state, state->node); }
  } else {
    if (!ajv_state_map_complete(state,ajv_state_parent(state))) {
      return 0;
    }
  }
  AJV_SUFFIX_NOARGS(end_map);
}
        

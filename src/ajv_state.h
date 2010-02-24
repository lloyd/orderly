/*
 * Copyright 2010, Greg Olszewski and Lloyd Hilaiel.
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
< * 
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

#ifndef __AJV_STATE_H__
#define __AJV_STATE_H__

#include "api/ajv_parse.h"
#include "orderly_alloc.h"
#include "api/node.h"
#include "orderly_ptrstack.h"
#include <pcre.h>


typedef struct ajv_node_t {
  /* these pointers mirror the structure of the nodes they contain */
  struct ajv_node_t * sibling;
  struct ajv_node_t * child;
  /* all children point to their parents */
  const struct ajv_node_t * parent;
  /* the orderly node we wrap */
  const orderly_node *node;
  /* a "format" callback */
  ajv_format_checker checker;
  /* a compiled regex for string nodes */
  pcre *regcomp;
  /* a ptrstack of required elements */
  orderly_ptrstack required;
} ajv_node;




/*
 * Enough information to describe an error
 */

typedef enum {
  ajv_e_no_error,
  ajv_e_type_mismatch,
  ajv_e_trailing_input,
  ajv_e_out_of_range,
  ajv_e_regex_failed, /* didn't match regex */
  ajv_e_incomplete_container, /* incopmlete tuple-array, map, or document */
  ajv_e_illegal_value, /* value found din't match enumerated list */
  ajv_e_unexpected_key, /* only valid if additional_properties == 0 XXX ?*/
  ajv_e_invalid_format, /* format checker returned invalid */
} ajv_error;

struct ajv_error_t  {
  ajv_error code;
  const ajv_node *node; 
  char *extra_info; /* TODO union */
};

typedef struct ajv_node_state_t {
  orderly_ptrstack seen;
  orderly_ptrstack required;
  const ajv_node   *node;
} * ajv_node_state;

typedef struct ajv_state_t {
  yajl_handle             yajl;
  ajv_schema              s;
  /* pointer into the node tree. if it is of type any, consult depth
  **/
  const ajv_node                *node;
  /* populated with a orderly_node of type any, used for representing 
   * unknown map keys
   */
  ajv_node                any;
  yajl_callbacks            ourcb;
  const orderly_alloc_funcs *AF;
  struct ajv_error_t        error;
  const yajl_callbacks      *cb;

  void                      *cbctx;
  unsigned int            depth;
  orderly_ptrstack        node_state;
  const yajl_parser_config *ypc;
} * ajv_state;



struct ajv_schema_t {
  ajv_node *root;
  orderly_node  *oroot;
  const orderly_alloc_funcs *af;
  
};
void ajv_state_push(ajv_state state, const ajv_node *n);
void ajv_state_pop(ajv_state state);
int ajv_state_map_complete (ajv_state state, const ajv_node *map);
int ajv_state_array_complete (ajv_state state);
ajv_node * ajv_alloc_tree(const orderly_alloc_funcs * alloc,
                          const orderly_node *n, ajv_node *parent);

ajv_node * ajv_alloc_node( const orderly_alloc_funcs * alloc, 
                           const orderly_node *on,    ajv_node *parent ) ;

void ajv_free_node (const orderly_alloc_funcs * alloc, ajv_node ** n);

void ajv_reset_node( ajv_node * n);


void ajv_set_error ( ajv_state s, ajv_error e,
                     const ajv_node * node, const char *info, int length );

void ajv_clear_error (ajv_state  s);

const char * ajv_error_to_string (ajv_error e);

void ajv_free_node_state( const orderly_alloc_funcs * alloc, 
                          ajv_node_state *node);

ajv_node_state ajv_alloc_node_state( const orderly_alloc_funcs * alloc, 
                                     const ajv_node *node);

void ajv_state_mark_seen(ajv_state s, const ajv_node *node) ;
int ajv_state_finished(ajv_state state);
const ajv_node * ajv_state_parent(ajv_state state);
void ajv_state_require(ajv_state state, ajv_node *req) ;
int ajv_check_integer_range(ajv_state state, const ajv_node *an, long l);

#endif

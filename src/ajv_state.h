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
< * 
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
#include <yajl/yajl_parse.h>
#include "orderly_alloc.h"
#include "api/node.h"

typedef struct ajv_node_t {
  /* these pointers mirror the structure of the nodes they contain */
  struct ajv_node_t * sibling;
  struct ajv_node_t * child;
  /* all children point to their parents */
  struct ajv_node_t * parent;
  /* the orderly node we wrap */
  orderly_node *node;
  /* have we seen data matching this node?
   * (only valid on nodes whose parent is a map/object, or tuple array)
   */
  int    seen;
  /* overrides the optional flag in the orderly node (for requires support)
   */
  int    required;
} ajv_node;

typedef struct ajv_state_t {
  ajv_node                *node;
  const unsigned char     *last_error;
  const yajl_callbacks    *cb;
  void                    *cbctx;
} * ajv_state;


ajv_node * 
ajv_alloc_node_recursive(orderly_alloc_funcs * alloc, 
                         orderly_node * n,
                         ajv_node *parent );

ajv_node * 
ajv_alloc_node(orderly_alloc_funcs * alloc);

void 
ajv_free_node (orderly_alloc_funcs * alloc, ajv_node ** n);

void 
ajv_reset_node( ajv_node * n);




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

#include "orderly_json_parse.h"
#include "orderly_parse.h"
#include "orderly_ptrstack.h"
#include "orderly_alloc.h"
#include "orderly_json.h"

#include <yajl/yajl_parse.h>
#include <string.h>
#include <pcre.h>

static void
interject_defaults(orderly_alloc_funcs *alloc, orderly_node *node) {
  orderly_node *d;
  switch (node->t) {
  case orderly_node_empty: 
  case orderly_node_null: 
  case orderly_node_string:
  case orderly_node_boolean:
  case orderly_node_any:
  case orderly_node_integer:
  case orderly_node_number:
  case orderly_node_object:
    break;
  case orderly_node_union: /* XXX: eek? */
    break;
  case orderly_node_array:
    if (! node->child ) {
      d = orderly_alloc_node(alloc,orderly_node_any);
      node->child = d;
    }
  }
}

static orderly_json_parse_status
parse_json_schema(orderly_alloc_funcs * alloc,
                  orderly_json * j, orderly_node ** n)
{
    orderly_json_parse_status s = orderly_json_parse_s_ok;
    
    orderly_json * k;
    *n = NULL;
    if (j->t != orderly_json_object) {
        /* XXX: offset into the buffer!? */
        return orderly_json_parse_s_object_expected;
    }

    *n = orderly_alloc_node(alloc, orderly_node_empty);
    (*n)->additional_properties = orderly_node_any;

    for (k=j->v.children.first; k != NULL; k=k->next) {
        if (k->k != NULL) {
            if (!strcmp(k->k, "type")) {
                if (k->t == orderly_json_string) {
                    (*n)->t = orderly_string_to_node_type(k->v.s, strlen(k->v.s));
                    if ((*n)->t == orderly_node_empty) {
                        s = orderly_json_parse_s_invalid_type_value;
                        goto toErrIsHuman;
                    }
                } else if (k->t == orderly_json_array) {
                    /* support items containing an *array* of schema
                     * for tuple typing */
                    orderly_json * pj = NULL;
                    orderly_node ** last = &((*n)->child);
                    (*n)->t = orderly_node_union;

                    for (pj = k->v.children.first; pj; pj = pj->next)
                    {
                        s = parse_json_schema(alloc, pj, last);
                        if (s != orderly_json_parse_s_ok) {
                            goto toErrIsHuman;
                        }
                        last = &((*last)->sibling);
                    }
                } else {
                    s = orderly_json_parse_s_type_expects_string_or_array;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "properties")) {
                orderly_json * p = NULL;
                orderly_node ** last = &((*n)->child);

                if (k->t != orderly_json_object) {                
                    s = orderly_json_parse_s_invalid_properties_value;
                    goto toErrIsHuman;
                }

                for (p=k->v.children.first; p != NULL; p=p->next) {
                    orderly_node * pn = NULL;
                    s = parse_json_schema(alloc, p, &pn);
                    if (pn) {
                        *last = pn;
                        last = &(pn->sibling);
                        BUF_STRDUP(pn->name, alloc, p->k, strlen(p->k));
                    }
                    
                    if (s != orderly_json_parse_s_ok) {
                        goto toErrIsHuman;
                    }
                }
            }
            else if (!strcmp(k->k, "items")) {
                /* support items containing an *array* of schema
                *  for tuple typing */
                if (k->t == orderly_json_array) {
                    orderly_json * pj = NULL;
                    orderly_node ** last = &((*n)->child);
                    (*n)->tuple_typed = 1;
                    for (pj = k->v.children.first; pj; pj = pj->next)
                    {
                        s = parse_json_schema(alloc, pj, last);
                        if (s != orderly_json_parse_s_ok) {
                            goto toErrIsHuman;
                        }
                        last = &((*last)->sibling);
                    }
                } else if (k->t == orderly_json_object) {
                    orderly_node * pn = NULL;
                    s = parse_json_schema(alloc, k, &pn);
                    if (s != orderly_json_parse_s_ok) {
                        goto toErrIsHuman;
                    }
                    (*n)->child = pn;
                } else {
                    s = orderly_json_parse_s_items_gets_object_or_array;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "optional")) {
                if (k->t != orderly_json_boolean) {
                    s = orderly_json_parse_s_invalid_optional_value;
                    goto toErrIsHuman;
                }
                (*n)->optional = k->v.b;
            }
            else if (!strcmp(k->k, "minimum")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_LHS_INT;
                    (*n)->range.lhs.i = k->v.i;
                } else if (k->t == orderly_json_number) {
                    (*n)->range.info |= ORDERLY_RANGE_LHS_DOUBLE;
                    (*n)->range.lhs.d = k->v.n;
                } else {
                    s = orderly_json_parse_s_minimum_requires_number;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "maximum")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_RHS_INT;
                    (*n)->range.rhs.i = k->v.i;
                } else if (k->t == orderly_json_number) {
                    (*n)->range.info |= ORDERLY_RANGE_RHS_DOUBLE;
                    (*n)->range.rhs.d = k->v.n;
                } else {
                    s = orderly_json_parse_s_maximum_requires_number;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "minLength")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_LHS_INT;
                    (*n)->range.lhs.i = k->v.i;
                } else {
                    s = orderly_json_parse_s_minlength_requires_integer;
                    goto toErrIsHuman;
                }

            }
            else if (!strcmp(k->k, "maxLength")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_RHS_INT;
                    (*n)->range.rhs.i = k->v.i;
                } else {
                    s = orderly_json_parse_s_maxlength_requires_integer;
                    goto toErrIsHuman;
                }

            }
            else if (!strcmp(k->k, "minItems")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_LHS_INT;
                    (*n)->range.lhs.i = k->v.i;
                } else {
                    s = orderly_json_parse_s_minitems_requires_integer;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "maxItems")) {
                if (k->t == orderly_json_integer) {
                    (*n)->range.info |= ORDERLY_RANGE_RHS_INT;
                    (*n)->range.rhs.i = k->v.i;
                } else {
                    s = orderly_json_parse_s_maxitems_requires_integer;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "additionalProperties")) {
                if (k->t == orderly_json_boolean) {
                    (*n)->additional_properties = 
                      k->v.b 
                      ? orderly_node_any
                      : orderly_node_empty;
                } else if (k->t == orderly_json_object
                           && !strcmp(k->v.children.first->k, "type")) {
                  if (k->v.children.first->t == orderly_json_string) {
                      (*n)->additional_properties = 
                        orderly_string_to_node_type(k->v.children.first->v.s, 
                                                    strlen(k->v.children.first->v.s));
                    } else {
                      s = orderly_json_parse_s_invalid_type_value;
                      goto toErrIsHuman;
                    }
                } else {
                  s = orderly_json_parse_s_addprop_requires_boolean;
                }
            }
            else if (!strcmp(k->k, "default")) {
                /* clone */
                (*n)->default_value = orderly_clone_json(alloc, k);
                /* remove the key */
                OR_FREE(alloc, (char *) (*n)->default_value->k);
                (*n)->default_value->k = NULL;

            }
            else if (!strcmp(k->k, "enum")) {
                /* clone */
                (*n)->values = orderly_clone_json(alloc, k);
                /* remove the key */
                OR_FREE(alloc, (char *) (*n)->values->k);
                (*n)->values->k = NULL;
            }
            else if (!strcmp(k->k, "pattern")) {
                if (k->t == orderly_json_string) {
                    pcre *regex;
                    const char *regerror;
                    int erroffset;
                    int error_code = 0;
                    BUF_STRDUP((*n)->regex, alloc, k->v.s, strlen(k->v.s));
                    regex = pcre_compile2((*n)->regex,
                                          0,
                                          &error_code,
                                          &regerror,
                                          &erroffset,
                                          NULL);
                    if (regex) {
                      pcre_free(regex);
                    }
                    if (error_code != 0) {
                      s = orderly_parse_s_regex_error + error_code;
                      goto toErrIsHuman;
                    }
                } else {
                    s = orderly_json_parse_s_pattern_requires_string;
                    goto toErrIsHuman;
                }
            }
            else if (!strcmp(k->k, "requires")) {
                if ((*n)->requires) {
                    s = orderly_json_parse_s_duplicate_requires;
                    goto toErrIsHuman;
                }
                
                if (k->t == orderly_json_string) {
                    (*n)->requires = OR_MALLOC(alloc, 2 * sizeof(char *));
                    BUF_STRDUP((*n)->requires[0], alloc, k->v.s, strlen(k->v.s));
                    (*n)->requires[1] = NULL;
                } else if (k->t == orderly_json_array) {
                    unsigned int num = 0;
                    orderly_json * ks;
                    
                    for (ks = k->v.children.first; ks; ks = ks->next)
                    {
                        unsigned int i;
                        char ** p;

                        if (ks->t != orderly_json_string) {
                            s = orderly_json_parse_s_requires_value_error;
                            goto toErrIsHuman;
                        }
                        num++;
                        p = OR_MALLOC(alloc, sizeof(char *) * (num + 1));
                        for (i = 0; i < num - 1; i++) p[i] = (char *) (*n)->requires[i];
                        BUF_STRDUP(p[i], alloc, ks->v.s, strlen(ks->v.s));
                        p[++i] = NULL;
                        if ((*n)->requires) OR_FREE(alloc, (*n)->requires);
                        (*n)->requires = (const char **) p;
                    }
                }
            }
            else if (!strcmp(k->k, "$schema")) {
              /* XXX: detect schema version, and store it somewhere!?,
                 then go back and adjust things */
              

            } else {
                orderly_json ** jPtr;
                    
                /* all unexpected properties are passed through using
                 * the passthrough_properties element of the node. */
                /* XXX: should this behavior be configurable? */
                if ((*n)->passthrough_properties == NULL) {
                    (*n)->passthrough_properties =
                        orderly_alloc_json(alloc, orderly_json_object);
                }
                jPtr = &((*n)->passthrough_properties->v.children.first);
                while (*jPtr) jPtr = &((*jPtr)->next);
                /* clone onto the passthrough object */
                (*jPtr) = orderly_clone_json(alloc, k);
            }
        }
    }
    
    /* json schema has some implied defaults, insert them */
    interject_defaults(alloc,*n);

    return s;

  toErrIsHuman:
    if (*n) orderly_free_node(alloc, n);
    return s;
}

orderly_json_parse_status
orderly_json_parse(orderly_alloc_funcs * alloc,
                   const unsigned char * schemaText,
                   const unsigned int schemaTextLen,
                   const char **error_message,
                   orderly_node ** n,
                   unsigned int * final_offset)
{
    /* a high level interface to non-stream based json parsing */
    orderly_json_parse_status s;
    orderly_json * j;

    *final_offset = schemaTextLen;
    
    j = orderly_read_json(alloc, (const char *) schemaText, final_offset);
    if (j == NULL) return orderly_json_parse_s_invalid_json;
    
    /* we've parsed the json into a memory representation, now let's
     * interpret it's semantic meaning as we merge it over into a
     * orderly_node representation */
    s = parse_json_schema(alloc, j, n);

    (void) orderly_free_json(alloc, &j);
    
    return s;
}

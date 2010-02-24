#include "api/ajv_parse.h"
#include "ajv_state.h"
#include "ajv_schema.h"
#include "orderly_alloc.h"
#include <string.h>
#include <assert.h>
typedef struct { 
  char *name;
  ajv_format_checker checker;
} checker_tuple;

orderly_ptrstack format_checkers = { NULL, 0, 0};

ajv_node * ajv_alloc_node( const orderly_alloc_funcs * alloc, 
                           const orderly_node *on,    ajv_node *parent ) 
{
  ajv_node *n = (ajv_node *)OR_MALLOC(alloc, sizeof(ajv_node));
  memset((void *) n, 0, sizeof(ajv_node));
  orderly_ps_init(n->required);
  const char *regerror = NULL;
  int erroffset;
  n->parent = parent;
  n->node   = on;
  if (on->regex) {
    n->regcomp = pcre_compile(on->regex,
                              0,
                              &regerror,
                              &erroffset,
                              NULL);
  }
  {
    const char *formatname = ajv_node_format(on);
    int i;
    if (formatname) {
      for (i = 0; i < orderly_ps_length(format_checkers); i++) {
        checker_tuple *chk = format_checkers.stack[i];
        if (!strcmp(formatname, chk->name)) {
          n->checker = chk->checker;
        }
      }
    }
  }
  return n;
}

const char *ajv_node_format(const orderly_node *on) {
  if (on->passthrough_properties
      && on->passthrough_properties->t == orderly_json_object ) {
    orderly_json *cur;
    
    for (cur = on->passthrough_properties->v.children.first; cur; cur = cur->next) {
      if (!strcmp(cur->k, "format")) {
        return cur->v.s;
      }
    }
  }
  return NULL;
}

ajv_node * ajv_alloc_tree(const orderly_alloc_funcs * alloc,
                          const orderly_node *n, ajv_node *parent) {

  ajv_node *an = ajv_alloc_node(alloc, n, parent);

  if (n->sibling) an->sibling = ajv_alloc_tree(alloc,n->sibling,parent);
  if (n->child)   an->child   = ajv_alloc_tree(alloc,n->child, an);
  if (n->t == orderly_node_object) {
    ajv_node *cur;
    for (cur = an->child; cur; cur = cur->sibling) {
      if (!cur->node->optional) {
        orderly_ps_push(alloc, an->required, cur);
      }
    }
  }
  
  return an;
}

void ajv_free_node (const orderly_alloc_funcs * alloc, ajv_node ** n) {
  if (n && *n) {
    if ((*n)->sibling) ajv_free_node(alloc,&((*n)->sibling));
    if ((*n)->child) ajv_free_node(alloc,&((*n)->child));
    /* the orderly_node *  belongs to the schema, don't free it */
    if ((*n)->regcomp) {
      pcre_free((*n)->regcomp);
    }
    OR_FREE(alloc, *n);
    *n = NULL;
  }
}


ajv_schema
ajv_alloc_schema(orderly_alloc_funcs *alloc, orderly_node *parsed) {
  const orderly_alloc_funcs * AF = (const orderly_alloc_funcs *) alloc;
  {
    static orderly_alloc_funcs orderlyAllocFuncBuffer;
    static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;
    
    if (orderlyAllocFuncBufferPtr == NULL) {
            orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
            orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
    }
    AF = orderlyAllocFuncBufferPtr;
    
  }

  
  struct ajv_schema_t *ret = 
    (struct ajv_schema_t *)
    OR_MALLOC(AF, sizeof(struct ajv_state_t));
  if (ret) {
    memset((void *) ret, 0, sizeof(struct ajv_schema_t));
    ret->root = 
      ajv_alloc_tree((const orderly_alloc_funcs *)AF, parsed, NULL);
    ret->oroot = parsed;
    ret->af = AF;
  }

  return ret;
}

void ajv_free_schema(ajv_schema schema) {
  ajv_free_node(schema->af, &schema->root);
  orderly_free_node(schema->af, &schema->oroot);
  OR_FREE(schema->af, schema);

}  


ajv_node *ajv_find_key(const ajv_node *map, const char *key, unsigned int len) {
  ajv_node *cur;
  
  for (cur = map->child; cur; cur = cur->sibling) {
    assert(cur->node->name);
    if (!strncmp(cur->node->name,key,len)) {
      break;
      }
  }
  return cur;
}

void ajv_register_format(const char *name, ajv_format_checker checker) {
  static orderly_alloc_funcs orderlyAllocFuncBuffer;
  checker_tuple *chk;  
  static orderly_alloc_funcs * orderlyAllocFuncBufferPtr = NULL;

  if (orderlyAllocFuncBufferPtr == NULL) {
    orderly_set_default_alloc_funcs(&orderlyAllocFuncBuffer);
    orderlyAllocFuncBufferPtr = &orderlyAllocFuncBuffer;
  }
  chk = OR_MALLOC(orderlyAllocFuncBufferPtr,sizeof(checker_tuple));
  BUF_STRDUP(chk->name, orderlyAllocFuncBufferPtr, name, strlen(name));
  chk->checker = checker;
  orderly_ps_push(orderlyAllocFuncBufferPtr, format_checkers, chk);
}

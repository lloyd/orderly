#include <yajl/yajl_parse.h>
#include "ajv_state.h"
#include "api/ajv_parse.h"
#include <string.h>
#include <stdio.h>

ajv_node * ajv_alloc_node( const orderly_alloc_funcs * alloc, 
                           const orderly_node *on,    ajv_node *parent ) 
{
  ajv_node *n = (ajv_node *)OR_MALLOC(alloc, sizeof(ajv_node));
  memset((void *) n, 0, sizeof(ajv_node));
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
    
  return n;
}
  
ajv_node * ajv_alloc_tree(const orderly_alloc_funcs * alloc,
                          const orderly_node *n, ajv_node *parent) {

  ajv_node *an = ajv_alloc_node(alloc, n, parent);

  if (n->sibling) an->sibling = ajv_alloc_tree(alloc,n->sibling,parent);
  if (n->child)   an->child   = ajv_alloc_tree(alloc,n->child, an);
  
  return an;
}

void ajv_free_node ( orderly_alloc_funcs * alloc, ajv_node ** n) {
  if (n && *n) {
    if ((*n)->sibling) ajv_free_node(alloc,&((*n)->sibling));
    if ((*n)->child) ajv_free_node(alloc,&((*n)->child));
    /* XXX: We leak the orderly_node tree */
    OR_FREE(alloc, *n);
    *n = NULL;
  }
}

void ajv_reset_node (ajv_node * n) {
  n->required = 0;
  n->seen     = 0;

  if (n->sibling) ajv_reset_node(n->sibling);
  if (n->child) ajv_reset_node(n->child);
}
  
void ajv_clear_error (ajv_state s) {

  s->error.code       = ajv_e_no_error;
  s->error.extra_info = NULL;
  s->error.node       = NULL;

  if (s->error.extra_info) OR_FREE(s->AF,s->error.extra_info);

}

void ajv_set_error ( ajv_state s, ajv_error e,
                     const ajv_node * node, const char *info ) {

  ajv_clear_error(s);
  s->error.node = node;
  s->error.code = e;
  if (info) {
    BUF_STRDUP(s->error.extra_info, 
               s->AF, info, strlen(info));
  }

}

const char * ajv_error_to_string (ajv_error e) {

  const char *outbuf;
  switch(e) {
  case ajv_e_type_mismatch:  
    outbuf = "type mismatch"; break;
  case ajv_e_trailing_input: 
    outbuf = "input continued validation completed"; break;
  case ajv_e_out_of_range:   
    outbuf = "value out of range"; break;
  case ajv_e_incomplete_container: 
    outbuf = "incomplete structure"; break;
  case ajv_e_illegal_value:  
    outbuf = "value not permitted"; break;
  case ajv_e_regex_failed:  
    outbuf = "string did not match regular expression"; break;
  case ajv_e_unexpected_key: 
    outbuf = "key not permitted"; break;
  default:                   
    outbuf = "Internal error: unrecognized error code"; 
  };
  
  return outbuf;
}

#define ERROR_BASE_LENGTH 1024
unsigned char * ajv_get_error(yajl_handle hand, int verbose,
                              const unsigned char * jsonText,
                              unsigned int jsonTextLength) {
  const char * yajl_err = "";
  char * ret;
  ajv_state s = (ajv_state)yajl_context(hand);

  struct ajv_error_t *e = &(s->error);

  int yajl_length;
  int max_length;

  if (e->code == ajv_e_no_error) { 
    return yajl_get_error(hand,verbose,jsonText,jsonTextLength);
  } 

  /* include the yajl error message when verbose */
  if (verbose == 1) {
    yajl_err = 
      (const char *)yajl_get_error(hand,verbose,
                                   (unsigned char *)jsonText,jsonTextLength);

    yajl_length = strlen(yajl_err);
  }

  max_length  = ERROR_BASE_LENGTH;
  max_length += e->extra_info ? strlen(e->extra_info) : 1;
  max_length += yajl_length;

  ret = OR_MALLOC(s->AF, max_length+1);
  ret[0] = '\n';
  strcat(ret, "VALIDATION ERROR:");
  if (e->node && e->node->node->name) {
    strcat(ret," value for map key '");
    strcat(ret,e->node->node->name); /* XXX */
    strcat(ret,"':");
  }
  strcat(ret, (const char *)ajv_error_to_string(e->code));
  /** TODO: produce more details, info is available */
  strcat(ret,"\n");
  strcat(ret,yajl_err);
  return (unsigned char *)ret;
}

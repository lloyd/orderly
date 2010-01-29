#include "api/ajv_parse.h"
#include "ajv_state.h"
#include "orderly_alloc.h"
#include <string.h>


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

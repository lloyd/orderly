#if 0
typedef enum {
  ajv_type_mismatch;
  ajv_missing_element;
  ajv_invalid_element;
  ajv_extra_element;
} yajlv_error_type;

struct yajlv_error {
  ajv_error_type type;
  orderly_node_type found;
  orderly_node_type wanted;
  orderly_node **missing;
}

#define ORDERLY_NODE(node) node->node

int typecheck(ajv_state *state, 
              orderly_node_type found, orderly_node* wanted ) {
  if (found == wanted) {
    return 0;
  }
  
  if (wanted == orderly_node_union) {
    
  }
  
}
#define AJV_STATE(x)                    \
  ajv_state *state = (struct ajv_state) x;
  

#endif
const int fuckyoustdc = 1;

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
  

static void push_state(ajv_state * state) {
  /* only maps and array have children */
  ASSERT(state->node->node->t == orderly_node_object
         || state->node->node->t == orderly_node_array);
  state->node = state->node->child;
}

static void pop_state(ajv_state * state) {
  ajv_node *cur;
  /* only maps and array have children */
  ASSERT(state->node->parent != NULL);
  state->node = state->node->parent;
}
#endif

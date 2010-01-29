#ifndef __YAJL_INTERFACE_H__
#define __YAJL_INTERFACE_H__

int orderly_synthesize_callbacks (const yajl_callbacks *cb, 
                                  void *cbctx,
                                  orderly_json *value);
  
extern const yajl_callbacks ajv_callbacks;


#endif

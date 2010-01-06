#include <yajl/yajl_parse.h>
#ifndef __AJV_PARSE_H__
#define __AJV_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif
YAJL_API yajl_handle ajv_alloc(const yajl_callbacks * callbacks,
                               const yajl_parser_config * config,
                               const yajl_alloc_funcs * allocFuncs,
                               void * ctx,
                               const char *schema);


#ifdef __cplusplus
};
#endif
#endif

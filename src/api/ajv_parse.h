#include <yajl/yajl_parse.h>
#include "common.h"
#ifndef __AJV_PARSE_H__
#define __AJV_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    yajl_parser_config yajl_config;
  } ajv_parser_config;

  /* Allocate a validating parser handle
   * callbacks, allocFuncs, and ctx are identical to yajl_gen_parser.
   * arguments are identical to yajl_gen_parser.
   * The extra argument references a schema that is used for validation.

   */
ORDERLY_API yajl_handle ajv_alloc(const yajl_callbacks * callbacks,
                                  const yajl_parser_config * config,
                                  const yajl_alloc_funcs * allocFuncs,
                                  void * ctx,
                                  const char *schema);

ORDERLY_API unsigned char * ajv_get_error(yajl_handle hand, int verbose,
                                          const unsigned char * jsonText,
                                          unsigned int jsonTextLength);


#ifdef __cplusplus
};
#endif
#endif

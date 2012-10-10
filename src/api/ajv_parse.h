#include <yajl/yajl_parse.h>
#include "common.h"
#include "node.h"
#include "json.h"
#ifndef __AJV_PARSE_H__
#define __AJV_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   yajl_parser_config yajl_config;
} ajv_parser_config;

typedef struct ajv_schema_t * ajv_schema;
typedef struct ajv_state_t * ajv_handle;


  /* Allocate a validating parser handle
   * callbacks, allocFuncs, and ctx are identical to yajl_gen_parser.
   * arguments are identical to yajl_gen_parser.
   * The extra argument references a schema that is used for validation.

   */
ORDERLY_API ajv_handle ajv_alloc(const yajl_callbacks * callbacks,
                                 const yajl_parser_config * config,
                                 const yajl_alloc_funcs * allocFuncs,
                                 void * ctx);
  

ORDERLY_API void ajv_free(ajv_handle hand);

/** parsed belongs to the returned ajv_schema */
ORDERLY_API ajv_schema ajv_alloc_schema(orderly_alloc_funcs *alloc,
                                        orderly_node *parsed);

ORDERLY_API void ajv_free_schema(ajv_schema schema);

ORDERLY_API yajl_status ajv_validate(ajv_handle hand,
                                    ajv_schema schema,
                                    orderly_json *json);

ORDERLY_API yajl_status ajv_parse_and_validate(ajv_handle hand,
                                               const unsigned char * jsonText,
                                               unsigned int jsonTextLength,
                                               ajv_schema schema);


ORDERLY_API unsigned char * ajv_get_error(ajv_handle hand, int verbose,
                                          const unsigned char * jsonText,
                                          unsigned int jsonTextLength);

ORDERLY_API yajl_status ajv_parse_complete(ajv_handle hand);
ORDERLY_API void ajv_free_error(ajv_handle hand, unsigned char *err);
ORDERLY_API unsigned int ajv_get_bytes_consumed(ajv_handle hand);
typedef int (*ajv_format_checker)(const char *string, unsigned int length);
  /* Register a callback to give meaning to value for the "format" 
     key. */
    
ORDERLY_API void ajv_register_format(const char *name, ajv_format_checker checker);
  
#ifdef __cplusplus
};
#endif
#endif

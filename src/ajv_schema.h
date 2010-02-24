#ifndef __AJV_SCHEMA_H__
#define __AJV_SCHEMA_H__
#include "ajv_state.h"

ajv_node *ajv_find_key(const ajv_node *map, const char *key, unsigned int len);

const char *ajv_node_format(const orderly_node *on);
#endif

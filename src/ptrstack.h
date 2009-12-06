/*
 * originally by lloyd, put in the public domain.  (relicense this as
 * your own tyler, to keep the licensing of the py ext clean)
 */ 

/*
 * A header only, highly efficient custom pointer stack implementation, used in
 * orderly to maintain parse state.
 */

#ifndef __ORDERLY_BYTESTACK_H__
#define __ORDERLY_BYTESTACK_H__

#include <Python.h>
#include "assert.h"

#define ORDERLY_PS_INC 128

typedef struct orderly_bytestack_t
{
    PyObject ** stack;
    unsigned int size;
    unsigned int used;
} orderly_bytestack;

/* initialize a bytestack */
#define orderly_ps_init(ops) {                  \
        (ops).stack = NULL;                     \
        (ops).size = 0;                         \
        (ops).used = 0;                         \
    }                                           \


/* initialize a bytestack */
#define orderly_ps_free(ops)                 \
    if ((ops).stack) free((ops).stack);   

#define orderly_ps_current(ops)               \
    (assert((ops).used > 0), (ops).stack[(ops).used - 1])

#define orderly_ps_length(ops) ((ops).used)

#define orderly_ps_push(ops, pointer) {                       \
    if (((ops).size - (ops).used) == 0) {               \
        (ops).size += ORDERLY_PS_INC;                      \
        (ops).stack = realloc((void *) (ops).stack, sizeof(PyObject *) * (ops).size); \
    }                                                   \
    (ops).stack[((ops).used)++] = (pointer);               \
}
    
/* removes the top item of the stack, returns nothing */
#define orderly_ps_pop(ops) { ((ops).used)--; }

#define orderly_ps_set(ops, pointer)                          \
    (ops).stack[((ops).used) - 1] = (pointer);             
    

#endif

/*
 * originally by lloyd, put in the public domain.  (relicense this as
 * your own tyler, to keep the licensing of the py ext clean)
 */

/*
 * A header only, highly efficient custom pointer stack implementation, used in
 * orderly to maintain parse state.
 */

#ifndef __ORDERLY_PTRSTACK_H__
#define __ORDERLY_PTRSTACK_H__

#include "assert.h"

#define ORDERLY_PS_INC 128

typedef struct orderly_ptrstack_t
{
    void ** stack;
    unsigned int size;
    unsigned int used;
} orderly_ptrstack;

/* initialize a ptrstack */
#define orderly_ps_init(ops) {                  \
        (ops).stack = NULL;                     \
        (ops).size = 0;                         \
        (ops).used = 0;                         \
    }


/* initialize a ptrstack */
#define orderly_ps_free(alloc, ops)                 \
    if ((ops).stack) OR_FREE(alloc, (ops).stack);

#define orderly_ps_current(ops)               \
    (assert((ops).used > 0), (ops).stack[(ops).used - 1])

#define orderly_ps_length(ops) ((ops).used)

#define orderly_ps_push(alloc, ops, pointer) {                              \
    if (((ops).size - (ops).used) == 0) {                                   \
        unsigned int oldsize = (ops).size;                                  \
        (ops).size += ORDERLY_PS_INC;                                       \
        {                                                                   \
            void * oldstack = (ops).stack;                                  \
            (ops).stack = OR_MALLOC((alloc), sizeof(void *) * (ops).size);  \
            if (oldstack) {                                                 \
                 memcpy((ops).stack, oldstack, sizeof(void*) * oldsize);    \
                 OR_FREE((alloc), oldstack);                                \
            }                                                               \
        }                                                                   \
    }                                                                       \
    (ops).stack[((ops).used)++] = (pointer);                                \
}

/* removes the top item of the stack, returns nothing */
#define orderly_ps_pop(ops) { ((ops).used)--; }

#define orderly_ps_set(ops, pointer)                          \
    (ops).stack[((ops).used) - 1] = (pointer);


#endif













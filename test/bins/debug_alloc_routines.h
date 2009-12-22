#ifndef __DEBUG_ALLOC_ROUTINES_H__
#define __DEBUG_ALLOC_ROUTINES_H__

#include "orderly/common.h"

/* memory debugging routines */
typedef struct 
{
    unsigned int numFrees;
    unsigned int numMallocs;
    /* XXX: we really need a hash table here with per-allocation
     *      information */ 
} orderlyTestMemoryContext;
 
/* cast void * into context */
#define TEST_CTX(vptr) ((orderlyTestMemoryContext *) (vptr))
 
static void orderlyTestFree(void * ctx, void * ptr)
{
    assert(ptr != NULL);
    TEST_CTX(ctx)->numFrees++;
    free(ptr);
}

 
static void * orderlyTestMalloc(void * ctx, unsigned int sz)
{
    assert(sz != 0);
    TEST_CTX(ctx)->numMallocs++;
    return malloc(sz);
}

 
static void * orderlyTestRealloc(void * ctx, void * ptr, unsigned int sz)
{
    if (ptr == NULL) {
        assert(sz != 0);
        TEST_CTX(ctx)->numMallocs++;
    }
    else if (sz == 0) {
        TEST_CTX(ctx)->numFrees++;
    }
 
    return realloc(ptr, sz);
}

#endif

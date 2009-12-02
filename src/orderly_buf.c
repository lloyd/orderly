/*
 * Copyright 2009, Lloyd Hilaiel.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "orderly_buf.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ORDERLY_BUF_INIT_SIZE 2048

struct orderly_buf_t {
    unsigned int len;
    unsigned int used;
    unsigned char * data;
    orderly_alloc_funcs * alloc;
};

static
void orderly_buf_ensure_available(orderly_buf buf, unsigned int want)
{
    unsigned int need;
    
    assert(buf != NULL);

    /* first call */
    if (buf->data == NULL) {
        buf->len = ORDERLY_BUF_INIT_SIZE;
        buf->data = (unsigned char *) OR_MALLOC(buf->alloc, buf->len);
        buf->data[0] = 0;
    }

    need = buf->len;

    while (want >= (need - buf->used)) need <<= 1;

    if (need != buf->len) {
        buf->data = (unsigned char *) OR_REALLOC(buf->alloc, buf->data, need);
        buf->len = need;
    }
}

orderly_buf orderly_buf_alloc(orderly_alloc_funcs * alloc)
{
    orderly_buf b = OR_MALLOC(alloc, sizeof(struct orderly_buf_t));
    memset((void *) b, 0, sizeof(struct orderly_buf_t));
    b->alloc = alloc;
    return b;
}

void orderly_buf_free(orderly_buf buf)
{
    assert(buf != NULL);
    if (buf->data) OR_FREE(buf->alloc, buf->data);
    OR_FREE(buf->alloc, buf);
}

void orderly_buf_append(orderly_buf buf, const void * data, unsigned int len)
{
    orderly_buf_ensure_available(buf, len);
    if (len > 0) {
        assert(data != NULL);
        memcpy(buf->data + buf->used, data, len);
        buf->used += len;
        buf->data[buf->used] = 0;
    }
}

void orderly_buf_clear(orderly_buf buf)
{
    buf->used = 0;
    if (buf->data) buf->data[buf->used] = 0;
}

const unsigned char * orderly_buf_data(orderly_buf buf)
{
    return buf->data;
}

unsigned int orderly_buf_len(orderly_buf buf)
{
    return buf->used;
}

void
orderly_buf_truncate(orderly_buf buf, unsigned int len)
{
    assert(len <= buf->used);
    buf->used = len;
}

void orderly_buf_append_string(orderly_buf buf, const char * s)
{
    orderly_buf_append(buf, (void *) s, strlen(s));
}


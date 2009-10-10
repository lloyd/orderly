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

#ifndef __ORDERLY_BUF_H__
#define __ORDERLY_BUF_H__

#include "api/orderly_common.h"
#include "orderly_alloc.h"

/*
 * Implementation/performance notes.  If this were moved to a header
 * only implementation using #define's where possible we might be 
 * able to sqeeze a little performance out of the guy by killing function
 * call overhead.  YMMV.
 */

/**
 * orderly_buf is a buffer with exponential growth.  the buffer ensures that
 * you are always null padded.
 */
typedef struct orderly_buf_t * orderly_buf;

/* allocate a new buffer */
orderly_buf orderly_buf_alloc(orderly_alloc_funcs * alloc);

/* free the buffer */
void orderly_buf_free(orderly_buf buf);

/* append a number of bytes to the buffer */
void orderly_buf_append(orderly_buf buf, const void * data, unsigned int len);

/* empty the buffer */
void orderly_buf_clear(orderly_buf buf);

/* get a pointer to the beginning of the buffer */
const unsigned char * orderly_buf_data(orderly_buf buf);

/* get the length of the buffer */
unsigned int orderly_buf_len(orderly_buf buf);

/* truncate the buffer */
void orderly_buf_truncate(orderly_buf buf, unsigned int len);

#endif

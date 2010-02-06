/*
 * Copyright 2009, 2010, Lloyd Hilaiel.
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

#ifndef __ORDERLY_READER_H__
#define __ORDERLY_READER_H__

#include "common.h"
#include "node.h"
#ifdef __cplusplus
extern "C" {
#endif    

typedef struct orderly_reader_t * orderly_reader;

/** allocate a new reader */
orderly_reader orderly_reader_new(const orderly_alloc_funcs * alloc);

/** release a reader */
void orderly_reader_free(orderly_reader *w);

/** read a schema */
const orderly_node * 
orderly_read(orderly_reader r, orderly_format fmt,
             const char * schema, unsigned int len);

/** claim responsibility for freeing an orderly_node * returned by orderly_read */
orderly_node * 
orderly_reader_claim(orderly_reader r, const orderly_node *);


/** when NULL is returned from orderly_read, this function can return
 *  an english, developer readable error code.  */
const char * orderly_get_error(orderly_reader r);

/** when NULL is returned from orderly_read, this function can return
 *  the location of the error in a human redable form, formatted in
 *  such a way that it can be displayed with a fixed width font.  The
 *  returned string is dynamically allocated and is valid until
 *  orderly_reader_free is called. */
const char * orderly_get_error_context(orderly_reader r,
                                       const char * schema, unsigned int len);

/** when NULL is returned from orderly_read, this function can return
 *  the location of the error as a numeric offset from the beginning of the
 *  schema buffer */
unsigned int orderly_get_error_offset(orderly_reader r);

#ifdef __cplusplus
}
#endif    
#endif

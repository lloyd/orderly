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

#include "orderly/writer.h"
#include "orderly/reader.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT_TEXT (1 << 20)
int
main(int argc, char ** argv) 
{
    /* XXX: 1 meg max schema size... */
    static char inbuf[MAX_INPUT_TEXT];
    size_t tot = 0, rd;
    while (0 < (rd = read(0, (void *) (inbuf + tot), MAX_INPUT_TEXT)))
    {
        tot += rd;
    }

    {
        orderly_writer w = orderly_writer_new(NULL);
        orderly_reader r = orderly_reader_new(NULL);
        const orderly_node * n;
        const char * schema;

        /* now read and parse the schema */
        n = orderly_read(r, ORDERLY_TEXTUAL, inbuf, tot);

printf("read hot foo: %p\n", n);
        
        /* now write the schema */
        schema = orderly_write(w, ORDERLY_TEXTUAL, n);
        
        if (schema) {
            write(0, schema, strlen(schema));
        }
        
        orderly_writer_free(&w);
        orderly_reader_free(&r);
    }
    
    return 0;
}

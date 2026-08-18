/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"
#include "libosns-eventloop.h"

#include "io.h"

/* BUFFER */

void IO_buffer_init(struct io_buffer_s *b)
{
    b->flags=0;
    b->bytesread=0;
    b->pos=0;
    b->size=0;
    b->ptr=NULL;
}

int IO_buffer_allocate(struct io_buffer_s *b, unsigned int size)
{
    int result=-1;

    if (size==0) {

        if (b->ptr && (b->flags & IO_BUFFER_FLAG_ALLOC)) free(b->ptr);
        IO_buffer_init(b);
        result=0;

    } else {

        b->ptr=realloc(b->ptr, size);

        if (b->ptr) {

            b->flags |= IO_BUFFER_FLAG_ALLOC;
            b->size=size;
            result=1;
            if (b->pos>size) b->pos=size;
            if (b->bytesread>size) b->bytesread=size;

        } else {

            IO_buffer_init(b);

        }

    }

    return result;

}

void IO_buffer_free(struct io_buffer_s *b)
{
    if (b->ptr && (b->flags & IO_BUFFER_FLAG_ALLOC)) free(b->ptr);
    IO_buffer_init(b);
}

void IO_buffer_memmove(struct io_buffer_s *bto, struct io_buffer_s *bfrom, unsigned int bytes2move)
{

    /* to */

    memmove((char *)(bto->ptr + bto->bytesread), bfrom->ptr, bytes2move);
    bto->bytesread+=bytes2move;

    /* from */

    if (bytes2move < bfrom->bytesread) {

        memmove(bfrom->ptr, (char *)(bfrom->ptr + bytes2move), (bfrom->bytesread - bytes2move));
        bfrom->bytesread-=bytes2move;

    } else {

        bfrom->bytesread=0;

    }

}

void IO_buffer_memmove_bytesread(struct io_buffer_s *b, unsigned int bytesread)
{

    if (bytesread < b->bytesread) {
        unsigned int bytes2move=(unsigned int)(b->bytesread - bytesread);

        memmove(b->ptr, (char *)(b->ptr + bytesread), bytes2move);
        b->bytesread=bytes2move;
        b->pos=((b->pos>bytesread) ? (b->pos - bytesread) : 0);

    } else {

        b->bytesread=0;
        b->pos=0;

    }

}

void IO_buffer_set(struct io_buffer_s *b, char *ptr, unsigned int size)
{
    b->ptr=ptr;
    b->size=size;
    b->bytesread=0;
    b->pos=0;
}

unsigned char IO_buffer_data_available(struct io_buffer_s *b)
{
    return (b && b->ptr && b->bytesread) ? 1 : 0;
}

unsigned char IO_buffer_cmp(struct io_buffer_s *a, struct io_buffer_s *b)
{
    unsigned char result=0;

    if (IO_buffer_data_available(a) && IO_buffer_data_available(b)) {

        result=((a->bytesread==b->bytesread) && (memcmp(a->ptr, b->ptr, a->bytesread)==0)) ? 1 : 0;

    }

    return result;
}

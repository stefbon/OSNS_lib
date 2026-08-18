/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_IO_BUFFER_H
#define LIB_IO_BUFFER_H

#define IO_BUFFER_FLAG_ALLOC                                            1
#define IO_BUFFER_FLAG_EOD                                             	2
#define IO_BUFFER_FLAG_ERROR                                            4
#define IO_BUFFER_FLAG_EOL                                              8

struct io_buffer_s {
    unsigned int                                                        flags;
    unsigned int                                                        bytesread;
    unsigned int                                                        pos;
    unsigned int                                                        size;
    char                                                                *ptr;
};

#define IM_BUFFER_INIT                                                  {0, 0, 0, 0, NULL}

/* prototypes */

void IO_buffer_init(struct io_buffer_s *b);
int IO_buffer_allocate(struct io_buffer_s *b, unsigned int size);
void IO_buffer_free(struct io_buffer_s *b);

void IO_buffer_memmove(struct io_buffer_s *bto, struct io_buffer_s *bfrom, unsigned int bytes2move);
void IO_buffer_memmove_bytesread(struct io_buffer_s *b, unsigned int bytesread);

void IO_buffer_set(struct io_buffer_s *b, char *ptr, unsigned int size);

unsigned char IO_buffer_data_available(struct io_buffer_s *b);
unsigned char IO_buffer_cmp(struct io_buffer_s *a, struct io_buffer_s *b);

#endif

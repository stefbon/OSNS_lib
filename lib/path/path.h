/*
  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Stef Bon <stefbon@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef LIB_FS_PATH_PATH_H
#define LIB_FS_PATH_PATH_H

#include "libosns-datatypes.h"

#define FS_PATH_BUFFER_ALLOC_DEFAULT_CHUNK_SIZE                 256

#define FS_PATH_FLAG_ALLOC				        1
#define FS_PATH_FLAG_BUFFER_ALLOC				2
#define FS_PATH_FLAG_PREPEND                                    4

struct fs_path_s {
    unsigned int						flags;
    unsigned char						back;
    struct dstr_s                                               start;
    unsigned int						size;
    char                                                        *buffer;
    unsigned int                                                (* calc_chunk_size)(struct fs_path_s *path, unsigned int size);
    unsigned int                                                (* append)(struct fs_path_s *path, char *b, unsigned int s, unsigned char addpathsep);
    unsigned int                                                (* prepend)(struct fs_path_s *path, char *b, unsigned int s, unsigned char addpathsep);
};

extern const struct fs_path_s fs_path_initializer;
#define FS_PATH_INIT					        fs_path_initializer

extern const struct fs_path_s fs_path_initializer_root;
#define FS_PATH_INIT_ROOT					fs_path_initializer_root

/* prototypes */

unsigned char FS_path_valid(struct fs_path_s *path);

void FS_path_set_bytes_raw(struct fs_path_s *path, char *buffer, unsigned int size, unsigned int length);
void FS_path_set_bytes(struct fs_path_s *path, char *buffer, unsigned int size, unsigned int length, unsigned int flags);
void FS_path_set(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned int flags);
void FS_path_clear(struct fs_path_s *path);

void FS_path_init(struct fs_path_s *path);

int FS_path_allocate(struct fs_path_s *path, unsigned int size);

unsigned int FS_path_export(struct fs_path_s *path, char *buffer, unsigned char endzero);
unsigned int FS_path_import(struct fs_path_s *path, char *buffer, unsigned int size, unsigned char endzero);

unsigned int FS_path_get_maximum_length();
struct dstr_s *FS_path_get_dstr(struct fs_path_s *path);

#endif

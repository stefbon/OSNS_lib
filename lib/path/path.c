/*
  2010, 2011, 2012, 2103, 2014, 2015, 2016, 2017 Stef Bon <stefbon@gmail.com>

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

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"

#include "path.h"
#include "append.h"
#include "utils.h"

unsigned char FS_path_valid(struct fs_path_s *path)
{

    /* test the part where the path starts is -> in <- the buffer : valid range */

    if ((path==NULL) || (path->buffer==NULL) || (path->start.str==NULL) || (path->start.str < path->buffer) || (path->start.str >= (path->buffer + path->size)) ||
        ((path->start.str + path->start.length) > (path->buffer + path->size))) return 0;

    return 1;
}

static unsigned int fs_path_calc_chunk_size_modulo(struct fs_path_s *p, unsigned int size)
{
    unsigned int totalsize=(p->size + size);
    unsigned int count = (totalsize / FS_PATH_BUFFER_ALLOC_DEFAULT_CHUNK_SIZE);

    if ((count * FS_PATH_BUFFER_ALLOC_DEFAULT_CHUNK_SIZE) < totalsize) count++;
    return ((count * FS_PATH_BUFFER_ALLOC_DEFAULT_CHUNK_SIZE) - p->size);
}

static unsigned int fs_path_calc_chunk_size_default(struct fs_path_s *p, unsigned int size)
{
    return size;
}

const struct fs_path_s fs_path_initializer = {
    .flags                              = 0,
    .back                               = 0,
    .start.str                          = NULL,
    .start.length                       = 0,
    .start.flags                        = 0,
    .size                               = 0,
    .buffer                             = NULL,
    .calc_chunk_size                    = fs_path_calc_chunk_size_default,
    .append                             = FS_path_append_nowrite,
    .prepend                            = FS_path_append_nowrite,
};

static char rootbuffer[2]={47, 0};

const struct fs_path_s fs_path_initializer_root = {
    .flags                              = 0,
    .back                               = 0,
    .start.str                          = rootbuffer,
    .start.length                       = 1,
    .start.flags                        = 0,
    .size                               = 2,
    .buffer                             = rootbuffer,
    .calc_chunk_size                    = fs_path_calc_chunk_size_default,
    .append                             = FS_path_append_nowrite,
    .prepend                            = FS_path_append_nowrite,
};

void FS_path_init(struct fs_path_s *path)
{
    if (path) memcpy(path, &fs_path_initializer, sizeof(struct fs_path_s));
}

void FS_path_set_bytes_raw(struct fs_path_s *path, char *buffer, unsigned int size, unsigned int length)
{

    FS_path_init(path);

    path->buffer=buffer;
    path->size=size;

    DSTR_set_bytes_raw(&path->start, buffer, length);

}

void FS_path_set_bytes(struct fs_path_s *path, char *buffer, unsigned int size, unsigned int length, unsigned int flags)
{

    FS_path_init(path);

    path->flags=flags;
    path->back=0;
    path->size=size;
    path->buffer=buffer;

    DSTR_set_bytes(&path->start, buffer, length);

    /* by default prepare the path to append */

    if (flags & FS_PATH_FLAG_PREPEND) {

        FS_path_prepend_init(path, 0);

    } else {

        FS_path_append_init(path, 0);

    }

}

void FS_path_set(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned int flags)
{
    struct dstr_s data=DSTR_INIT;

    if (FS_path_convert_type_ptr(type, ptr, &data)>0) FS_path_set_bytes(path, data.str, data.length, 0, flags);
}

void FS_path_clear(struct fs_path_s *path)
{

    if (path->flags & FS_PATH_FLAG_BUFFER_ALLOC) {

	free(path->buffer);
	path->flags &= ~FS_PATH_FLAG_BUFFER_ALLOC;

    }

    path->back=0;
    path->size=0;
    path->buffer=NULL;

    FS_path_init(path);

}

int FS_path_allocate(struct fs_path_s *path, unsigned int size)
{
    int result=-1;

    if (path==NULL) return -1;
    if (path->buffer && (path->size==size)) return 0;

    path->buffer=realloc(path->buffer, size);

    if (path->buffer) {

        memset(path->buffer, 0, size);
        path->size=size;

        path->start.str=path->buffer;
        path->start.length=0;
        path->flags |= FS_PATH_FLAG_BUFFER_ALLOC;
        result=1;

    } else {

        path->size=0;
        path->start.str=NULL;
        path->start.length=0;
        path->flags &= ~FS_PATH_FLAG_BUFFER_ALLOC;

    }

    return result;

}

unsigned int FS_path_export(struct fs_path_s *path, char *buffer, unsigned char endzero)
{
    unsigned int length=0;

    if (FS_path_valid(path)==0) return 0;
    length=path->start.length;

    if (buffer) {

        memcpy(buffer, path->start.str, length);

        if (endzero) {

            buffer[length]='\0';
            length++;

        }

    } else {

        length += (endzero ? 1 : 0);

    }

    return length;

}

unsigned int FS_path_import(struct fs_path_s *path, char *buffer, unsigned int size, unsigned char endzero)
{

    endzero=(endzero ? 1 : 0);

    if (path->buffer && ((path->size)>=(size+emdzero))) {

	memset(path->buffer, 0, path->size);
	goto success;

    } else if ((path->flags & FS_PATH_FLAG_BUFFER_ALLOC) || (path->buffer==NULL)) {

	if (FS_path_allocate(path, size+endzero)==1) goto success;

    }

    return 0;

    success:

    memcpy(path->buffer, buffer, size);
    path->start.str=path->buffer;
    path->start.length=size;

    return size;

}

unsigned int FS_path_get_maximum_length()
{

#ifdef PATH_MAX

    return PATH_MAX;

#else

    return 4096;

#endif
}

struct dstr_s *FS_path_get_dstr(struct fs_path_s *path)
{
    return FS_path_valid(path) ? &path->start : NULL;
}

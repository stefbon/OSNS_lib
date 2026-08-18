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

#include <math.h>

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"

#include "path.h"
#include "utils.h"

/* static unsigned int get_fieldsize_uint(unsigned int value)
{
    return (unsigned int)(log10((double) value) + 1);
}*/

unsigned int FS_path_get_append_required_size(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep)
{
    struct dstr_s data=DSTR_INIT;
    return (FS_path_convert_type_ptr(type, ptr, &data)>0) ? (path->start.length + 1 + data.length +((addpathsep) ? 1 : 0)) : 0;
}

static void fs_path_append_hlpr(struct fs_path_s *path, char *start, char *bytes, unsigned int size, unsigned char addpathsep)
{

    if (addpathsep) {

        start[0]='/';
        start++;
        path->start.length++;

    }

    memcpy(start, bytes, size);
    path->start.length += size;

}

static unsigned int fs_path_append_write_dynamic(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{
    char *start=NULL;
    int left=0;
    unsigned int bytes2append=size + ((addpathsep) ? 1 : 0);
    char *ptr2compare=path->buffer;

    if (path->buffer) {

        if (path->start.str==NULL) {

            path->start.str=path->buffer;
            path->start.length=0;

        }

        start=(char *)(path->start.str + path->start.length);
        left=(int)(path->buffer + path->size - start);

    }

    if (left < bytes2append) {
        unsigned int bytes2grow = (* path->calc_chunk_size)(path, (unsigned int)(bytes2append - left));

        path->buffer=realloc(path->buffer, path->size + bytes2grow);

        if (path->buffer==NULL) {

            FS_path_clear(path);
            return 0;

        } else if (ptr2compare) {

            if (path->buffer != ptr2compare) {
                unsigned int length=(unsigned int)(path->start.str - ptr2compare);

                /* correct the start str when moved */

                path->start.str=(char *)(path->buffer + length);
                start=(char *)(path->start.str + path->start.length);

            }

        } else if (path->start.str==NULL) {

            path->start.str=path->buffer;
            path->start.length=0;
            start=path->start.str;

        }

        path->size += bytes2grow;

    }

    fs_path_append_hlpr(path, start, bytes, size, addpathsep);
    return bytes2append;

}

static unsigned int fs_path_append_write_static(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{
    char *start=NULL;
    int left=0;
    unsigned int bytes2append=0;

    if (path->start.str==NULL) {

        path->start.str=path->buffer;
        path->start.length=0;

    }

    start=(char *)(path->start.str + path->start.length);
    left=(int)(path->buffer + path->size - start);
    bytes2append=size + ((addpathsep) ? 1 : 0);
    if (left < bytes2append) return 0;

    fs_path_append_hlpr(path, start, bytes, size, addpathsep);
    return bytes2append;
}

unsigned int FS_path_append_nowrite(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{
    return (size + ((addpathsep) ? 1 : 0));
}

void FS_path_append_init(struct fs_path_s *path, unsigned int flags)
{

    path->flags |= flags;
    path->flags &= ~FS_PATH_FLAG_PREPEND;

    if (path->flags & FS_PATH_FLAG_BUFFER_ALLOC) {

        path->append=fs_path_append_write_dynamic;

    } else {

        path->append=(path->buffer) ? fs_path_append_write_static : FS_path_append_nowrite;

    }

}

unsigned int FS_path_append(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep)
{
    unsigned int result=0;
    struct dstr_s data=DSTR_INIT;

    if ((FS_path_convert_type_ptr(type, ptr, &data)>0) && (data.length)) result=(* path->append)(path, data.str, data.length, addpathsep);
    return result;
}

static void fs_path_prepend_hlpr(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{

    if (addpathsep) {

        path->start.str--;
        path->start.str[0]='/';
        path->start.length++;

    }

    path->start.str -= size;
    memcpy(path->start.str, bytes, size);
    path->start.length += size;

}

static void fs_path_prepend_set_at_end(struct fs_path_s *path)
{
    path->start.str=(char *)(path->buffer + path->size - 1);
    path->start.str[0]=0; /* have a zero terminator */
    path->start.length=1; /* length of the data, not the string, which is zero */
}

static unsigned int fs_path_prepend_write_dynamic(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{
    unsigned int bytes2prepend=size + ((addpathsep) ? 1 : 0);
    char *start=NULL;
    int left=0;
    char *ptr2compare=path->buffer;

    if (path->buffer) {

        /* move the start of the path to the end if not already */

        if (path->start.str==NULL) fs_path_prepend_set_at_end(path);
        start=(char *)(path->start.str - bytes2prepend);
        left=(int)(start - path->buffer); /* may be less than zero */

    }

    if (left < 0) {
        unsigned int bytes2grow = (* path->calc_chunk_size)(path, (unsigned int) abs(left));

        path->buffer=realloc(path->buffer, path->size + bytes2grow);

        if (path->buffer==NULL) {

            FS_path_clear(path);
            return 0;

        } else if (ptr2compare) {

            if (path->buffer != ptr2compare) {
                unsigned int length=(unsigned int)(path->start.str - ptr2compare);

                /* correct the start str when moved */

                path->start.str=(char *)(path->buffer + length);
                start=(char *)(path->start.str - bytes2prepend);

            }

        } else if (path->start.str==NULL) {

            fs_path_prepend_set_at_end(path);

        }

        /* move extrabytes to make enough space to fit in bytes */

        path->size += bytes2grow;
        memmove((char *)(path->start.str + bytes2grow), path->start.str, path->start.length);
        path->start.str += bytes2grow;

    }

    fs_path_prepend_hlpr(path, bytes, size, addpathsep);
    return bytes2prepend;

}

static unsigned int FS_path_prepend_write_static(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep)
{
    unsigned int bytes2prepend=size + ((addpathsep) ? 1 : 0);
    char *start=(char *)(path->start.str - bytes2prepend);
    int left=(int)(start - path->buffer); /* may be less than zero */

    if (left < 0) return 0;
    fs_path_prepend_hlpr(path, bytes, size, addpathsep);
    return bytes2prepend;
}

void FS_path_prepend_init(struct fs_path_s *path, unsigned int flags)
{

    /* start at the end to append backwards */

    path->flags |= (flags | FS_PATH_FLAG_PREPEND);

    if (path->flags & FS_PATH_FLAG_BUFFER_ALLOC) {

        path->prepend= fs_path_prepend_write_dynamic;

    } else {

        path->prepend=(path->buffer) ? FS_path_prepend_write_static : FS_path_append_nowrite;

    }

}

unsigned int FS_path_prepend(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep)
{
    unsigned int result=0;
    struct dstr_s data=DSTR_INIT;

    if ((FS_path_convert_type_ptr(type, ptr, &data)>0) && (data.length)) result=(* path->prepend)(path, data.str, data.length, addpathsep);
    return result;

}

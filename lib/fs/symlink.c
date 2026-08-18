/*

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

#include <fcntl.h>
#include <sys/stat.h>

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-io.h"
#include "libosns-path.h"

#include "fs.h"
#include "symlink.h"
#include "realpath.h"

#ifdef __linux__

int FS_readlink(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *buffer, unsigned int flags)
{
    int result=0;
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];

    memcpy(name, data.str, length);
    name[length]='\0';

    {
        int fd=FS_object_get_unix_fd(fso);
        int bytesread=0;
        unsigned char dorealloc=0;
        unsigned int size=buffer->length ? buffer->length : 128;

        if ((buffer->str==NULL) || (buffer->flags & DSTR_FLAG_ALLOC_DATA)) dorealloc=1;

        /* get the target of the symbolic link ...
            note that there is no need to add an extra zero byte ...
            the buffer to hold the target is filled with zero's and
            FS path works like a string, it gives a start and a length by default ... 
            so a -> terminating <- zero is not required  */

        doreadlinkat:

        if (dorealloc) {

            if (buffer->length<size) {

                if (DSTR_alloc_str_raw(buffer, size, 1)==0) return 0;
                logoutput_debug("%s: created buffer with size %u to read the link at %s", __FUNCTION__, buffer->length, name);

            }

        }

        bytesread=readlinkat(fd, name, buffer->str, buffer->length);

        if (bytesread==-1) {

            logoutput_debug("%s: error %u readlink on fd %i path %s (%s)", __FUNCTION__, errno, fd, name, strerror(errno));

        } else if (bytesread>=buffer->length) {

            if (flags & FS_READLINK_FLAG_ALLOCATE) {

                if (dorealloc) {

                    size+=128;
                    goto doreadlinkat;

                }

            } else {

                result=(uint64_t) bytesread;

            }

        } else {

            buffer->str[(unsigned int) bytesread]='\0';
            buffer->length=(unsigned int) bytesread;

            result=(int) bytesread;
            logoutput_debug("%s: found target %s", __FUNCTION__, buffer->str);

            if ((flags & FS_READLINK_FLAG_REALPATH) && (flags & FS_READLINK_FLAG_ALLOCATE)) {

                result=FS_realpath(fso, 'c', name, buffer);

            }

        }

    }

    return result;

}

int FS_symlink(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *target)
{
    int result=-1;
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];
    int tmp=-1;

    memcpy(name, data.str, length);
    name[length]='\0';

    logoutput_debug("%s", __FUNCTION__);

    if (DSTR_is_empty(target)) {

        logoutput_debug("%s: target symlink not defined ... cannot continue", __FUNCTION__);
        return -1;

    }

    {
        int fd=FS_object_get_unix_fd(fso);
        unsigned int size=DSTR_get_length(target);
        char buffer[size + 1];

        if (size==0) return -1;

        memset(buffer, 0, size+1);
        memcpy(buffer, target->str, size);

        tmp=symlinkat(buffer, fd, name);

    }

    if (tmp==-1) {

        logoutput_debug("%s: error %u on path %s (%s)", __FUNCTION__, errno, name, strerror(errno));

    } else {

        result=(int) tmp;
        logoutput_debug("%s: success path %s", __FUNCTION__, name);

    }

    return result;
}

#else

int FS_readlink(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *buffer, unsigned int flags)
{
    return -1;
}

int FS_symlink(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *target)
{
    return -1;
}

#endif

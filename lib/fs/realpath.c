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

#include "fs.h"

#ifdef __linux__

static unsigned int fs_realpath_hlpr(char *path, struct dstr_s *buffer)
{
    char *resolved=NULL;
    unsigned int result=0;

    resolved=realpath(path, NULL);

    if (resolved) {
        unsigned int length=strlen(resolved);

        DSTR_set_bytes_raw(buffer, resolved, length, 0);
        result=length;

    } else {

        logoutput_debug("%s: unable to get resolved path from %.*s", __FUNCTION__, buffer->length, buffer->str);

    }

    return result;

}

static int fs_read_path_from_fd(int fd, char *buffer, unsigned int size)
{
    int result=-1;

    memset(buffer, 0, size);

    if (fd==AT_FDCWD) {

        if (getcwd(buffer, size)==NULL) {

            result=(errno==ERANGE) ? 0 : -1;

        } else {

            result=strlen(buffer);

        }

    } else if (fd>=0) {
        char procpath[64];
        int tmp=snprintf(procpath, 64, "/proc/self/fd/%i", fd);

        tmp=readlink(procpath, buffer, size);

        if (tmp==-1) {

            result=-1;

        } else if (tmp>=size) {

            result=0;

        } else {

            buffer[(unsigned int) tmp]='\0';
            result=tmp;

        }

    }

    return result;

}

unsigned int FS_realpath(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *path)
{
    unsigned int result=0;
    int fd=-1;
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];

    if (length==0) {

        logoutput_debug("%s: unable to get realpath ... name not defined", __FUNCTION__);
        return 0;

    }

    memcpy(name, data.str, length);
    name[length]='\0';

    /* absolute path ... */

    if (FS_path_is_path_seperator(name[0])) return fs_realpath_hlpr(name, path);

    /* relative to fs object */

    fd=FS_object_get_unix_fd(fso);

    if ((fd==AT_FDCWD) || (fd>=0)) {
        unsigned int size=256 + length;

        /* construct path using the current workking directory */

        while (result==0) {
            char buffer[size];
            int tmp=0;

            if (size>PATH_MAX) return 0;

            tmp=fs_read_path_from_fd(fd, buffer, size);

            if (tmp==-1) {

                result=-1;

            } else if (tmp==0) {

                size+=256;
                continue;

            } else if (tmp>0) {

                if ((tmp + length + 2) >= size) {

                    size+=256;
                    continue;

                }

                buffer[tmp]='/';
                memcpy(&buffer[tmp + 1], name, length);
                return fs_realpath_hlpr(buffer, path);

            }

        }

    } 

    return result;

}

#else

unsigned int FS_realpath(struct fs_object_s *fso, const unsigned char type, void *ptr, struct dstr_s *path)
{
    return 0;
}

#endif

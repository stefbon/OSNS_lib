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
#include "libosns-datatypes.h"
#include "libosns-io.h"

#include "fs.h"

static unsigned int FS_parse_open_how(const char *how)
{
    struct dstr_s tmp=DSTR_INIT;
    struct dstr_s part=DSTR_INIT;
    unsigned int flags=0;

    DSTR_set_bytes_raw(&tmp, how, 0, 0);

    while (DSTR_get_first_dstr(&tmp, ',', &part, 1, 1)) {

	if (DSTR_cmp_bytes(&part, "directory", 0, 1, 0, 1)) {

	    flags |= O_DIRECTORY;

	} else if (DSTR_cmp_bytes(&part, "rdonly", 0, 1, 0, 1) || DSTR_cmp_bytes(&part, "readdonly", 0, 1, 0, 1)) {

	    flags |= O_RDONLY;

	} else if (DSTR_cmp_bytes(&part, "rdwr", 0, 1, 0, 1)) {

	    flags |= O_RDWR;

	} else if (DSTR_cmp_bytes(&part, "wronly", 0, 1, 0, 1)) {

	    flags |= O_WRONLY;

	} else if (DSTR_cmp_bytes(&part, "append", 0, 1, 0, 1)) {

	    flags |= O_APPEND;

	} else if (DSTR_cmp_bytes(&part, "create", 0, 1, 0, 1)) {

	    flags |= O_CREAT;

	} else if (DSTR_cmp_bytes(&part, "exclusive", 0, 1, 0, 1)) {

	    flags |= O_EXCL;

	} else if (DSTR_cmp_bytes(&part, "path", 0, 1, 0, 1)) {

	    flags |= O_PATH;

	} else if (DSTR_cmp_bytes(&part, "tmpfile", 0, 1, 0, 1)) {

	    flags |= O_TMPFILE;

	} else if (DSTR_cmp_bytes(&part, "trunc", 0, 1, 0, 1)) {

	    flags |= O_TRUNC;

	} else {

	    logoutput_debug("%s: flagstr %.*s not reckognized", __FUNCTION__, part.length, part.str);

	}

    }

    return flags;

}

unsigned char FS_open(struct fs_object_s *fsor, const unsigned char type, void *ptr, struct fs_object_s *fso, struct fs_init_s *init, const char *how)
{
    unsigned char result=0;
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];
    unsigned int flags=(how ? FS_parse_open_how(how) : 0);

    logoutput_debug("%s", __FUNCTION__);

    if (FS_object_valid(fso)==0) {

        logoutput_debug("%s: fs object not valid ... cannot continue", __FUNCTION__);
        return 0;

    }

    memcpy(name, data.str, length);
    name[length]='\0';

#ifdef __linux__

    {
        unsigned int length=strlen(name);

        if (length==0) {

            logoutput_debug("%s: unable to open ... empty path ... error %u (%s)", __FUNCTION__, EINVAL, strerror(EINVAL));

        } else {
            int fdr=(fsor ? IO_object_backend_get_unix_fd(&fsor->backend) : -1);
            int fd=0;

	    flags |= (O_NONBLOCK | O_CLOEXEC); /* always nonblocking and close on exec */

            fd=openat(fdr, name, flags, (init ? init->mode : 0755));

            if (fd>=0) {

                logoutput_debug("%s: open %s with fd %u and flags %u", __FUNCTION__, name, fd, fso->openflags);
                if (fso->backend.type==0) IO_object_backend_init(&fso->backend, IO_OBJECT_BACKEND_TYPE_FD);
                IO_object_backend_set_unix_fd(&fso->backend, fd);
                result=1;

            } else {

                logoutput_debug("%s: unable to open %s with flags %u ... error %u (%s)", __FUNCTION__, name, fso->openflags, errno, strerror(errno));

            }

        }

    }

#endif

    return result;

}

unsigned char FS_close(struct fs_object_s *fso)
{
    return (FS_object_valid(fso) ? IO_object_backend_close(&fso->backend) : 0);
}

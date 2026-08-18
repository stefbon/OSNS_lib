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

void FS_object_init(struct fs_object_s *fso)
{

    if (fso==NULL) {

        logoutput_debug("%s: error ... fs object not defined", __FUNCTION__);
        return;

    }

    memset(fso, 0, sizeof(struct fs_object_s));

#ifdef __linux__

    IO_object_backend_init(&fso->backend, IO_OBJECT_BACKEND_TYPE_FD);

#else

    IO_object_backend_init(&fso->backend, 0);

#endif

    IO_buffer_init(&fso->buffer);

}

unsigned char FS_object_valid(struct fs_object_s *fso)
{
    return (fso ? 1 : 0);
}

void FS_object_clear(struct fs_object_s *fso)
{

    if (FS_object_valid(fso)) {

        IO_buffer_free(&fso->buffer);
        memset(fso, 0, sizeof(struct fs_object_s));

    }

}

unsigned char FS_object_close(struct fs_object_s *fso)
{
    return (FS_object_valid(fso)) ? IO_object_backend_close(&fso->backend) : 0;
}

unsigned char FS_object_is_open(struct fs_object_s *fso)
{
    return (FS_object_valid(fso)) ? IO_object_backend_is_open(&fso->backend) : 0;
}

#ifdef __linux__

int FS_object_get_unix_fd(struct fs_object_s *fso)
{
    int fd=-1;

    if (FS_object_valid(fso)) {

        fd=IO_object_backend_get_unix_fd(&fso->backend);

    } else {

        logoutput_debug("%s: unable to get fd ... object not valid", __FUNCTION__);

    }

    return fd;
}

void FS_object_set_unix_fd(struct fs_object_s *fso, int fd)
{

    if (FS_object_valid(fso)) {

        IO_object_backend_set_unix_fd(&fso->backend, fd);

    } else {

        logoutput_debug("%s: unable to set fd ... object not valid", __FUNCTION__);

    }

}

#else

int FS_object_get_unix_fd(struct fs_object_s *fso)
{
    return -1;
}

void FS_object_set_unix_fd(struct fs_object_s *fso, int fd)
{}

#endif

static struct fs_object_s fso_cdw = {
    .backend.type                                                       = IO_OBJECT_BACKEND_TYPE_FD,
    .backend.close                                                      = NULL,
    .backend.handle.fd                                                  = AT_FDCWD,
    .offset                                                             = 0,
    .openflags                                                          = 0,
    .readflags                                                          = 0,
    .writeflags                                                         = 0,
    .buffer.flags                                                       = 0,
    .buffer.bytesread                                                   = 0,
    .buffer.pos                                                         = 0,
    .buffer.size                                                        = 0,
    .buffer.ptr                                                         = NULL,
};

struct fs_object_s *FS_object_cwd()
{
    return &fso_cdw;
}

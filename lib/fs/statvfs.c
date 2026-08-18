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

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-io.h"
#include "libosns-path.h"

#include "fs.h"
#include "statvfs.h"

#ifdef __linux__

unsigned int FS_fgetstatvfs(struct fs_object_s *fso, char *name, uint64_t mask, struct fs_statvfs_s *fstvfs, struct osns_status_s *os)
{
    int fd=FS_object_get_unix_fd(fso);
    int result=-1;

    if (fd==-1) {

        result=statvfs(name, &fstvfs->stvfs);

    } else if ((name==NULL) || (strlen(name)==0)) {

        result=fstatvfs(fd, &fstvfs->stvfs);

    } else {
        int fdtmp=openat(fd, name, O_RDONLY, 0);

        if (fdtmp>=0) {

            result=fstatvfs(fdtmp, &fstvfs->stvfs);
            close(fdtmp);

        }

    }

    return ((result==0) ? FS_STATVFS_ALL : 0);
}

unsigned long FS_statvfs_get_blocksize(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_bsize;
}

unsigned long FS_statvfs_get_fragmentsize(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_frsize;
}

unsigned long FS_statvfs_get_blocks(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_blocks;
}

unsigned long FS_statvfs_get_freeblocks(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_bfree;
}

unsigned long FS_statvfs_get_availblocks(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_bavail;
}

unsigned long FS_statvfs_get_files(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_files;
}

unsigned long FS_statvfs_get_freefiles(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_ffree;
}

unsigned long FS_statvfs_get_availfiles(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_favail;
}

unsigned long FS_statvfs_get_fsid(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_fsid;
}

unsigned long FS_statvfs_get_mountflags(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_flag;
}

unsigned long FS_statvfs_get_namemax(struct fs_statvfs_s *fstvfs)
{
    return fstvfs->stvfs.f_namemax;
}

#else

unsigned int FS_fgetstatvfs(struct fs_object_s *fso, char *name, uint64_t mask, struct fs_statvfs_s *fstvfs, struct osns_status_s *os)
{
    return 0;
}

#endif
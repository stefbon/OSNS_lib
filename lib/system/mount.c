/*
  2010, 2011, 2012, 2103, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Stef Bon <stefbon@gmail.com>

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
#include "libosns-fs.h"
#include "libosns-error.h"

#ifdef __linux__

#include <fcntl.h>
#include <sys/mount.h>

static int system_mount_worker(const char *source, struct fs_path_s *path, const char *fstype, unsigned long mountflags, void *mountoptions, unsigned char dounmount)
{
    unsigned int size=FS_path_export(path, NULL, 1);
    char buffer[size];
    int result=-1;

    size=FS_path_export(path, buffer, 1);
    result=(dounmount) ? umount2(buffer, MNT_DETACH) : mount(source, buffer, fstype, mountflags, mountoptions);
    return result;
}

#else

static int system_mount_worker(const char *source, struct fs_path_s *path, const char *fstype, unsigned long mountflags, void *mountoptions, unsigned char unmount)
{
    return -1;
}

#endif

int SYSTEM_mount(const char *source, struct fs_path_s *path, const char *fstype, unsigned long mountflags, void *mountoptions)
{
    return system_mount_worker(source, path, fstype, mountflags, mountoptions, 0);
}

int SYSTEM_umount(struct fs_path_s *path)
{
    return system_mount_worker(NULL, path, NULL, 0, NULL, 1);
}

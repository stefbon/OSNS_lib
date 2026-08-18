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

#ifndef LIB_FS_STATVFS_H
#define LIB_FS_STATVFS_H

#include <fcntl.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#define FS_STATVFS_BLOCKSIZE                            1
#define FS_STATVFS_FRAGMENTSIZE                         2

#define FS_STATVFS_BLOCKS                               4
#define FS_STATVFS_FREEBLOCKS                           8
#define FS_STATVFS_AVAILBLOCKS                          16

#define FS_STATVFS_FILES                                32
#define FS_STATVFS_FREEFILES                            64
#define FS_STATVFS_AVAILFILES                           128

#define FS_STATVFS_FSID                                 256
#define FS_STATVFS_FLAG                                 512
#define FS_STATVFS_NAMEMAX                              1024

#define FS_STATVFS_ALL ( FS_STATVFS_BLOCKSIZE | FS_STATVFS_FRAGMENTSIZE | FS_STATVFS_BLOCKS | FS_STATVFS_FREEBLOCKS | FS_STATVFS_AVAILBLOCKS | FS_STATVFS_FILES | FS_STATVFS_FREEFILES | FS_STATVFS_AVAILFILES | FS_STATVFS_FSID | FS_STATVFS_FLAG | FS_STATVFS_NAMEMAX )

#ifdef __linux__

#include <sys/stat.h>

struct fs_statvfs_s {
    struct statvfs 	stvfs;
};

#else

struct fs_statvfs_s {
    unsigned long                                       dummylong;
};

#endif /* __linux__ */

/* Prototypes */

/* system calls */

/* get */

unsigned long FS_statvfs_get_blocksize(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_fragmentsize(struct fs_statvfs_s *fstvfs);

unsigned long FS_statvfs_get_blocks(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_freeblocks(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_availblocks(struct fs_statvfs_s *fstvfs);

unsigned long FS_statvfs_get_files(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_freefiles(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_availfiles(struct fs_statvfs_s *fstvfs);

unsigned long FS_statvfs_get_fsid(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_flag(struct fs_statvfs_s *fstvfs);
unsigned long FS_statvfs_get_namemax(struct fs_statvfs_s *fstvfs);

#endif

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

#ifndef LIB_FS_STAT_STAT_H
#define LIB_FS_STAT_STAT_H

#include <fcntl.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>

#include "libosns-time.h"

#define SYSTEM_FILE_PROPERTY_MIMETYPE		1

#define FS_STAT_FLAG_FOLLOW_SYMLINK		1

#define FS_STAT_INDEX_TYPE			0
#define FS_STAT_INDEX_MODE			1
#define FS_STAT_INDEX_NLINK			2
#define FS_STAT_INDEX_UID			3
#define FS_STAT_INDEX_GID			4
#define FS_STAT_INDEX_ATIME			5
#define FS_STAT_INDEX_MTIME			6
#define FS_STAT_INDEX_CTIME			7
#define FS_STAT_INDEX_INO			8
#define FS_STAT_INDEX_SIZE			9
#define FS_STAT_INDEX_BLOCKS		        10
#define FS_STAT_INDEX_BTIME			11
#define FS_STAT_INDEX_MNTID			12

struct fs_stat_dev_s {
    uint32_t				        major;
    uint32_t				        minor;
};

#define FS_STAT_DEV_INIT			{0, 0}

#ifdef __linux__

#include <sys/stat.h>

#ifdef STATX_TYPE

#define FS_STAT_TYPE			        STATX_TYPE
#define FS_STAT_MODE			        STATX_MODE
#define FS_STAT_NLINK			        STATX_NLINK
#define FS_STAT_UID		                STATX_UID
#define FS_STAT_GID			        STATX_GID
#define FS_STAT_ATIME			        STATX_ATIME
#define FS_STAT_MTIME			        STATX_MTIME
#define FS_STAT_CTIME			        STATX_CTIME
#define FS_STAT_INO			        STATX_INO
#define FS_STAT_SIZE			        STATX_SIZE
#define FS_STAT_BLOCKS			        STATX_BLOCKS

#define FS_STAT_BTIME			        STATX_BTIME
#define FS_STAT_MNTID			        STATX_MNT_ID

#define FS_STAT_BASIC_STATS			(FS_STAT_TYPE | FS_STAT_MODE | FS_STAT_NLINK | FS_STAT_UID | FS_STAT_GID | FS_STAT_ATIME | FS_STAT_MTIME | FS_STAT_CTIME | FS_STAT_INO | FS_STAT_SIZE | FS_STAT_BLOCKS)
#define FS_STAT_ALL				FS_STAT_BASIC_STATS

struct fs_stat_s {
	unsigned char	flags;
	unsigned int	mask;
	struct statx 	stx;
};

#define sst_mode				stx.stx_mode
#define sst_nlink				stx.stx_nlink
#define sst_uid					stx.stx_uid
#define sst_gid					stx.stx_gid
#define sst_atime				stx.stx_atime
#define sst_mtime				stx.stx_mtime
#define sst_btime				stx.stx_btime
#define sst_ctime				stx.stx_ctime
#define sst_ino					stx.stx_ino
#define sst_size				stx.stx_size
#define sst_blocks				stx.stx_blocks
#define sst_blksize				stx.stx_blksize

#else

#define FS_STAT_TYPE			        (1 << 0)
#define FS_STAT_MODE			        (1 << 1)
#define FS_STAT_NLINK			        (1 << 2)
#define FS_STAT_UID				(1 << 3)
#define FS_STAT_GID				(1 << 4)
#define FS_STAT_ATIME			        (1 << 5)
#define FS_STAT_MTIME			        (1 << 6)
#define FS_STAT_CTIME			        (1 << 7)
#define FS_STAT_INO				(1 << 8)
#define FS_STAT_SIZE			        (1 << 9)
#define FS_STAT_BLOCKS			        (1 << 10)

#define FS_STAT_BTIME			        (1 << 11)
#define FS_STAT_MNTID			        (1 << 12)

#define FS_STAT_BASIC_STATS			(FS_STAT_TYPE | FS_STAT_MODE | FS_STAT_NLINK | FS_STAT_UID | FS_STAT_GID | FS_STAT_ATIME | FS_STAT_MTIME | FS_STAT_CTIME | FS_STAT_INO | FS_STAT_SIZE | FS_STAT_BLOCKS)
#define FS_STAT_ALL				FS_STAT_BASIC_STATS

struct fs_stat_s {
	unsigned char	flags;
	unsigned int	mask;
	struct stat	st;
	struct timespec	dummy;
};

#define sst_mode				st.st_mode
#define sst_nlink				st.st_nlink
#define sst_uid					st.st_uid
#define sst_gid					st.st_gid
#define sst_atime				st.st_atim
#define sst_mtime				st.st_mtim
#define sst_btime				dummy;
#define sst_ctime				st.st_ctim
#define sst_ino					st.st_ino
#define sst_size				st.st_size
#define sst_blocks				st.st_blocks
#define sst_blksize				st.st_blksize

#endif

#endif /* __linux__ */

/* all different times */

#define FS_STAT_TIME_ACCESS                     1
#define FS_STAT_TIME_MODIFY                     2
#define FS_STAT_TIME_CHANGE                     3
#define FS_STAT_TIME_CREATION                   4

#define FS_STAT_TIME_BIRTH                      FS_STAT_TIME_CREATION

/* Prototypes */

/* system calls */

/* get */

uint64_t FS_fgetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst);

/* set */

uint64_t FS_fsetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst);

#endif

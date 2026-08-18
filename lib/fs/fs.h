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

#ifndef LIB_FS_FS_H
#define LIB_FS_FS_H

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-misc.h"
#include "libosns-error.h"
#include "libosns-datatypes.h"
#include "libosns-io.h"
#include "libosns-path.h"


struct fs_path_s;

#define FS_RW_FLAG_DSYNC                                                RWF_DSYNC
#define FS_RW_FLAG_HIPRI                                                RWF_HIPRI
#define FS_RW_FLAG_SYNC                                                 RWF_SYNC
#define FS_RW_FLAG_NOWAIT                                               RWF_NOWAIT
#define FS_RW_FLAG_APPEND                                               RWF_APPEND

#define FS_FSYNC_LEVEL_DATA                                             FS_SYNC_LEVEL_DATA
#define FS_FSYNC_LEVEL_FULL                                             FS_SYNC_LEVEL_FULL

#define FS_RMDIR_FLAG_RECURSIVE                                         1

#define FS_DENTRY_TYPE_NOTSET                                           0
#define FS_DENTRY_TYPE_FILE				                1
#define FS_DENTRY_TYPE_DIRECTORY		                        2
#define FS_DENTRY_TYPE_UNKNOWN                                          3

#define FS_DENTRY_FLAG_LAST				                1

struct fs_dentry_s {
    uint16_t						                flags; /* last? */
    uint16_t						                type;
    uint64_t						                ino; /* ino on the server; some filesystems rely on this */
    off64_t                                                             offset;
    struct dstr_s                                                       name;
};

#define FS_DENTRY_INIT					                {0, 0, 0, 0, DSTR_INIT}

struct fs_init_s {
#ifdef __linux__
    mode_t						                mode;
    dev_t                                                               dev;
#endif
};

struct fs_data_s {
    char                                                                *data;
    size_t                                                              size;
    off64_t                                                             offset;
};

#ifdef __linux__

#define FS_SEEK_SET                                                     SEEK_SET
#define FS_SEEK_CUR                                                     SEEK_CUR
#define FS_SEEK_END                                                     SEEK_END

#ifdef SEEK_DATA

#define FS_SEEK_DATA                                                    SEEK_DATA
#define FS_SEEK_HOLE                                                    SEEK_HOLE

#endif /* SEEK_DATA */

#else

#define FS_SEEK_SET                                                     1
#define FS_SEEK_CUR                                                     2
#define FS_SEEK_END                                                     3

#define FS_SEEK_DATA                                                    4
#define FS_SEEK_HOLE                                                    5

#endif

#define FS_RW_FLAG_NEXT_DENTRY                                          1

#define FS_STAT_FLAG_READLINK_ALLOCATE                                  1
#define FS_STAT_FLAG_READLINK_REALPATH                                  2

#define FS_DIRECTORY_DEFAULT_BUFFER_SIZE                                4096

struct fs_object_s {
    struct io_object_backend_s                                          backend; /* reference to the kernel object like fd */
    off64_t                                                             offset;
    unsigned int                                                        openflags;
    unsigned int                                                        readflags;
    unsigned int                                                        writeflags;
    struct io_buffer_s                                                  buffer;
};

/* Prototypes */

void FS_object_init(struct fs_object_s *fso);
unsigned char FS_object_valid(struct fs_object_s *fso);
void FS_object_clear(struct fs_object_s *fso);

unsigned char FS_object_close(struct fs_object_s *fso);
unsigned char FS_object_is_open(struct fs_object_s *fso);

int FS_object_get_unix_fd(struct fs_object_s *fso);
void FS_object_set_unix_fd(struct fs_object_s *fso, int fd);

struct fs_object_s *FS_object_cwd();

#endif

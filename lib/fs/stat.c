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

#include "fs.h"
#include "stat.h"
#include "stat-utils.h"
#include "path/utils.h"

#ifdef __linux__

uint64_t FS_fgetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst)
{
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];

    if (mask==0) mask=FS_STAT_ALL;
    fst->mask=0;

    if (data.str) memcpy(name, data.str, length);
    name[length]='\0';

    {
        int fd=FS_object_get_unix_fd(fso); /* maybe not valid (==-1) */

#ifdef STATX_TYPE
        struct statx *stx=&fst->stx;

        /* use the flag EMPTY_PATH, which allows the path/buffer to be empty ... note this says it may be empty */

        if (statx(fd, name, (AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW), mask, stx)==0) {

            fst->mask=stx->stx_mask;

        } else {

	    logoutput_debug("%s: error %u on fd %i (%s)", __FUNCTION__, errno, fd, strerror(errno));

        }

#else
        struct stat *st=&fst->st;

        /* use the flag EMPTY_PATH, which allows the path/buffer to be empty ... note this says it may be empty */

        if (fstatat(fd, name, st, (AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW))==0) {

            fst->mask = (mask & SYSTEM_STAT_BASIC_STATS); /* with "old" stat/fstatat every property/value is read */

        } else {

	    logoutput_warning("%s: error %u on fd %i (%s)", __FUNCTION__, errno, fd, strerror(errno));

        }

#endif /* STATX_TYPE */

    }

    return fst->mask;

}

/* hlpr function cause the AT version of truncate does not exist */

static int ftruncatat(int fdr, const char *name, off_t size)
{
    int result=-1;

    if ((name==NULL) || (strlen(name)==0)) {

        result=ftruncate(fdr, size);

    } else {
        int fd=openat(fdr, name, O_WRONLY, 0);

        if (fd>=0) {

            result=ftruncate(fd, size);
            close(fd);

        }

    }

    return result;
}

uint64_t FS_fsetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst)
{
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];
    uint64_t maskdone=0;
    int fd=FS_object_get_unix_fd(fso);

    memcpy(name, data.str, length);
    name[length]='\0';

    if (mask & FS_STAT_SIZE) {
        off_t size=FS_stat_get_size(fst);
        int result=-1;

        result=ftruncatat(fd, name, size);

	if (result==0) {

	    logoutput_debug("%s: set size to %lu", __FUNCTION__, size);
	    maskdone |= FS_STAT_SIZE;
	    mask &= ~FS_STAT_SIZE;

	} else {

	    logoutput_warning("%s: error %i set size to %lu (%s)", __FUNCTION__, errno, size, strerror(errno));

	}

    }

    if (mask & (FS_STAT_UID | FS_STAT_GID)) {
	uid_t uid=(mask & FS_STAT_UID) ? FS_stat_get_unique_uid(fst) : (uid_t) -1;
	gid_t gid=(mask & FS_STAT_GID) ? FS_stat_get_unique_gid(fst) : (gid_t) -1;

	/* set the uid and gid
	        NOTE: using (uid_t) -1 and (gid_t) -1 in fchown will ignore this value */

	if (fchownat(fd, name, uid, gid, (AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW))==0) {

	    logoutput_debug("%s: uid and gid set to %i:%i", __FUNCTION__, uid, gid);
	    maskdone |= ((mask & FS_STAT_UID) ? FS_STAT_UID : 0) | ((mask & FS_STAT_GID) ? FS_STAT_GID : 0);
	    mask &= ~(FS_STAT_UID | FS_STAT_GID);

        } else {

	    logoutput_warning("%s: error %u set user and/or group (%s)", __FUNCTION__, errno, strerror(errno));

	}

    }

    if (mask & FS_STAT_MODE) {
	mode_t mode=FS_stat_get_mode(fst);

	if (fchmodat(fd, name, mode, (AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW))==0) {

	    logoutput_debug("%s: mode set to %u", __FUNCTION__, mode);
	    maskdone |= FS_STAT_MODE;
	    mask &= ~FS_STAT_MODE;

        } else {

	    logoutput_warning("%s: error %i set mode to %i (%s)", __FUNCTION__, errno, mode, strerror(errno));

	}

    }

    if (mask & (FS_STAT_ATIME | FS_STAT_MTIME)) {
	struct timespec times[2];
	unsigned int todo=0;

	if (mask & FS_STAT_ATIME) {
	    struct timespec_s atime;

            if (FS_stat_get_time(fst, FS_STAT_TIME_ACCESS, &atime)) {

	        times[0].tv_sec=TIME_get_sec(&atime);
	        times[0].tv_nsec=TIME_get_nsec(&atime);
	        todo|=FS_STAT_ATIME;

            }

	} else {

	    times[0].tv_sec=0;
	    times[0].tv_nsec=UTIME_OMIT;

	}

	if (mask & FS_STAT_MTIME) {
	    struct timespec_s mtime;

            if (FS_stat_get_time(fst, FS_STAT_TIME_MODIFY, &mtime)) {

	        times[1].tv_sec=TIME_get_sec(&mtime);
	        times[1].tv_nsec=TIME_get_sec(&mtime);
	        todo|=FS_STAT_MTIME;

            }

	} else {

	    times[1].tv_sec=0;
	    times[1].tv_nsec=UTIME_OMIT;

	}

	if (utimensat(fd, name, times, (AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW))==0) {

	    maskdone |= todo;
	    mask &= ~todo;

        } else {

	    logoutput_warning("%s: error %i set atime and/or mtime (%s)", __FUNCTION__, errno, strerror(errno));

	}

    }

    if (mask & (FS_STAT_CTIME | FS_STAT_BTIME)) {

	logoutput_warning("%s: change of ctime/btime not supported/possible", __FUNCTION__);
	mask &= ~(FS_STAT_CTIME | FS_STAT_BTIME);

    }


    if (mask>0) {

	logoutput_warning("%s: still actions (%i) requested but ignored (done %i)", __FUNCTION__, mask, maskdone);

    } else {

	logoutput_debug("%s: all actions done %i", __FUNCTION__, maskdone);

    }

    return maskdone;
}

#else

uint64_t FS_fgetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst)
{
    return 0;
}

uint64_t FS_fsetstat(struct fs_object_s *fso, const unsigned char type, void *ptr, uint64_t mask, struct fs_stat_s *fst)
{
    return 0;
}

#endif


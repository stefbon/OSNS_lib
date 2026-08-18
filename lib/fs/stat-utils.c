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
#include "stat.h"

#include <sys/stat.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

#include "stat.h"

#ifdef STATX_TYPE

/* GET statx values */

uint64_t FS_stat_get_ino(struct fs_stat_s *fst)
{
    return fst->stx.stx_ino;
}

uint16_t FS_stat_get_type(struct fs_stat_s *fst)
{
    return fst->stx.stx_mode & S_IFMT;
}

uint16_t FS_stat_get_mode(struct fs_stat_s *fst)
{
    return fst->stx.stx_mode & ~S_IFMT;
}

uint32_t FS_stat_get_nlink(struct fs_stat_s *fst)
{
    return fst->stx.stx_nlink;
}

uint32_t FS_stat_get_unique_uid(struct fs_stat_s *fst)
{
    return fst->stx.stx_uid;
}

uint32_t FS_stat_get_unique_gid(struct fs_stat_s *fst)
{
    return fst->stx.stx_gid;
}

off_t FS_stat_get_size(struct fs_stat_s *fst)
{
    return fst->stx.stx_size;

}

static void copy_statxtime2timespec(struct timespec_s *time, struct statx_timestamp *stxt)
{
    time->st_sec=(int64_t) stxt->tv_sec;
    time->st_nsec=(uint32_t) stxt->tv_nsec;
}

unsigned char FS_stat_get_time(struct fs_stat_s *fst, unsigned char type, struct timespec_s *time)
{
    unsigned char result=1;
    struct statx *stx=&fst->stx;

    switch (type) {

        case FS_STAT_TIME_ACCESS:

            copy_statxtime2timespec(time, &stx->stx_atime);
            break;

        case FS_STAT_TIME_MODIFY:

            copy_statxtime2timespec(time, &stx->stx_mtime);
            break;

        case FS_STAT_TIME_CHANGE:

            copy_statxtime2timespec(time, &stx->stx_ctime);
            break;

        case FS_STAT_TIME_CREATION:

            copy_statxtime2timespec(time, &stx->stx_btime);
            break;

        default:

            result=0;
            logoutput_debug("%s: type %u not supported", __FUNCTION__);


    }

    return result;

}

unsigned char FS_stat_get_dev(struct fs_stat_s *fst, struct fs_stat_dev_s *dev, unsigned char represented)
{
    struct statx *stx=&fst->stx;

    if (represented==0) {

        dev->major=stx->stx_dev_major;
        dev->minor=stx->stx_dev_minor;
        return 1;

    }

    /* no check it's a device ?? */

    dev->major=stx->stx_rdev_major;
    dev->minor=stx->stx_rdev_minor;
    return 1;

}

uint32_t FS_stat_get_blocks(struct fs_stat_s *fst)
{
    return fst->stx.stx_blocks;
}

uint32_t FS_stat_get_blksize(struct fs_stat_s *fst)
{
    return fst->stx.stx_blksize;
}

/* SET statx values */

void FS_stat_set_ino(struct fs_stat_s *fst, uint64_t ino)
{
    fst->stx.stx_ino=ino;
}

void FS_stat_set_type(struct fs_stat_s *fst, uint16_t type)
{
    uint16_t perm=(fst->stx.stx_mode & ~S_IFMT);

    fst->stx.stx_mode = (type & S_IFMT) | perm;
    fst->mask |= FS_STAT_TYPE;
}

void FS_stat_set_mode(struct fs_stat_s *fst, uint16_t mode)
{
    uint16_t type=(fst->stx.stx_mode & S_IFMT);

    fst->stx.stx_mode = type | (mode & ~S_IFMT);
    fst->mask |= FS_STAT_MODE;
}

void FS_stat_set_nlink(struct fs_stat_s *fst, uint32_t nlink)
{
    fst->stx.stx_nlink=nlink;
    fst->mask |= FS_STAT_NLINK;
}

void FS_stat_increase_nlink(struct fs_stat_s *fst, int32_t count)
{
    fst->stx.stx_nlink+=count;
}

void FS_stat_decrease_nlink(struct fs_stat_s *fst, int32_t count)
{
    uint32_t eff_count=((count <= fst->stx.stx_nlink) ? count : fst->stx.stx_nlink);

    fst->stx.stx_nlink-=eff_count;
}

void FS_stat_set_uid(struct fs_stat_s *fst, uint32_t uid)
{
    fst->stx.stx_uid=uid;
    fst->mask |= FS_STAT_UID;
}

void FS_stat_set_gid(struct fs_stat_s *fst, uint32_t gid)
{
    fst->stx.stx_gid=gid;
    fst->mask |= FS_STAT_GID;
}

void FS_stat_set_size(struct fs_stat_s *fst, off_t size)
{
    fst->stx.stx_size=size;
    fst->mask |= FS_STAT_SIZE;
}

/* with the statx a statx_timestamp is used, tv_sec is of type int64_t, tv_nsec of uint32_t */

static void copy_timespec2statxtime(struct fs_stat_s *fst, struct statx_timestamp *stxt, struct timespec_s *time, unsigned int flag)
{
    stxt->tv_sec=(int64_t) time->st_sec;
    stxt->tv_nsec=(uint32_t) time->st_nsec;
    fst->mask |= flag;
}

unsigned char FS_stat_set_time(struct fs_stat_s *fst, unsigned char type, struct timespec_s *time)
{
    unsigned char result=1;
    struct statx *stx=&fst->stx;

    switch (type) {

        case FS_STAT_TIME_ACCESS:

            copy_timespec2statxtime(fst, &stx->stx_atime, time, FS_STAT_ATIME);
            break;

        case FS_STAT_TIME_MODIFY:

            copy_timespec2statxtime(fst, &stx->stx_mtime, time, FS_STAT_MTIME);
            break;

        case FS_STAT_TIME_CHANGE:

            copy_timespec2statxtime(fst, &stx->stx_ctime, time, FS_STAT_CTIME);
            break;

        case FS_STAT_TIME_CREATION:

            copy_timespec2statxtime(fst, &stx->stx_btime, time, FS_STAT_BTIME);
            break;

        default:

            result=0;
            logoutput_debug("%s: type %u not supported", __FUNCTION__);


    }

    return result;

}

unsigned char FS_stat_set_dev(struct fs_stat_s *fst, struct fs_stat_dev_s *dev, unsigned char represented)
{
    struct statx *stx=&fst->stx;

    if (represented==0) {

        stx->stx_dev_major=dev->major;
        stx->stx_dev_minor=dev->minor;
        return 1;

    }

    /* no check it's a device ?? */
    stx->stx_rdev_major=dev->major;
    stx->stx_rdev_minor=dev->minor;
    return 1;

}

void FS_stat_set_blksize(struct fs_stat_s *fst, uint32_t blksize)
{
    fst->stx.stx_blksize=blksize;
}

void FS_stat_set_blocks(struct fs_stat_s *fst, uint32_t blocks)
{
    fst->stx.stx_blocks=blocks;
}

#else

/* GET stat values */

uint64_t FS_stat_get_ino(struct fs_stat_s *fst)
{
    return (uint64_t) fst->st.st_ino;
}

uint16_t FS_stat_get_type(struct fs_stat_s *fst)
{
    return (uint16_t) fst->st.st_mode & S_IFMT;
}

uint16_t FS_stat_get_mode(struct fs_stat_s *fst)
{
    return (uint16_t) fst->st.st_mode & ~S_IFMT;
}

uint32_t FS_stat_get_nlink(struct fs_stat_s *fst)
{
    return (uint32_t) fst->st.st_nlink;
}

uint32_t FS_stat_get_uid(struct fs_stat_s *fst)
{
    return (uint32_t) fst->st.st_uid;
}

uint32_t FS_stat_get_gid(struct fs_stat_s *fst)
{
    return (uint32_t) fst->st.st_gid;
}

off_t FS_stat_get_size(struct fs_stat_s *fst)
{
    return (off_t) fst->st.st_size;
}

static void copy_stattime2timespec(struct timespec_s *time, struct timeval_s *stt)
{
    time->st_sec=(int64_t) stt->tv_sec;
    time->st_nsec=(uint32_t) stt->tv_nsec;
}

unsigned char FS_stat_get_time(struct fs_stat_s *fst, unsigned char type, struct timespec_s *time)
{
    unsigned char result=1;
    struct stat *st=&fst->st;

    switch (type) {

        case FS_STAT_TIME_ACCESS:

            copy_stattime2timespec(time, &st->st_atim);
            break;

        case FS_STAT_TIME_MODIFY:

            copy_stattime2timespec(time, &st->st_mtim);
            break;

        case FS_STAT_TIME_CHANGE:

            copy_stattime2timespec(time, &st->st_ctim);
            break;

        case FS_STAT_TIME_CREATION:
        default:

            result=0;
            logoutput_debug("%s: type %u not supported", __FUNCTION__);


    }

    return result;

}

unsigned char FS_stat_get_dev(struct fs_stat_s *fst, struct fs_stat_dev_s *dev, unsigned char represented)
{
    struct stat *st=&fst->st;

    if (represented==0) {

        dev->major=major(st->st_dev);
        dev->minor=minor(st->st_dev);
        return 1;

    }

    /* no check it's a device ?? */
    dev->major=major(st->st_rdev);
    dev->minor=minor(st->st_rdev);
    return 1;

}

uint32_t FS_stat_get_blocks(struct fs_stat_s *fst)
{
    return fst->st.st_blocks;
}

uint32_t FS_stat_get_blksize(struct fs_stat_s *fst)
{
    return fst->st.st_blksize;
}

/* SET stat values */

void FS_stat_set_ino(struct fs_stat_s *fst, uint64_t ino)
{
    fst->st.st_ino=ino;
}

void FS_stat_set_type(struct fs_stat_s *fst, uint16_t type)
{
    uint16_t perm=(fst->st.st_mode & ~S_IFMT);

    fst->st.st_mode = (type & S_IFMT) | perm;
    fst->mask |= FS_STAT_TYPE;
}

void FS_stat_set_mode(struct fs_stat_s *fst, uint16_t mode)
{
    uint16_t type=(fst->st.st_mode & S_IFMT);

    fst->st.st_mode = type | (mode & ~S_IFMT);
    fst->mask |= FS_STAT_MODE;
}

void FS_stat_set_uid(struct fs_stat_s *fst, uint32_t uid)
{
    fst->st.st_uid=uid;
    fst->mask |= FS_STAT_UID;
}

void FS_stat_set_gid(struct fs_stat_s *fst, uint32_t gid)
{
    fst->st.st_gid=gid;
    fst->mask |= FS_STAT_GID;
}

void FS_stat_set_size(struct fs_stat_s *fst, off_t size)
{
    fst->st.st_size=size;
    fst->mask |= FS_STAT_SIZE;
}

void FS_stat_set_nlink(struct fs_stat_s *fst, uint32_t nlink)
{
    fst->st.st_nlink=nlink;
    fst->mask |= FS_STAT_NLINK;
}

void FS_stat_increase_nlink(struct fs_stat_s *fst, int32_t count)
{
    fst->st.st_nlink+=count;
}

void FS_stat_decrease_nlink(struct fs_stat_s *fst, int32_t count)
{
    uint32_t eff_count=((count <= fst->st.st_nlink) ? count : fst->st.st_nlink);

    fst->st.st_nlink-=eff_count;

}

/* with the stat a timespec is used, tv_sec is of type time_t, tv_nsec of long */

static void copy_timespec2statxtime(struct fs_stat_s *fst, struct timeval_s *stt, struct timespec_s *time, unsigned int flag)
{
    stt->tv_sec=(int64_t) time->st_sec;
    stt->tv_nsec=(uint32_t) time->st_nsec;
    fst->mask |= flag;
}

unsigned char FS_stat_set_time(struct fs_stat_s *fst, unsigned char type, struct timespec_s *time)
{
    unsigned char result=1;
    struct stat *st=&fst->st;

    switch (type) {

        case FS_STAT_TIME_ACCESS:

            copy_timespec2statxtime(fst, &st->st_atim, time, FS_STAT_ATIME);
            break;

        case FS_STAT_TIME_MODIFY:

            copy_timespec2statxtime(fst, &st->st_mtim, time, FS_STAT_MTIME);
            break;

        case FS_STAT_TIME_CHANGE:

            copy_timespec2statxtime(fst, &st->st_ctim, time, FS_STAT_CTIME);
            break;

        case FS_STAT_TIME_CREATION:
        default:

            result=0;
            logoutput_debug("%s: type %u not supported", __FUNCTION__);


    }

    return result;

}

unsigned char FS_stat_set_dev(struct fs_stat_s *fst, struct fs_stat_dev_s *dev, unsigned char represented)
{

    if (represented==0) {

        fst->st.st_dev=makedev(dev->major, dev->minor);
        return 1;

    }

    /* no check it's a device ?? */
    fst->st.st_rdev=makedev(dev->major, dev->minor);
    return 1;

}

void FS_stat_set_blksize(struct fs_stat_s *fst, uint32_t blksize)
{
    fst->st.st_blksize=blksize;
}

void FS_stat_set_blocks(struct fs_stat_s *fst, uint32_t blocks)
{
    fst->st.st_blocks=blocks;
}

#endif

uint32_t FS_stat_calc_amount_blocks(uint64_t size, uint32_t blksize)
{
    uint32_t count=(size / blksize);
    count += ((size % blksize)==0 ? 0 : 1);

    return count;
}

uint32_t FS_stat_dev_get_unique(struct fs_stat_dev_s *dev)
{
    return makedev(dev->major, dev->minor);
}

int FS_stat_test_ISDIR(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISDIR(fst->sst_mode);
#else
    return 0;
#endif
}

int FS_stat_test_ISLNK(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISLNK(fst->sst_mode);
#else
    return 0;
#endif
}

int FS_stat_test_ISSOCK(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISSOCK(fst->sst_mode);
#else
    return 0;
#endif
}

int FS_stat_test_ISCHR(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISCHR(fst->sst_mode);
#else
    return 0;
#endif
}

int FS_stat_test_ISBLK(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISBLK(fst->sst_mode);
#else
    return 0;
#endif
}

int FS_stat_test_ISREG(struct fs_stat_s *fst)
{
#ifdef __linux__
    return S_ISREG(fst->sst_mode);
#else
    return 0;
#endif
}

uint64_t FS_stat_copy(struct fs_stat_s *fsta, struct fs_stat_s *fstb, uint64_t mask)
{
    uint64_t maskset=0;

    if (mask==0) mask=FS_STAT_ALL;

    if (mask & FS_STAT_TYPE) {

        maskset|=FS_STAT_TYPE;
        FS_stat_set_type(fsta, FS_stat_get_type(fstb));

    }

    if (mask & FS_STAT_MODE) {

        maskset|=FS_STAT_MODE;
        FS_stat_set_mode(fsta, FS_stat_get_mode(fstb));

    }

    if (mask & FS_STAT_NLINK) {

        maskset|=FS_STAT_NLINK;
        FS_stat_set_nlink(fsta, FS_stat_get_nlink(fstb));

    }

    if (mask & FS_STAT_UID) {

        maskset|=FS_STAT_UID;
        FS_stat_set_uid(fsta, FS_stat_get_unique_uid(fstb));

    }

    if (mask & FS_STAT_GID) {

        maskset|=FS_STAT_GID;
        FS_stat_set_gid(fsta, FS_stat_get_unique_gid(fstb));

    }

    if (mask & FS_STAT_SIZE) {

        maskset|=FS_STAT_SIZE;
        FS_stat_set_size(fsta, FS_stat_get_size(fstb));

    }

    if (mask & FS_STAT_ATIME) {
        struct timespec_s atime=TIME_INIT;

        maskset|=FS_STAT_ATIME;
        FS_stat_get_time(fstb, FS_STAT_TIME_ACCESS, &atime);
        FS_stat_set_time(fsta, FS_STAT_TIME_ACCESS, &atime);

    }

    if (mask & FS_STAT_MTIME) {
        struct timespec_s mtime=TIME_INIT;

        maskset|=FS_STAT_MTIME;
        FS_stat_get_time(fstb, FS_STAT_TIME_MODIFY, &mtime);
        FS_stat_set_time(fsta, FS_STAT_TIME_MODIFY, &mtime);

    }

    if (mask & FS_STAT_CTIME) {
        struct timespec_s ctime=TIME_INIT;

        maskset|=FS_STAT_CTIME;
        FS_stat_get_time(fstb, FS_STAT_TIME_CHANGE, &ctime);
        FS_stat_set_time(fsta, FS_STAT_TIME_CHANGE, &ctime);

    }

    if (mask & FS_STAT_BTIME) {
        struct timespec_s btime=TIME_INIT;

        maskset|=FS_STAT_BTIME;
        FS_stat_get_time(fstb, FS_STAT_TIME_BIRTH, &btime);
        FS_stat_set_time(fsta, FS_STAT_TIME_BIRTH, &btime);

    }

    if (mask & FS_STAT_INO) {

        maskset|=FS_STAT_INO;
        FS_stat_set_ino(fsta, FS_stat_get_ino(fstb));

    }

    if (mask & FS_STAT_BLOCKS) {

        maskset|=FS_STAT_BLOCKS;
        FS_stat_set_gid(fsta, FS_stat_get_unique_gid(fstb));

    }

    return maskset;

}

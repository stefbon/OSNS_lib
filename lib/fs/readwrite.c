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
#include <sys/uio.h>

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-io.h"

#include "fs.h"

static unsigned int FS_parse_rw_how(const char *how)
{
    struct dstr_s tmp=DSTR_INIT;
    struct dstr_s part=DSTR_INIT;
    unsigned int flags=0;

    DSTR_set_bytes_raw(&tmp, how, 0, 0);

    while (DSTR_get_first_dstr(&tmp, ',', &part, 1, 1)) {

	if (DSTR_cmp_bytes(&part, "append", 0, 1, 0, 1)) {

	    flags |= RWF_APPEND;

	} else if (DSTR_cmp_bytes(&part, "dsync", 0, 1, 0, 1)) {

	    flags |= RWF_DSYNC;

	} else if (DSTR_cmp_bytes(&part, "hipri", 0, 1, 0, 1)) {

	    flags |= RWF_HIPRI;

	} else if (DSTR_cmp_bytes(&part, "sync", 0, 1, 0, 1)) {

	    flags |= RWF_SYNC;

	} else if (DSTR_cmp_bytes(&part, "nowait", 0, 1, 0, 1)) {

	    flags |= RWF_NOWAIT;

	} else if (DSTR_cmp_bytes(&part, "noappend", 0, 1, 0, 1)) {

	    flags |= RWF_NOAPPEND;

	} else if (DSTR_cmp_bytes(&part, "atomic", 0, 1, 0, 1)) {

	    flags |= RWF_ATOMIC;

	} else if (DSTR_cmp_bytes(&part, "dontcache", 0, 1, 0, 1)) {

	    flags |= RWF_DONTCACHE;

	} else {

	    logoutput_debug("%s: flagstr %.*s not reckognized", __FUNCTION__, part.length, part.str);

	}

    }

    return flags;

}


unsigned char FS_fsync(struct fs_object_s *fso, const char *what)
{
    unsigned char result=0;

    logoutput_debug("%s", __FUNCTION__);

    if (FS_object_valid(fso)==0) {

        logoutput_debug("%s: fs object not valid ... cannot continue", __FUNCTION__);
        return 0;

    }

#ifdef __linux__

    int fd=IO_object_backend_get_unix_fd(&fso->backend);
    int tmp=-1;

    if ((what==NULL) || (strcmp(what, "default")==0)) {

        tmp=fsync(fd);

    } else if (strcmp(what, "data")==0) {

        tmp=fdatasync(fd);

    } else {

        errno=ENOSYS;

    }

    if (tmp==0) {

        result=1;

    } else {

	logoutput_warning("%s: error %u on fd %i (%s)", __FUNCTION__, errno, fd, strerror(errno));

    }

#endif

    return result;

}

off64_t FS_pread(struct fs_object_s *fso, char *buffer, size_t size, off64_t offset, const char *how)
{
    off64_t result=0;
    unsigned int flags=(how ? FS_parse_rw_how(how) : 0);

#ifdef __linux__

    int fd=IO_object_backend_get_unix_fd(&fso->backend);
    struct iovec iov[1];
    ssize_t tmp=0;

    iov[0].iov_base=buffer;
    iov[0].iov_len=size;

    tmp=preadv2(fd, iov, 1, offset, flags);

    if (tmp==-1) {

        logoutput_warning("%s: error %u on fd %i (%s)", __FUNCTION__, errno, fd, strerror(errno));

    } else {

        fso->offset=offset + tmp;
        result=(off64_t) tmp;

    }

#endif

    return result;
}

off64_t FS_pwrite(struct fs_object_s *fso, char *buffer, size_t size, off_t offset, const char *how)
{
    off64_t result=0;
    unsigned int flags=(how ? FS_parse_rw_how(how) : 0);

#ifdef __linux__

    int fd=IO_object_backend_get_unix_fd(&fso->backend);
    struct iovec iov[1];
    ssize_t tmp=0;

    iov[0].iov_base=buffer;
    iov[0].iov_len=size;

    tmp=pwritev2(fd, iov, 1, offset, flags);

    if (tmp==-1) {

        logoutput_warning("%s: error %u on fd %i flags %u (%s)", __FUNCTION__, errno, fd, flags, strerror(errno));

    } else {

        fso->offset=offset + tmp;
        result=(off64_t) tmp;

    }

#endif

    return result;
}

off64_t FS_lseek(struct fs_object_s *fso, off64_t offset, int whence)
{
    off64_t result=(off64_t) -1;

#ifdef __linux__

    int fd=IO_object_backend_get_unix_fd(&fso->backend);

    result=lseek64(fd, offset, whence);

    if (result==(off64_t) -1) {

        logoutput_warning("%s: error %u on fd %i (%s)", __FUNCTION__, errno, fd, strerror(errno));

    }

#endif

    return result;
}

#ifdef __linux__

#include <dirent.h>

struct linux_dirent64 {
    ino64_t                                         d_ino;
    off64_t                                         d_off;
    unsigned short                                  d_reclen;
    unsigned char                                   d_type;
    char                                            d_name[];
};

static void copy_dirent_2_dentry(struct fs_object_s *fso, struct linux_dirent64 *de, struct fs_dentry_s *dentry)
{

    dentry->flags=0;
    dentry->type=DTTOIF(de->d_type);
    dentry->ino=de->d_ino;
    dentry->offset=fso->offset;
    dentry->name.str=de->d_name;

    /* it is not the right way to get the length of the name from other variables here like 
        de->d_reclen - offsetof(struct linux_dirent64, d_name) - 2 (see man getdents)
        since the length of the name may be less this ...
        the kernel may allocate more bytes than strictly required ...
        for that use strlen
    */

    dentry->name.length=strlen(de->d_name);

}

#endif

off64_t FS_readdentry(struct fs_object_s *fso, struct fs_dentry_s *dentry, unsigned char next)
{

#ifdef __linux__

    int fd=IO_object_backend_get_unix_fd(&fso->backend);
    struct io_buffer_s *buffer=&fso->buffer;
    struct linux_dirent64 *de=NULL;
    int left=0;
    unsigned int size=FS_DIRECTORY_DEFAULT_BUFFER_SIZE;
    unsigned char firsttime=0;

    if (lseek64(fd, 0, SEEK_CUR)==0) {

	firsttime=1;
	fso->offset=1;

    }

    allocatebuffer:

    if ((buffer->ptr==NULL) || (buffer->size<size)) {

        if (IO_buffer_allocate(buffer, size)==0) {

            logoutput_error("%s: unable to allocate %u bytes for directory buffer", __FUNCTION__, size);
            return 0;

        }

        memset(buffer->ptr, 0, size);

    }

    readfromsystem:

    if ((buffer->pos>=buffer->bytesread) || (buffer->flags & IO_BUFFER_FLAG_EOL)) {
        int fd=IO_object_backend_get_unix_fd(&fso->backend);
        int result=0;

        buffer->bytesread=0;
        buffer->pos=0;
        buffer->flags &= ~IO_BUFFER_FLAG_EOL;
        if (buffer->flags & (IO_BUFFER_FLAG_EOD | IO_BUFFER_FLAG_ERROR)) return 0;

        result=syscall(SYS_getdents64, fd, (struct linux_dirent64 *) buffer->ptr, buffer->size);

        if (result==-1) {

            if (errno==EINVAL) {
                unsigned int stepsize=512;

                size+=stepsize;
                logoutput_debug("%s: error %u ... buffer too small ... increasing with %u to %u bytes", __FUNCTION__, errno, stepsize, size);
                goto allocatebuffer;

            }

            logoutput_debug("%s: error %u calling SYS_getdents64 (%s)", __FUNCTION__, errno, strerror(errno));
            buffer->flags |= IO_BUFFER_FLAG_ERROR;
            return 0;

        } else if (result==0) {

            /* no more dentries available */
            buffer->flags |= IO_BUFFER_FLAG_EOD;
            return 0;

        }

        buffer->bytesread=(unsigned int) result;
        logoutput_debug("%s: read %u bytes size buffer %u", __FUNCTION__, buffer->bytesread, buffer->size);

    }

    left=(buffer->bytesread - buffer->pos);

    /* go to the next, but don't do this when the first time */

    if (next && (firsttime==0)) {

        de=(struct linux_dirent64 *)(buffer->ptr + buffer->pos);

        if ((left <= sizeof(struct linux_dirent64)) || (left <= de->d_reclen)) {

            buffer->flags |= IO_BUFFER_FLAG_EOL;
            goto readfromsystem;

        }

        buffer->pos += de->d_reclen;
        left -= de->d_reclen;
        fso->offset++;

    }

    de=(struct linux_dirent64 *)(buffer->ptr + buffer->pos);

    /* test there is enough space for the linux direntry */

    if ((left < sizeof(struct linux_dirent64)) || (left < de->d_reclen)) {

        buffer->flags |= IO_BUFFER_FLAG_EOL;
        goto readfromsystem;

    }

    copy_dirent_2_dentry(fso, de, dentry);

#endif

    return fso->offset;

}


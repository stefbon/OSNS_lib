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

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"
#include "libosns-fs.h"

#include <fcntl.h>
#include <sys/syscall.h>

#include "pid.h"

#ifdef __linux__

int PID_share_io(pid_t pid, unsigned int fd)
{
    int result=-1;

    if (pid==getpid()) {

        result=dup(fd);

    } else {

#ifdef SYS_pidfd_getfd

        int pidfd=syscall(SYS_pidfd_open, pid, 0);

        if (pidfd>=0) {

            result=syscall(SYS_pidfd_getfd, pidfd, fd, 0);
            if (result==-1) logoutput_debug("%s: unable to get fd .. errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));
            close(pidfd);

        } else {

            logoutput_debug("%s: unable to open pid .. errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));

        }
#else

        logoutput_debug("%s: share fd between processes not supported", __FUNCTION__);

#endif

    }

    return result;

}

unsigned char PID_share_io_supported()
{
#ifdef SYS_pidfd_getfd

    return 1;

#else

    return 0;

#endif

}

#else

int PID_share_io(pid_t pid, unsigned int fd)
{
    return -1;
}

unsigned char PID_share_io_supported()
{
    return 0;
}

#endif

void PID_info_init(struct pid_info_s *info)
{

    if (info==NULL) return;

    info->pid=0;
    info->uid=(uid_t) -1;
    info->gid=(gid_t) -1;

    FS_path_init(&info->executable);
}

void PID_info_clear(struct pid_info_s *info)
{
    if (info==NULL) return;
    FS_path_clear(&info->executable);
}

#ifdef __linux__

unsigned int PID_get_info(pid_t pid, unsigned int mask, struct pid_info_s *info)
{

    if ((mask==0) || (info==NULL)) return 0;

    if (mask & PID_INFO_MASK_PID) {

        info->pid=getpid();
        mask &= ~PID_INFO_MASK_PID;
        info->mask|=PID_INFO_MASK_PID;

        pid=info->pid;

    }

    if (pid==0) return 0;

    if (mask & (PID_INFO_MASK_UID | PID_INFO_MASK_GID | PID_INFO_MASK_EXECUTABLE)) { 
        char procpath[64];

        if (mask & (PID_INFO_MASK_UID | PID_INFO_MASK_GID)) {
            struct fs_stat_s fst;
            int result=snprintf(procpath, 64, "/proc/%u", pid);

            memset(&fst, 0, sizeof(struct fs_stat_s));

            if (FS_fgetstat(NULL, 'c', (void *) &procpath[0], (FS_STAT_UID | FS_STAT_GID), &fst)>0) {

                if (mask & PID_INFO_MASK_UID) {

                    info->uid=FS_stat_get_unique_uid(&fst);
                    info->mask|=PID_INFO_MASK_UID;

                }

                if (mask & PID_INFO_MASK_GID) {

                    info->gid=FS_stat_get_unique_gid(&fst);
                    info->mask|=PID_INFO_MASK_GID;

                }

            } else {

                logoutput_debug("%s: error fgetstat", __FUNCTION__);
                return 0;

            }

        }

        if (mask & PID_INFO_MASK_EXECUTABLE) {
            int result=snprintf(procpath, 64, "/proc/%u/exe", pid); /* under Linux (other unixes also?) the symlink /proc/%PID%/exe points to the executable file (full path) */
            struct dstr_s buffer=DSTR_INIT;

            if (FS_readlink(NULL, 'c', (void *) &procpath[0], &buffer, FS_READLINK_FLAG_ALLOCATE)) {

                info->mask|=PID_INFO_MASK_EXECUTABLE;
                FS_path_set(&info->executable, 's', (void *) &buffer, 0);
                logoutput_debug("%s: found exe %.*s", __FUNCTION__, buffer.length, buffer.str);
                DSTR_init(&buffer);

            } else {

                DSTR_clear(&buffer);
                logoutput_debug("%s: error readlink", __FUNCTION__);
                return 0;

            }

        }

    }

    return info->mask;

}

#else

unsigned int PID_get_info(pid_t pid, unsigned int mask, struct pid_info_s *info)
{
    return 0;
}

#endif

#ifdef __linux__

unsigned char PID_get_unique_pid(pid_t *p_pid)
{

    if (p_pid) {

        *p_pid=getpid();
        return 1;

    }

    return 0;
}

#else

unsigned char PID_get_unique_pid(pid_t *p_pid)
{
    return 0;
}

#endif


#ifdef HAVE_LIBSYSTEMD

#include <systemd/sd-login.h>

unsigned char PID_get_user_session(pid_t pid, struct dstr_s *session)
{
    char *tmp=NULL;

    if (sd_pid_get_session(pid, &tmp)==0) {

        DSTR_set_bytes(session, tmp, strlen(tmp), 0);
        session->flags |= DSTR_FLAG_ALLOC_DATA;

    }

    return (tmp) ? 1 : 0;

}

unsigned char PID_get_user_slice(pid_t pid, struct dstr_s *slice)
{
    char *tmp=NULL;

    if (sd_pid_get_user_slice(pid, &tmp)==0) {

        DSTR_set_bytes(slice, tmp, strlen(tmp), 0);
        slice->flags |= DSTR_FLAG_ALLOC_DATA;

    }

    return (tmp) ? 1 : 0;

}

unsigned char PID_get_user_cgroup(pid_t pid, struct dstr_s *cgroup)
{
    char *tmp=NULL;

    if (sd_pid_get_cgroup(pid, &tmp)==0) {

        DSTR_set_bytes(cgroup, tmp, strlen(tmp), 0);
        cgroup->flags |= DSTR_FLAG_ALLOC_DATA;

    }

    return (tmp) ? 1 : 0;

}

#else

unsigned char PID_get_user_session(pid_t pid, struct dstr_s *session)
{
    return 0;
}

unsigned char PID_get_user_slice(pid_t pid, struct dstr_s *slice)
{
    return 0;
}

unsigned char PID_get_user_cgroup(pid_t pid, struct dstr_s *cgroup)
{
    return 0;
}

#endif

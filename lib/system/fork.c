/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include <fcntl.h>

#include "libosns-log.h"
#include "libosns-socket.h"
#include "libosns-path.h"
#include "libosns-system.h"

#ifdef __linux__

#include <dirent.h>

#define SYSTEM_FORK_FLAG_CLOSE_ALL                              1
#define SYSTEM_FORK_FLAG_REDIRECT_STD                           2
#define SYSTEM_FORK_FLAG_SWITCH_LOG_SYSLOG                      4
#define SYSTEM_FORK_FLAG_CHDIR_ROOT                             8
#define SYSTEM_FORK_FLAG_SET_SESSION_ID                         16

int SYSTEM_fork(unsigned int flags, const char *logname, int loglevel)
{
    pid_t pid=0;
    int tmp=0;

    pid=fork();

    switch (pid) {

	case -1:

            /* failed to fork */

	    logoutput_warning("%s: errcode %u (%s)", __FUNCTION__, errno, strerror(errno));
	    return -1;

	case 0:

            /* in the new forked process */

            logoutput_debug("%s: new forked pid %u", __FUNCTION__, getpid());
	    break;

	default:

            /* remain in the calling process */

	    return (int) pid;

    }


    if (flags & SYSTEM_FORK_FLAG_SET_SESSION_ID) {

        /* become a new process leader */

        int tmp=setsid();

        if (tmp == -1) {

	    logoutput_warning("%s: errcode %u (%s) setsid", __FUNCTION__, errno, strerror(errno));
	    return -1;

        }

    }

    if (flags & SYSTEM_FORK_FLAG_CHDIR_ROOT) {

        /* change to root */

        int tmp=chdir("/");

    }

    if (flags & SYSTEM_FORK_FLAG_REDIRECT_STD) {
        int fd = open("/dev/null", O_RDWR, 0);

        if (fd==-1) {

            logoutput_warning("%s: errcode %u (%s) open /dev/null", __FUNCTION__, errno, strerror(errno));

        } else if (fd>=0) {

	    (void) dup2(fd, STDIN_FILENO);
	    (void) dup2(fd, STDOUT_FILENO);
	    (void) dup2(fd, STDERR_FILENO);
	    if (fd > 2) close(fd);

        }

    }

    if (flags & SYSTEM_FORK_FLAG_CLOSE_ALL) {
        char procpath[64];
        int tmp=snprintf(procpath, 64, "/proc/%u/fd/", getpid());
        DIR *dp=opendir(procpath);
        struct dirent *de=NULL;

        while ((de=readdir(dp))) {
            int fd=atoi(de->d_name);

            if ((fd>=3) && (fd != dirfd(dp))) close(fd);

        }

        closedir(dp);

    }

    /* from here in the new forked process */

    if (flags & SYSTEM_FORK_FLAG_SWITCH_LOG) {

        /* set the log target */

        if (logname) LOGGING_switch_backend(logname);
        if (loglevel>=0) LOGGING_set_level((unsigned int) loglevel);

    }

    return 0;

}

#else

int SYSTEM_fork(unsigned int flags, const char *logname, int loglevel)
{
    return -1;
}

#endif

#ifdef __linux__

int SYSTEM_execve(struct fs_path_s *path, char *argv[])
{
    unsigned int size=FS_path_export(path, NULL, 1);
    char buffer[size];
    int result=0;

    size=FS_path_export(path, buffer, 1);
    logoutput_debug("%s: exec %s", __FUNCTION__, buffer);
    if (argv[0]==NULL) argv[0]=buffer;
    result=execv(buffer, argv);

    /* when here it must be be an error */

    if (result==-1) logoutput_warning("%s: error %i (%s)", __FUNCTION__, errno, strerror(errno));
    return result;

}

#else

int SYSTEM_execve(struct fs_path_s *path, char *argv[])
{
    return -1;
}

#endif

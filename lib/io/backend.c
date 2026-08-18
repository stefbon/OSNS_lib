/* SPDX-License-Identifier: GLP-2.0-only */

#include "io.h"

#include <fcntl.h>

static unsigned char io_backend_close_fd(struct io_object_backend_s *b)
{

    if (b->handle.fd>=0) {

	close(b->handle.fd);
	b->handle.fd=-1;

    }

}

void IO_object_backend_init(struct io_object_backend_s *b, unsigned char type)
{
    b->type=type;

    if (type==IO_OBJECT_BACKEND_TYPE_FD) {

	b->handle.fd=-1;
	b->close=io_backend_close_fd;
    }

}

unsigned char IO_object_backend_get_error(struct io_object_backend_s *obck, struct error_s *error)
{
    unsigned char result=0;

#ifdef __linux__

    int errnum=0;
    socklen_t errlen=sizeof(errnum);
    int fd=IO_object_backend_get_unix_fd(obck);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *) &errnum, &errlen)==0) {

	if (errnum>0) {

	    ERROR_set_errnum(error, errnum);
	    result=1;

	}

    }
#else

    return result;

#endif

}

static int io_object_backend_set_system_flag(struct io_object_backend_s *b, unsigned int flag, int set)
{
    int result=-1;

    if (b->type == IO_OBJECT_BACKEND_TYPE_FD) {

#ifdef __linux__
        int fd=b->handle.fd;
	int flags = fcntl(fd, F_GETFL, 0);

        if (flags==-1) {

            logoutput_debug("%s: unable to set backend/fd flags using fcntl ... error %u (%s)", __FUNCTION__, errno, strerror(errno));
            return -1;

        }

        if (set) {

            /* only set when not already */

	    result=((flags & flag)==0) ? fcntl(fd, F_SETFL, (flags | flag)) : 0;

        } else {

            result=0;

            if (flags & flag) {

                flags &= ~flag;
                result=fcntl(fd, F_SETFL, flags);

            }

        }

#endif

    } else {

        logoutput_debug("%s: unable to get error from backend (%u) other than fd", __FUNCTION__, b->type);

    }

    return result;

}

unsigned char IO_object_backend_is_open(struct io_object_backend_s *b)
{
    int flags=-1;

    if (b->type == IO_OBJECT_BACKEND_TYPE_FD) {

#ifdef __linux__

        flags=fcntl(b->handle.fd, F_GETFL, 0);

#endif

    }

    return (flags>=0) ? 1 : 0;
}

unsigned char IO_object_backend_close(struct io_object_backend_s *b)
{
    int flags=-1;

    if (b->type == IO_OBJECT_BACKEND_TYPE_FD) {

#ifdef __linux__

	if (b->handle.fd>=0) {

    	    close(b->handle.fd);
    	    b->handle.fd=-1;

	}

#endif

    }

    return (flags>=0) ? 1 : 0;
}

int IO_object_backend_get_unix_fd(struct io_object_backend_s *b)
{
    return ((b->type==IO_OBJECT_BACKEND_TYPE_FD) ? b->handle.fd : -1);
}

unsigned char IO_object_backend_set_unix_fd(struct io_object_backend_s *b, int fd)
{
    unsigned char result=0;

    if ((b->type == IO_OBJECT_BACKEND_TYPE_FD) || (b->type==0)) {

        b->handle.fd=fd;
        result=1;
        if (b->type==0) b->type=IO_OBJECT_BACKEND_TYPE_FD;

    }

    return result;
}

unsigned char IO_object_backend_set_non_blocking(struct io_object_backend_s *b, unsigned char enable)
{
    int result=io_object_backend_set_system_flag(b, O_NONBLOCK, ((enable) ? 1 : 0));
    return (result>=0) ? 1 : 0;
}

unsigned char IO_object_backend_set_cloexec(struct io_object_backend_s *b, unsigned char enable)
{
    int result=io_object_backend_set_system_flag(b, O_CLOEXEC, ((enable) ? 1 : 0));
    return (result>=0) ? 1 : 0;
}

/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef _LIB_IO_IO_H
#define _LIB_IO_IO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-misc.h"
#include "libosns-error.h"
#include "libosns-datatypes.h"

#include "buffer.h"
#include "backend.h"

struct ip_address_s;
struct network_port_s;
struct fs_path_s;

#define IO_OBJECT_BACKEND_TYPE_FD                                       1
#define IO_OBJECT_BACKEND_TYPE_DATA                                     2
#define IO_OBJECT_BACKEND_TYPE_PTR                                      3

struct io_object_backend_s {
    unsigned char                                                       type;
    unsigned char                                                       (* close)(struct io_object_backend_s *backend);
    union io_object_backend_u {
        int                                                             fd;
        struct dstr_s                                                   data;
        void                                                            *ptr;
    } handle;
};

struct io_addr_object_s {
    union io_object_addr_u {
        struct sockaddr_un                                              local;
        struct sockaddr_in                                              inet4;
        struct sockaddr_in6                                             inet6;
    } type;
    struct sockaddr					                *addr; /* link to any of the ones above */
    socklen_t						                length;
};

struct io_connection_object_s {
    struct io_object_backend_s                                          backend; /* reference to the kernel object like fd */
    unsigned int                                                       	openflags; /* stream, dgram, .... and nonblock and cloexec */
    unsigned int                                                        recvflags;
    unsigned int                                                        sendflags;
    struct io_addr_object_s                                             addr;
};

/* io with the system like (under Linux):

    - fifo
    - block device
    - character device (like used by FUSE)
        (See for some info: man 2 mknod
    - fd for reading signals using signalfd
    - fd for(reading timers using timerfd_create
    - .... etc ....

    in general there is no need to have a special structure for these cases
    since they are special, and all relevant optgions are to be handled in
    dedicated files,

    for exaomple a signal handler using signalfd. This will give a filedescriptor,
    To make use of the functionality the best way to do is is a dedicated source file
    where all relevant function reside, like:

    - open signal handler
    - close sh
    - add to eventloop
    - remove from eventloop
    - change/remove signals to watch for
    - read events and call a customable cb to inform ctx/environment about event/signal

    others, like a timer montor (using timerfd_create) has the exact same behaviour and
    can be solved the same way. And also, these calls are system/Linux specific, and therefore
    have to be kept in one dedicated source file.

    So there is no need for a generic structure for these kind of fd's,
    ("they are not coming outside the dedicated source file"), but there is (at least) 
    one exception, and that's FUSE. In the construction I've written, fd's of the fuse
    connection with the kernel have to be send and/or shared (using pidfd). This created
    in my opnion the need for the a special structure for "system" connections.

*/

#define IO_SYSTEM_OBJECT_NAME_LENGTH                                    64

struct io_system_object_s {
    struct io_object_backend_s                                          backend; /* reference to the kernel object like fd */
    char                                                               	name[IO_SYSTEM_OBJECT_NAME_LENGTH];
};

struct io_object_local_properties_s {
#ifdef __linux__
    uid_t                                                               uid;
    gid_t                                                               gid;
    pid_t                                                               pid;
#endif
};

#define IO_OBJECT_PROPERTIES_TYPE_LOCAL                                 1
#define IO_OBJECT_PROPERTIES_TYPE_NETWORK                               2
#define IO_OBJECT_PROPERTIES_TYPE_CUSTOM                                3

struct io_object_properties_s {
    unsigned char                                                       type;
    unsigned char                                                       remote;
    union io_object_properties_u {
        struct io_object_local_properties_s                             local;
        struct io_addr_object_s                                         network;
        struct dstr_s                                                  	custom;
    } scope;
};

#define IO_OBJECT_TYPE_CONNECTION                                       1
#define IO_OBJECT_TYPE_SYSTEM                                           2

struct io_object_s {
    unsigned char                                                       type;
    union io_object_u {
        struct io_connection_object_s                                   connection;
        struct io_system_object_s                                       system;
    } io;
    struct io_object_backend_s                                          *backend;
};

/* Prototypes */

void IO_object_init(struct io_object_s *object, unsigned char type);
unsigned char IO_object_valid(struct io_object_s *object);
void IO_object_clear(struct io_object_s *object);

unsigned char IO_object_copy(struct io_object_s *ao, struct io_object_s *bo, unsigned char cb);
unsigned char IO_object_close(struct io_object_s *ao);
unsigned char IO_object_is_open(struct io_object_s *ao);
unsigned char IO_object_get_error(struct io_object_s *ao, struct error_s *error);

#ifdef __linux__

int IO_object_get_unix_fd(struct io_object_s *ao);
void IO_object_set_unix_fd(struct io_object_s *ao, int fd);

#endif

int IO_object_get_properties(struct io_object_s *ao, struct io_object_properties_s *prop, unsigned char remote);

#endif

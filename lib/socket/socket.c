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
#include "libosns-event.h"
#include "libosns-fs.h"
#include "libosns-eventloop.h"


#include "socket.h"
#include "ctx.h"

static unsigned int unique_ctr=0;

/* a read event on a listning socket means an incoming connection, read the data with accept4 and creates a connection
    see man accept4
*/

static void SOCKET_io_read(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct socket_s *sock=(struct socket_s *)((char *)bctx - offsetof(struct socket_s, bctx));
    struct io_object_s *ios=&sock->object;
    struct io_object_s ioc;
    unsigned char acceptdone=0;
    unsigned char tmp=IO_object_copy(&ioc, ios, 0);

#ifdef __linux__

    if (ios->type==IO_OBJECT_TYPE_CONNECTION) {
        int fds=IO_object_get_unix_fd(ios);
        socklen_t length=ioc.io.connection.addr.length;
        int fdc=accept4(fds, ioc.io.connection.addr.addr, &length, SOCK_CLOEXEC);

        IO_object_set_unix_fd(&ioc, fdc);
        acceptdone=(fdc>=0) ? 1 : 0;

    }

#endif

    if (acceptdone) {

        if ((* sock->ctx->accept)(sock->ctx, &ioc)==1) {

            logoutput_debug("%s: connection accepted by context", __FUNCTION__);
            return;

        }

    }

    logoutput_debug("%s: connection not accepted by context", __FUNCTION__);
    if (ioc.backend) IO_object_backend_close(ioc.backend);

}

static void SOCKET_io_close(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct socket_s *sock=(struct socket_s *)((char *) bctx - offsetof(struct socket_s, bctx));

    /* close from remote .... can this happen ? */

    if (IO_object_close(&sock->object)) (* sock->ctx->close)(sock->ctx);
}

static void SOCKET_io_error(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct socket_s *sock=(struct socket_s *)((char *) bctx - offsetof(struct socket_s, bctx));

    /* error from remote .... can this happen ? */

    if (IO_object_close(&sock->object)) (* sock->ctx->close)(sock->ctx);
}

int SOCKET_add_to_eventloop(struct socket_s *sock, struct beventloop_s *eloop)
{
    int result=-1;

    if (sock==NULL) return -1;
    if (BEVENT_ctx_attach_to_eventloop(eloop, &sock->bctx)) {

        /* set cb's */

        BEVENT_ctx_set_cb(&sock->bctx, BEVENT_EVENT_BIT_READABLE, SOCKET_io_read);
	BEVENT_ctx_set_cb(&sock->bctx, BEVENT_EVENT_BIT_CLOSE, SOCKET_io_close);
	BEVENT_ctx_set_cb(&sock->bctx, BEVENT_EVENT_BIT_ERROR, SOCKET_io_error);

        /* add */

	BEVENT_ctx_add(eloop, &sock->bctx, 0, 0);
        result=1;

    }

    return result;

}

void SOCKET_remove_from_eventloop(struct socket_s *sock)
{
    BEVENT_ctx_detach(&sock->bctx);
}

unsigned char SOCKET_set_network_address(struct socket_s *sock, struct ip_address_s *ip, struct network_port_s *port)
{
    struct io_connection_object_s *ico=NULL;

    if (sock->type && (sock->type != SOCKET_TYPE_NETWORK)) {

        logoutput_warning("%s: type of socket is %u ... not of type network ... cannot continue", __FUNCTION__, sock->type);
        return 0;

    }

    ico=&sock->object.io.connection;
    if (sock->type==0) sock->type=SOCKET_TYPE_NETWORK;

    return IO_connection_object_set_network_address(ico, ip, port);
}

unsigned char SOCKET_set_local_address(struct socket_s *sock, struct fs_path_s *path)
{
    struct io_connection_object_s *ico=NULL;

    if (sock->type && (sock->type != SOCKET_TYPE_LOCAL)) {

        logoutput_warning("%s: type of connection is %u ... not of type local ... cannot continue", __FUNCTION__, sock->type);
        return 0;

    }

    ico=&sock->object.io.connection;
    if (sock->type==0) sock->type=SOCKET_TYPE_LOCAL;

    return IO_connection_object_set_local_address(ico, path);

}

/* initialization */

void SOCKET_init(struct socket_s *sock, unsigned int type, struct socket_ctx_s *ctx)
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    memset(sock, 0, sizeof(struct socket_s));

    sock->type=type;
    sock->status=SOCKET_STATUS_INIT;
    sock->flags=0;

    EVENT_signal_lock(esignal);
    sock->unique=unique_ctr;
    unique_ctr++;
    EVENT_signal_unlock(esignal);

    IO_object_init(&sock->object, IO_OBJECT_TYPE_CONNECTION);
    BEVENT_ctx_init(&sock->bctx, sock->object.backend, 0);

    sock->ctx=((ctx) ? ctx : SOCKET_ctx_get_default());
}

void SOCKET_set_connection_type_stream(struct socket_s *sock)
{
    struct io_connection_object_s *ico=&sock->object.io.connection;

    unsigned char tmp=IO_connection_set_openflag(ico, "stream");
}

void SOCKET_set_connection_type_dgram(struct socket_s *sock)
{
    struct io_connection_object_s *ico=&sock->object.io.connection;

    unsigned char tmp=IO_connection_set_openflag(ico, "dgram");
}

void SOCKET_close(struct socket_s *sock, unsigned char remove)
{

    BEVENT_ctx_detach(&sock->bctx);
    if (IO_object_close(&sock->object)) logoutput_debug("%s: socket closed", __FUNCTION__);

    if (remove) {

        /* remove: only incase of a named socket, which is visible in the filesystem
            also remove the file */

        if (sock->type==SOCKET_TYPE_LOCAL) {
            struct fs_path_s path=FS_PATH_INIT;
            struct io_connection_object_s *ico=&sock->object.io.connection;

            if (IO_connection_object_get_fs_path(ico, &path)==1) {

                int tmp=FS_rm(NULL, 'p', (void *) &path);

            }

        }

    }

}

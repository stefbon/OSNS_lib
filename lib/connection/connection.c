/*
 
  2010, 2011, 2012, 2013, 2014, 2015 Stef Bon <stefbon@gmail.com>

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
#include "libosns-datatypes.h"
#include "libosns-misc.h"
#include "libosns-main.h"
#include "libosns-eventloop.h"
#include "libosns-error.h"
#include "libosns-event.h"

#include "connection.h"

static struct event_shared_signal_s *esignal_connections; /* dedicated signal for connections only */
static unsigned char initdone=0;

static void CONNECTION_module_init()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(esignal)) {

	if (initdone==0) {

	    initdone=1;
	    esignal_connections=EVENT_signal_create_custom();

	}

	EVENT_signal_unlock(esignal);

    }

}

void CONNECTION_init(struct connection_s *conn, struct event_shared_signal_s *esignal, unsigned char type, unsigned char role)
{

    /* handle module specific data once */

    CONNECTION_module_init();

    /* initialize connection */

    memset(conn, 0, sizeof(struct connection_s));

    conn->type=type;
    conn->status=0;
    conn->flags=0;
    conn->errcode=0;
    conn->unique=0;
    conn->esignal=(esignal ? esignal : esignal_connections);

    if (role==CONNECTION_ROLE_CLIENT) {

        conn->flags |= CONNECTION_FLAG_ROLE_CLIENT;

    } else if (role==CONNECTION_ROLE_SERVER) {

        conn->flags |= CONNECTION_FLAG_ROLE_SERVER;

    }

    LIST_element_init(&conn->list, NULL);
    IO_object_init(&conn->object, IO_OBJECT_TYPE_CONNECTION);
    BEVENT_ctx_init(&conn->bctx, conn->object.backend, 0);

}

unsigned char CONNECTION_set_network_address(struct connection_s *conn, struct ip_address_s *ip, struct network_port_s *port)
{

    if (conn->type && (conn->type != CONNECTION_TYPE_NETWORK)) {

        logoutput_warning("%s: type of connection is %u ... not of type network ... cannot continue", __FUNCTION__, conn->type);
        return 0;

    }

    if (conn->type==0) conn->type=CONNECTION_TYPE_NETWORK;
    return IO_connection_object_set_network_address(&conn->object.io.connection, ip, port);
}

unsigned char CONNECTION_set_local_address(struct connection_s *conn, struct fs_path_s *path)
{

    if (conn->type && (conn->type != CONNECTION_TYPE_LOCAL)) {

        logoutput_warning("%s: type of connection is %u ... not of type local ... cannot continue", __FUNCTION__, conn->type);
        return 0;

    }

    if (conn->type==0) conn->type=CONNECTION_TYPE_LOCAL;
    return IO_connection_object_set_local_address(&conn->object.io.connection, path);

}

unsigned char CONNECTION_connect(struct connection_s *conn)
{
    unsigned char result=0;

    /* address has to be set AND
        type connection has to be set AND
        this connection may not be on the server end (cause then the accept call has to be used)
    */

    if ((IO_object_valid(&conn->object)==0) || (conn->flags & CONNECTION_FLAG_ROLE_SERVER)) return 0;

#ifdef __linux__

    int fd=-1;
    struct io_connection_object_s *ico=&conn->object.io.connection;
    unsigned int type=(ico->openflags | SOCK_CLOEXEC | SOCK_NONBLOCK); /* per default always non blocking and close on exec */

    fd=socket(ico->addr.addr->sa_family, type, 0);

    if (fd==-1) {

        logoutput_debug("%s: unable to create socket ... errcode %u (%s)", __FUNCTION__, errno, strerror(errno));
        return 0;

    }

    if (connect(fd, ico->addr.addr, ico->addr.length)==-1) {

        logoutput_debug("%s: unable to connect ... errcode %u (%s)", __FUNCTION__, errno, strerror(errno));
        close(fd);
        return 0;

    }

    IO_object_set_unix_fd(&conn->object, fd);
    result=1;

#endif

    return result;

}

unsigned char CONNECTION_close(struct connection_s *conn)
{
    struct io_object_s *object=&conn->object;

    return IO_object_close(object);
}

unsigned char CONNECTION_is_open(struct connection_s *conn)
{
    struct io_object_s *object=&conn->object;

    return IO_object_is_open(object);
}


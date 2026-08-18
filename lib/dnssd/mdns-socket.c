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

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"
#include "libosns-list.h"

#include "dnssd.h"

#include "mdns-utils.h"
#include "mdns-interface.h"

/* ADD and REMOVE a mdns socket */

struct mdns_socket_s *MDNS_socket_add(struct list_header_s *header, struct sockaddr *saddr, struct mdns_socket_ctx_s *mctx)
{
    struct mdns_socket_s *msock=malloc(sizeof(struct mdns_socket_s));

    if (msock==NULL) {

        logoutput_debug("%s: unable to allocate mdns socket", __FUNCTION__);
        return NULL;

    }

    memset(msock, 0, sizeof(struct mdns_socket_s));

    msock->mctx=mctx;

    IO_object_init(&msock->object, IO_OBJECT_TYPE_CONNECTION);
    IO_connection_object_set(&msock->object.io.connection, saddr);

    LIST_element_init(&msock->list, NULL);
    LIST_header_add_last(header, &msock->list);
    BEVENT_ctx_init(&msock->bctx, msock->object.backend, NULL);

    logoutput_debug("%s: created mdns socket", __FUNCTION__);

    return msock;

}

void MDNS_socket_remove(struct mdns_socket_s *msock)
{

    BEVENTLOOP_detach_bctx(&msock->bctx);
    IO_object_close(&msock->object);
    LIST_element_remove(&msock->list);
    free(msock);

}

/* EVENTS like data available, close and error */

static void mdns_socket_recv_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct mdns_socket_s *msock=(struct mdns_socket_s *)((char *)bctx - offsetof(struct mdns_socket_s, bctx));
    struct mdns_socket_ctx_s *mctx=msock->mctx;
    struct generic_error_s error;
    int bytesread=0;
    char tmpbuffer[32];

    logoutput_debug("%s", __FUNCTION__);

    ERROR_init(&error, "system");
    EVENT_signal_lock_flag(mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_READ);

    while (bytesread>=0) {

        /* check how much data is available to allocate using the PEEK flag */

        bytesread=IO_connection_recvmsg_peek_cb(&msock->object.io.connection, &error);

        if (bytesread>0) {
            char buffer[(unsigned int) bytesread];

            /* data available */

            logoutput_debug("%s: mdns socket %u bytes available", __FUNCTION__, (unsigned int) bytesread);
            size_t numberrecords=MDNS_recv_query(msock, buffer, (unsigned int) bytesread);

        } else if (bytesread==0) {

            /* what to do here ?*/
            logoutput_debug("%s: mdns socket closed", __FUNCTION__);
            break;

        } else {

            /* error */
            logoutput_debug("%s: mdns socket error %s", __FUNCTION__, (* error.get_description)(&error));
            break;

        }

    }

    EVENT_signal_unlock_flag(mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_READ);

}

static void mdns_socket_close_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{}

static void mdns_socket_error_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{}

int MDNS_socket_add_to_eventloop(struct mdns_socket_s *msock, struct beventloop_s *loop)
{
    int result=-1;

    if (msock==NULL) return -1;

    if (BEVENTLOOP_attach_bctx(loop, &msock->bctx, msock->object.backend)) {

        /* set cb's */

        BEVENT_ctx_set_cb(&msock->bctx, BEVENT_FLAG_DATA, mdns_socket_recv_event);
	BEVENT_ctx_set_cb(&msock->bctx, BEVENT_FLAG_CLOSE, mdns_socket_close_event);
	BEVENT_ctx_set_cb(&msock->bctx, BEVENT_FLAG_ERROR, mdns_socket_error_event);
	BEVENT_ctx_add(&msock->bctx, 0);
        result=1;

    }

    return result;

}

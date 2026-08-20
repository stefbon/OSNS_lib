/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"
#include "libosns-list.h"
#include "libosns-event.h"

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
    BEVENT_ctx_init(&msock->bctx, msock->object.backend, 0);

    logoutput_debug("%s: created mdns socket", __FUNCTION__);

    return msock;

}

void MDNS_socket_remove(struct mdns_socket_s *msock)
{

    BEVENT_ctx_detach(&msock->bctx);
    IO_object_close(&msock->object);
    LIST_element_remove(&msock->list);
    free(msock);

}

/* EVENTS like data available, close and error */

static void mdns_socket_recv_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct mdns_socket_s *msock=(struct mdns_socket_s *)((char *)bctx - offsetof(struct mdns_socket_s, bctx));
    struct mdns_socket_ctx_s *mctx=msock->mctx;
    int bytesread=0;
    struct error_s error;
    char tmpbuffer[32];

    logoutput_debug("%s", __FUNCTION__);

    ERROR_init(&error);
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

	    if (error.errnum==EAGAIN) {

		bytesread=0;
		continue;

	    }

            logoutput_debug("%s: mdns socket error %s", __FUNCTION__, ERROR_get_description(&error));
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

    if (BEVENT_ctx_attach_to_eventloop(loop, &msock->bctx)) {

        /* set cb's */

        BEVENT_ctx_set_cb(&msock->bctx, BEVENT_EVENT_BIT_READABLE, mdns_socket_recv_event);
	BEVENT_ctx_set_cb(&msock->bctx, BEVENT_EVENT_BIT_CLOSE, mdns_socket_close_event);
	BEVENT_ctx_set_cb(&msock->bctx, BEVENT_EVENT_BIT_ERROR, mdns_socket_error_event);
	BEVENT_ctx_add(NULL, &msock->bctx, 0, 0);
        result=1;

    }

    return result;

}

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

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-network.h"

#include "dnssd.h"
#include "addrs.h"
#include "mdns-socket.h"
#include "mdns-socket-ctx.h"
#include "mdns-interface.h"

static struct program_module_s dnssd_module= {
    .status                                     = 0,
    .esignal                                    = EVENT_SHARED_SIGNAL_DEFAULT_INIT_P,
};

static void dnssd_manager_start_init()
{
}

static void dnssd_manager_refresh_init()
{
}

static void dnssd_manager_finish_init()
{
}

#define DNSSD_MANAGER_FLAG_LOCK_MSOCKETS        1

struct dnssd_manager_s {
    unsigned int                                status;
    struct list_header_s                        msockets;
    struct mdns_socket_ctx_s                    *mctx;
    void                                        (* start)();
    void                                        (* refresh)();
    void                                        (* finish)();
};

static struct dnssd_manager_s manager = {
    .start                              = dnssd_manager_start_init,
    .refresh                            = dnssd_manager_refresh_init,
    .finish                             = dnssd_manager_finish_init,
};

static unsigned char dnssd_add_mdns_socket_cb(struct io_connection_object_s *ico, unsigned int flags, void *ptr)
{
    struct mdns_socket_s *msock=NULL;
    struct list_element_s *list=NULL;

    if (MDNS_skip_network_interface_addr(ico, flags)) return 0;

    /* compare to the already used sockets */

    list=LIST_header_get_first(&manager.msockets);

    while (list) {

        msock=(struct mdns_socket_s *)((char *)list - offsetof(struct mdns_socket_s, list));
        if (IO_connection_object_compare(&msock->object.io.connection, ico->addr.addr)==1) break;
        list=LIST_element_get_next(list);
        msock=NULL;

    }

    if (msock) return 0;

    msock=MDNS_socket_add(&manager.msockets, ico->addr.addr, manager.mctx);

    if (msock==NULL) {

        logoutput_debug("%s: unable to create mdns socket", __FUNCTION__);
        return 0;

    }

    return 1;

}

static void dnssd_manager_get_all_mdns_sockets(unsigned char refresh)
{
    int result=0;
    struct list_element_s *list=NULL;

    if (EVENT_signal_lock_flag_simple(manager.mctx->esignal, &manager.status, DNSSD_MANAGER_FLAG_LOCK_MSOCKETS, DNSSD_MANAGER_FLAG_LOCK_MSOCKETS)==0) {

        logoutput_debug("%s: unable to lock list with mdns sockets ... cannot continue", __FUNCTION__);
        return;

    }

    /* create a list of mdns capable interfaces */

    result=NETWORK_list_network_addrs(dnssd_add_mdns_socket_cb, NULL);

    EVENT_signal_unlock_flag(manager.mctx->esignal, &manager.status, DNSSD_MANAGER_FLAG_LOCK_MSOCKETS);

    if (result==-1) {

        logoutput_debug("%s: error opening sockets ... cannot continue", __FUNCTION__);
        return;

    }

    logoutput_debug("%s: %u sockets found", __FUNCTION__, (unsigned int) result);

    /* for every addr/socket found add to eventloop and send dns sd */

    list=LIST_header_get_first(&manager.msockets);

    while (list) {
        struct mdns_socket_s *msock=(struct mdns_socket_s *)((char *)list - offsetof(struct mdns_socket_s, list));

        if (msock->status & MDNS_SOCKET_STATUS_FLAG_ERROR) goto nextmsock;

        if ((msock->status & MDNS_SOCKET_STATUS_FLAG_OPEN)==0) {

            if (MDNS_socket_open(msock)<1) {

                logoutput_debug("%s: unable to open mdns socket", __FUNCTION__);
                EVENT_signal_set_flag(msock->mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_ERROR);
                goto nextmsock;

            }

            EVENT_signal_set_flag(msock->mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_OPEN);

        }

        if ((msock->status & MDNS_SOCKET_STATUS_FLAG_EVENTLOOP)==0) {

            /* add to eventloop */

            if (MDNS_socket_add_to_eventloop(msock, NULL)<=0) {

                logoutput_debug("%s: unable to add mdns socket to eventloop", __FUNCTION__);
                EVENT_signal_set_flag(msock->mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_ERROR);
                goto nextmsock;

            }

            EVENT_signal_set_flag(msock->mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_EVENTLOOP);

        }

        if ((msock->status & MDNS_SOCKET_STATUS_FLAG_DISCOVERY_SEND)==0) {

            /* send dns sd on socket */

            if (MDNS_discovery_send(msock)<0) logoutput_debug("%s: error sending dnssd", __FUNCTION__);
            EVENT_signal_set_flag(msock->mctx->esignal, &msock->status, MDNS_SOCKET_STATUS_FLAG_DISCOVERY_SEND);

        }

        nextmsock:
        list=LIST_element_get_next(list);

    }

}

static void dnssd_manager_start()
{
    dnssd_manager_get_all_mdns_sockets(0);
}

static void dnssd_manager_refresh()
{
    dnssd_manager_get_all_mdns_sockets(1);
}

static void dnssd_manager_finish()
{
    struct list_element_s *list=LIST_header_remove_first(&manager.msockets);

    while (list) {
        struct mdns_socket_s *msock=(struct mdns_socket_s *)((char *)list - offsetof(struct mdns_socket_s, list));

        MDNS_socket_remove(msock);
        list=LIST_header_remove_first(&manager.msockets);

    }

}

void DNSSD_init(struct mdns_socket_ctx_s *mctx)
{

    EVENT_signal_lock_flag(dnssd_module.esignal, &dnssd_module.status, PROGRAM_MODULE_FLAG_LOCK);

    if (dnssd_module.status & PROGRAM_MODULE_FLAG_INIT_DONE) {

        EVENT_signal_unlock_flag(dnssd_module.esignal, &dnssd_module.status, PROGRAM_MODULE_FLAG_LOCK);
        return;

    }

    LIST_header_init(&manager.msockets, 0, NULL);

    manager.mctx=(mctx) ? mctx : MDNS_socket_get_default_ctx();

    manager.start=dnssd_manager_start;
    manager.refresh=dnssd_manager_refresh;
    manager.finish=dnssd_manager_finish;

    if (manager.mctx->esignal==NULL) manager.mctx->esignal=EVENT_signal_get_default();

    dnssd_module.status |= PROGRAM_MODULE_FLAG_INIT_DONE;
    EVENT_signal_unlock_flag(dnssd_module.esignal, &dnssd_module.status, PROGRAM_MODULE_FLAG_LOCK);

}

void DNSSD_start()
{
    (* manager.start)();
}

void DNSSD_refresh()
{
    (* manager.refresh)();
}

void DNSSD_finish()
{
    (* manager.finish)();
}

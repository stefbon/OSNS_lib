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

#include "dnssd.h"
#include "mdns-socket.h"
#include "mdns-interface.h"

static void cb_errorclose_mdns_socket_default(struct mdns_socket_ctx_s *mctx)
{}

static unsigned char cb_select_mdns_socket_default(struct mdns_socket_ctx_s *mctx, char *name, unsigned int length)
{
    return 1; /* by default select everything */
}

static void cb_host_mdns_socket_default(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct dstr_s *service, struct dstr_s *semantics, unsigned int ttl)
{
    /* log */

    logoutput_debug("%s: found host %.*s domain %.*s service %.*s semantics %.*s ttl %u", __FUNCTION__, hostname->length, hostname->str, domain->length, domain->str, service->length, service->str, semantics->length, semantics->str, ttl);
}

static void cb_service_mdns_socket_default(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct dstr_s *service, struct dstr_s *semantics, unsigned int port)
{
    /* log */

    logoutput_debug("%s: found host %.*s domain %.*s service %.*s semantics %.*s port %u", __FUNCTION__, hostname->length, hostname->str, domain->length, domain->str, service->length, service->str, semantics->length, semantics->str, port);
}

static void cb_addr_mdns_socket_default(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct io_addr_object_s *peer)
{
    /* log */

    logoutput_debug("%s: found host %.*s domain %.*s", __FUNCTION__, hostname->length, hostname->str, domain->length, domain->str);
}

static struct mdns_socket_ctx_s default_ctx = {
    .esignal                                    = NULL,
    .cb_close                                   = cb_errorclose_mdns_socket_default,
    .cb_error                                   = cb_errorclose_mdns_socket_default,
    .cb_select                                  = cb_select_mdns_socket_default,
    .cb_host                                    = cb_host_mdns_socket_default,
    .cb_service                                 = cb_service_mdns_socket_default,
    .cb_addr                                    = cb_addr_mdns_socket_default,
};

struct mdns_socket_ctx_s *MDNS_socket_get_default_ctx()
{
    return &default_ctx;
}

void MDNS_socket_set_default_ctx(struct mdns_socket_ctx_s *mctx)
{
    if (mctx) memcpy(mctx, &default_ctx, sizeof(struct mdns_socket_ctx_s));
}

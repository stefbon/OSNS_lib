/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-list.h"

#include "dnssd.h"

#ifdef __linux__

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
static const unsigned char localhost_mapped[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};

unsigned char MDNS_skip_network_interface_addr(struct io_connection_object_s *ico, unsigned int flags)
{

    if (ico==NULL) return 1;                                            /* not defined */
    if (IO_connection_object_is(ico, "network")==0) return 1;           /* skip non network scokets */

    /* is the interface up ? */

    if ((flags & IFF_UP)==0) return 1;

    /* interface has to support multicast */

    if ((flags & IFF_MULTICAST)==0) return 1;

    /* it's not a loopback device */

    if (flags & IFF_LOOPBACK) return 1;

    /* it's not a point to point */

    if (flags & IFF_POINTOPOINT) return 1;

    if (IO_connection_object_get_family(ico)==AF_INET) {
        struct sockaddr_in *saddr = (struct sockaddr_in *) ico->addr.addr;

        /* loopback ipv4 */

        if (saddr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) return 1;

    } else if (IO_connection_object_get_family(ico)==AF_INET6) {
	struct sockaddr_in6* saddr = (struct sockaddr_in6 *) ico->addr.addr;

	if (saddr->sin6_scope_id) return 1;                             /* Ignore link-local addresses */

        /* loopback ipv6 */

	if ((memcmp(saddr->sin6_addr.s6_addr, localhost, 16)==0) || (memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)==0)) return 1;

    } else {

        return 1;

    }

    /* do not skip the rest ...
        add more tests here .... */

    return 0;

}

#else

unsigned char MDNS_skip_network_interface_addr(struct io_connection_object_s *ico, unsigned int flags)
{
    return 1;
}

#endif

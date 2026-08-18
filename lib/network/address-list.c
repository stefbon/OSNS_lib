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
#include "libosns-list.h"

#ifdef __linux__

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "address-list.h"

/* this function will gather all network sockets */

int NETWORK_list_network_addrs(unsigned char (* cb)(struct io_connection_object_s *ico, unsigned int flags, void *ptr), void *ptr)
{
    int result = 0;
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;

    if (getifaddrs(&ifaddr) == -1) {

        logoutput_debug("%s: Unable to get interface addresses errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));
        return -1;

    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        struct io_connection_object_s ico;

        IO_connection_object_init(&ico);
        if (IO_connection_object_set(&ico, ifa->ifa_addr)) result += (* cb)(&ico, ifa->ifa_flags, ptr);

    }

    freeifaddrs(ifaddr);
    return result;
}

#else

int NETWORK_list_network_addrs(unsigned char (* cb)(struct io_object_addr_s *oaddr, unsigned int flags, void *ptr), void *ptr)
{
    return -1;
}

#endif

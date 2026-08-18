/*
  2017 Stef Bon <stefbon@gmail.com>

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

#include "address.h"

unsigned char NETWORK_ip_address_check_family(char *address, const char *what)
{

    if (strcmp(what, "ipv4")==0) {
	struct in_addr tmp;

	return inet_pton(AF_INET, address, (void *)&tmp);

    } else if (strcmp(what, "ipv6")==0) {
	struct in6_addr tmp;

	return inet_pton(AF_INET6, address, (void *)&tmp);

    }

    return 0;

}

int NETWORK_ip_address_compare(struct ip_address_s *a, struct ip_address_s *b)
{

    if (a->family==b->family) {

        if (a->family==IP_ADDRESS_FAMILY_IPv4) {

            return memcmp(a->addr.v4, b->addr.v4, INET_ADDRSTRLEN);

        } else if (a->family==IP_ADDRESS_FAMILY_IPv6) {

            return memcmp(a->addr.v6, b->addr.v6, INET6_ADDRSTRLEN);

        }

    }

    return -1;
}

int NETWORK_ip_address_compare_with(struct ip_address_s *a, const unsigned char type, void *ptr)
{
    int result=-1;

    switch (type) {

        case 'i' :
        {
            struct ip_address_s *b=(struct ip_address_s *) ptr;

            result=NETWORK_ip_address_compare(a, b);
            break;

        }

        case 'c' :
        {
            char *tmp=(char *) ptr;
            struct ip_address_s b;

            if (NETWORK_ip_address_check_family(tmp, "ipv4")) {

                b.family=IP_ADDRESS_FAMILY_IPv4;
                memcpy(b.addr.v4, tmp, strlen(tmp));

            } else if (NETWORK_ip_address_check_family(tmp, "ipv6")) {

                b.family=IP_ADDRESS_FAMILY_IPv6;
                memcpy(b.addr.v6, tmp, strlen(tmp));

            }

            result=NETWORK_ip_address_compare(a, &b);
            break;
        }

        default :

            logoutput_warning("%s: type %u not supported", __FUNCTION__, type);

    }

    return result;
}

unsigned char NETWORK_ip_address_valid(struct ip_address_s *ip)
{
    return (ip && ((ip->ip==ip->addr.v4) || (ip->ip==ip->addr.v6))) ? 1 : 0;
}

void NETWORK_ip_address_set_family(struct ip_address_s *ip, unsigned int family)
{

    ip->family=0;
    ip->ip=NULL;
    ip->length=0;

    if (family==IP_ADDRESS_FAMILY_IPv4) {

        ip->family=IP_ADDRESS_FAMILY_IPv4;
        ip->ip=ip->addr.v4;
        ip->length=INET_ADDRSTRLEN;

    } else if (family==IP_ADDRESS_FAMILY_IPv6) {

        ip->family=IP_ADDRESS_FAMILY_IPv6;
        ip->ip=ip->addr.v6;
        ip->length=INET6_ADDRSTRLEN;

    }

}

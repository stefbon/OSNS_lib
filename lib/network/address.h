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

*/

#ifndef LIB_NETWORK_ADDRESS_H
#define LIB_NETWORK_ADDRESS_H

#include "libosns-datatypes.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define IP_ADDRESS_FAMILY_IPv4			AF_INET
#define IP_ADDRESS_FAMILY_IPv6			AF_INET6

union ip_address_u {
    char					v4[INET_ADDRSTRLEN + 1];
    char					v6[INET6_ADDRSTRLEN + 1];
};

struct ip_address_s {
    unsigned int				family;
    union ip_address_u                          addr;
    char                                        *ip;
    unsigned int                                length;
};

/* prototypes */

unsigned char NETWORK_ip_address_check_family(char *address, const char *what);

int NETWORK_ip_address_compare(struct ip_address_s *a, struct ip_address_s *b);
int NETWORK_ip_address_compare_with(struct ip_address_s *a, const unsigned char type, void *ptr);

unsigned char NETWORK_ip_address_valid(struct ip_address_s *ip);
void NETWORK_ip_address_set_family(struct ip_address_s *ip, unsigned int family);

#endif

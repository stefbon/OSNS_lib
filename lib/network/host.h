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

#ifndef LIB_NETWORK_HOST_H
#define LIB_NETWORK_HOST_H

#include "libosns-datatypes.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NETWORK_HOSTNAME_FQDN_MAX_LENGTH		253
#define NETWORK_HOSTNAME_MAX_LENGTH		        63

#define NETWORK_HOSTNAME_FLAG_ALLOC                     1
#define NETWORK_HOSTNAME_FLAG_IP			2
#define NETWORK_HOSTNAME_FLAG_CANONNAME		        4
#define NETWORK_HOSTNAME_FLAG_DNSNAME		        8

struct network_hostname_s {
    unsigned int				flags;
    struct dstr_s                               name;
    char					buffer[];
};

/* prototypes */

void NETWORK_network_hostname_init(struct network_hostname_s *a, unsigned int size);

#endif

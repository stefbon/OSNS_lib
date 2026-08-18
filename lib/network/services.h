/*
  2017 Stef Bon <stefbon@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANPABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software

*/

#ifndef _LIB_NETWORK_SERVICES_H
#define _LIB_NETWORK_SERVICES_H

struct network_port_s {
    unsigned int				nr;
};

#define NETWKRK_SEMANTICS_TCP                   1
#define NETWORK_SEMANTICS_UDP                   2

struct network_service_s {
    struct network_port_s			port;
    unsigned int                                semantics;
    struct dstr_s                               name;
};

/* prototypes */

int NETWORK_service_get_port_by_name(struct dstr_s *name);

void NETWORK_service_init(struct network_service_s *netsrv, struct dstr_s *name, unsigned int semantics, unsigned int portnr);
struct network_service_s *NETWORK_service_create(struct dstr_s *name, unsigned int semantics, unsigned int portnr, unsigned char allocate);

#endif

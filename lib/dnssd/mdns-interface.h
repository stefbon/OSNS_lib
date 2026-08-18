/*
  2010, 2011, 2012, 2013, 2014, 2015 Stef Bon <stefbon@gmail.com>

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

#ifndef LIB_DNSSD_MDNS_INTERFACE_H
#define LIB_DNSSD_MDNS_INTERFACE_H

/* prototypes */

int MDNS_discovery_send(struct mdns_socket_s *msock);
size_t MDNS_recv_query(struct mdns_socket_s *msock, char *buffer, unsigned int size);
int MDNS_socket_open(struct mdns_socket_s *msock);

#endif

/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_DNSSD_MDNS_INTERFACE_H
#define LIB_DNSSD_MDNS_INTERFACE_H

/* prototypes */

int MDNS_discovery_send(struct mdns_socket_s *msock);
size_t MDNS_recv_query(struct mdns_socket_s *msock, char *buffer, unsigned int size);
int MDNS_socket_open(struct mdns_socket_s *msock);

#endif

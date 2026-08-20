/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_DNSSD_MDNS_SOCKET_H
#define LIB_DNSSD_MDNS_SOCKET_H

/* prototypes */

struct mdns_socket_s *MDNS_socket_add(struct list_header_s *header, struct sockaddr *saddr, struct mdns_socket_ctx_s *mctx);
void MDNS_socket_remove(struct mdns_socket_s *msock);
int MDNS_socket_add_to_eventloop(struct mdns_socket_s *msock, struct beventloop_s *loop);

#endif

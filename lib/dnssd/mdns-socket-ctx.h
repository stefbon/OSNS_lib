/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_DNSSD_MDNS_SOCKET_CTX_H
#define LIB_DNSSD_MDNS_SOCKET_CTX_H

/* prototypes */

struct mdns_socket_ctx_s *MDNS_socket_get_default_ctx();
void MDNS_socket_set_default_ctx(struct mdns_socket_ctx_s *mctx);

#endif

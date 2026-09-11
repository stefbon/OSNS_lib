/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_SYSTEM_DNSSD_H
#define OSNS_SYSTEM_DNSSD_H

#include "libosns-eventloop.h"

#define MDNS_SOCKET_ACTION_ADD                          0
#define MDNS_SOCKET_ACTION_REMOVE                       1

struct mdns_socket_ctx_s {
    struct event_shared_signal_s                        *esignal;
    void                                                (* cb_close)(struct mdns_socket_ctx_s *mctx);
    void                                                (* cb_error)(struct mdns_socket_ctx_s *mctx);
    unsigned char                                       (* cb_select)(struct mdns_socket_ctx_s *mctx, char *name, unsigned int length);
    void                                                (* cb_host)(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct dstr_s *service, struct dstr_s *semantics, unsigned int ttl);
    void                                                (* cb_service)(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct dstr_s *service, struct dstr_s *semantics, unsigned int port);
    void                                                (* cb_addr)(struct mdns_socket_ctx_s *mctx, unsigned char action, struct io_addr_object_s *from, struct dstr_s *hostname, struct dstr_s *domain, struct io_addr_object_s *peer);
    void						*ptr;
};

#define MDNS_SOCKET_STATUS_FLAG_INIT                    1
#define MDNS_SOCKET_STATUS_FLAG_ERROR                   2
#define MDNS_SOCKET_STATUS_FLAG_OPEN                    4
#define MDNS_SOCKET_STATUS_FLAG_EVENTLOOP               8
#define MDNS_SOCKET_STATUS_FLAG_DISCOVERY_SEND          16
#define MDNS_SOCKET_STATUS_FLAG_READ                    32


struct mdns_socket_s {
    struct list_element_s                               list;
    unsigned int                                        status;
    struct io_buffer_s                                  iob;
    struct io_object_s                                  object;
    struct bevent_ctx_s                                 bctx;
    struct mdns_socket_ctx_s                            *mctx;
};

/* prototypes */

void DNSSD_init(struct mdns_socket_ctx_s *mctx);
void DNSSD_start();
void DNSSD_refresh();
void DNSSD_finish();

#endif

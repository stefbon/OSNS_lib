/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_EVENTLOOP_CTX_H
#define LIB_EVENTLOOP_CTX_H

#include "bevent.h"

/* Prototypes */

void BEVENT_ctx_init(struct bevent_ctx_s *bctx, struct io_object_backend_s *obck, unsigned char isfallback);

void BEVENT_ctx_init_fallback();
struct bevent_ctx_s *BEVENT_ctx_get_fallback();
void BEVENT_ctx_replace_with_fallback(struct bevent_ctx_s *bctx, unsigned char move2inactive);

unsigned char BEVENT_ctx_attach_to_eventloop(struct beventloop_s *eloop, struct bevent_ctx_s *bctx);
void BEVENT_ctx_detach(struct bevent_ctx_s *bctx);

void BEVENT_ctx_set_cb(struct bevent_ctx_s *bctx, unsigned int flag, bctx_cb_t *cb);
void BEVENT_ctx_set_signal(struct bevent_ctx_s *bctx, bctx_cb_t *cb);

#endif

/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef _LIB_EVENTLOOP_BACKEND_H
#define _LIB_EVENTLOOP_BACKEND_H

#include "loop.h"
#include "bevent.h"

/* Prototypes */

void BACKEND_fill_system_event_values();

uint32_t BACKEND_get_system_event_value(unsigned int index);
uint32_t BACKEND_get_system_event_value_raw(unsigned int index);
uint32_t BACKEND_get_system_mode_value(unsigned int index);
uint32_t BACKEND_get_system_mode_value_raw(unsigned int index);

int BEVENT_ctx_add(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how);
int BEVENT_ctx_mod(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how);
int BEVENT_ctx_del(struct beventloop_s *eloop, struct bevent_ctx_s *bctx);

void BEVENTLOOP_backend_init(struct beventloop_s *eloop);
int BEVENTLOOP_backend_start(struct beventloop_s *eloop);
void BEVENTLOOP_backend_close(struct beventloop_s *eloop);

#endif

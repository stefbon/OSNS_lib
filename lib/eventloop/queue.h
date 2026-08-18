/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef _LIB_EVENTLOOP_QUEUE_H
#define _LIB_EVENTLOOP_QUEUE_H

#include "bevent.h"

/* Prototypes */

void BEVENT_release_thread(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg);

void BEVENT_process_events_thread(void *ptr);
unsigned char BEVENT_bevent_queue_events(struct beventloop_s *loop, struct bevent_s *bevent);

#endif

/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef _LIB_EVENTLOOP_LOOP_H
#define _LIB_EVENTLOOP_LOOP_H

#define BEVENTLOOP_MAX_SUBSYSTEMS		12

#include "libosns-list.h"
#include "libosns-time.h"
#include "libosns-io.h"
#include "bevent.h"

struct beventloop_s;

/* eventloop */

#define BEVENTLOOP_FLAG_ALLOC			1 << 0
#define BEVENTLOOP_FLAG_MAIN			1 << 1
#define BEVENTLOOP_FLAG_INIT			1 << 2
#define BEVENTLOOP_FLAG_EPOLL			1 << 3
#define BEVENTLOOP_FLAG_START			1 << 4
#define BEVENTLOOP_FLAG_STOP			1 << 5

#define BEVENTLOOP_LOCK_BEVENTS			1 << 0
#define BEVENTLOOP_LOCK_INACTIVE		1 << 1
#define BEVENTLOOP_LOCK_BCTX			1 << 2

#define BEVENTLOOP_TASK_STOP			1
#define BEVENTLOOP_TASK_CLEAN_INACTIVE		2

struct beventloop_task_s {
    struct beventloop_s				*eloop;
    struct io_object_s				object;
    struct bevent_ctx_s				bctx;
};

#ifdef __linux__

struct beventloop_epoll_s {
    int						fd;
};

#endif

struct beventloop_s {
    unsigned int				flags;
    unsigned int				lock;

    /* shared signal */

    struct event_shared_signal_s	        *esignal;

    /* list with bevents (=watches) */

    struct list_header_s			bevents;
    struct list_header_s			inactive;

    /* backend code of the system eventloop used (epoll, glib, ...)*/

    union beventloop_backend_u {
#ifdef __linux__
	struct beventloop_epoll_s 		epoll;
#endif
	void					*ptr;
    } backend;
};

/* Prototypes */

void BEVENTLOOP_init();
int BEVENTLOOP_start();
void BEVENTLOOP_stop();
void BEVENTLOOP_clear();

unsigned char BEVENTLOOP_wait_to_start(struct timespec_s *timeout);
void BEVENTLOOP_clear_list(struct beventloop_s *eloop, unsigned char inactive);
struct beventloop_s *BEVENTLOOP_get_default_loop();
struct event_shared_signal_s *BEVENTLOOP_get_event_shared_signal(struct beventloop_s *eloop);

void BEVENTLOOP_send_task(struct beventloop_s *eloop, unsigned int task);

#endif

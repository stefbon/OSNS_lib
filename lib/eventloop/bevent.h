/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef _LIB_EVENTLOOP_BEVENT_H
#define _LIB_EVENTLOOP_BEVENT_H

#include "loop.h"

#define BEVENT_EVENT_INDEX_READABLE			0
#define BEVENT_EVENT_INDEX_PRI		        	1
#define BEVENT_EVENT_INDEX_WRITEABLE			2
#define BEVENT_EVENT_INDEX_ERROR			3
#define BEVENT_EVENT_INDEX_CLOSE			4
#define BEVENT_EVENT_INDEX_COUNT                	5

#define BEVENT_MODE_INDEX_EDGE                  	0
#define BEVENT_MODE_INDEX_ONESHOT               	1
#define BEVENT_MODE_INDEX_COUNT                 	2

#define BEVENT_EVENT_BIT_ERROR				(1 << BEVENT_EVENT_INDEX_ERROR)
#define BEVENT_EVENT_BIT_CLOSE				(1 << BEVENT_EVENT_INDEX_CLOSE)
#define BEVENT_EVENT_BIT_READABLE			(1 << BEVENT_EVENT_INDEX_READABLE)
#define BEVENT_EVENT_BIT_WRITEABLE			(1 << BEVENT_EVENT_INDEX_WRITEABLE)
#define BEVENT_EVENT_BIT_PRI				(1 << BEVENT_EVENT_INDEX_PRI)

#define BEVENT_EVENT_ALL_BITS                  		(BEVENT_EVENT_BIT_ERROR | BEVENT_EVENT_BIT_CLOSE | BEVENT_EVENT_BIT_READABLE | BEVENT_EVENT_BIT_WRITEABLE | BEVENT_EVENT_BIT_PRI)

#define BEVENT_MODE_BIT_EDGE                    	(1 << BEVENT_MODE_INDEX_EDGE)
#define BEVENT_MODE_BIT_ONESHOT                 	(1 << BEVENT_MODE_INDEX_ONESHOT)

#define BEVENT_MODE_ALL_BITS                  		(BEVENT_MODE_BIT_ONESHOT | BEVENT_MODE_BIT_EDGE)

#define BEVENT_CTX_MODE_SET				1
#define BEVENT_CTX_MODE_DISABLE				2
#define BEVENT_CTX_MODE_ENABLE				3

#define BEVENT_STATUS_FLAG_PENDING              	(1 << 0)
#define BEVENT_STATUS_FLAG_CLOSED               	(1 << 1)
#define BEVENT_STATUS_FLAG_DEL          		(1 << 2)
#define BEVENT_STATUS_FLAG_NON_WRITEABLE        	(1 << 3)
#define BEVENT_STATUS_FLAG_INACTIVE			(1 << 4)

#define BEVENT_STATUS_LOCK_PENDING         		(1 << 0)
#define BEVENT_STATUS_LOCK_CLOSED            		(1 << 1)
#define BEVENT_STATUS_LOCK_THREAD            		(1 << 2)
#define BEVENT_STATUS_LOCK_BCTX            		(1 << 3)
#define BEVENT_STATUS_LOCK_REVENTS         		(1 << 4)
#define BEVENT_STATUS_LOCK_EVENTS          		(1 << 5)
#define BEVENT_STATUS_LOCK_NON_WRITEABLE		(1 << 6)

struct bevent_s;

struct bevent_argument_s {
    unsigned int                                	index;
    struct beventloop_s					*loop;
#ifdef __linux__
    unsigned int					errcode;
#endif
};

#ifdef __linux__
#define BEVENT_ARGUMENT_INIT                    	{0, NULL, 0}
#else
#define BEVENT_ARGUMENT_INIT                    	{0, NULL}
#endif

struct bevent_events_s {
    unsigned int                               		events;
    unsigned int					mode;
    struct list_header_s                        	header;
};

struct bevent_revents_s {
    unsigned int					events;
    pthread_t						threadid;
    struct list_header_s                        	header;
};

#define BAE_STATUS_FLAG_PENDING        			1
#define BAE_STATUS_FLAG_RLIST				2

#define BAE_STATUS_LOCK_PENDING        			1
#define BAE_STATUS_LOCK_RLIST				2

struct bevent_ctx_s;

struct bevent_array_element_s {
    struct list_element_s				elist; /* list of events to watch */
    struct list_element_s				rlist; /* list of events reported */
    unsigned int                                	index;
    unsigned int                                	status;
    unsigned int					lock;
    unsigned char                               	(* action)(struct bevent_array_element_s *bae, struct bevent_s *bevent);
};

typedef void (bctx_cb_t)(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg);

#define BEVENT_CTX_FLAG_FALLBACK         		1

struct bevent_ctx_s {
    unsigned int                                	flags;
    unsigned int                                	status;
    unsigned int					refcount;
    struct bevent_s                            		*bevent;
    struct io_object_backend_s                  	*io_backend;
    bctx_cb_t						*cb[BEVENT_EVENT_INDEX_COUNT];
    void						(* signal)(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg);
};

struct bevent_s {
    unsigned int					flags;
    unsigned int                                	status;
    unsigned int					lock;
    struct event_shared_signal_s                	*esignal;
    struct beventloop_s					*eloop;
    struct list_element_s				list;
    struct timespec_s					unblocked;
    struct bevent_ctx_s                         	*bctx;
    struct bevent_events_s                      	events; /* list with events to watch */
    struct bevent_revents_s                      	revents; /* list with reported events */
    struct bevent_array_element_s               	bae[BEVENT_EVENT_INDEX_COUNT];
};

/* Prototypes */

struct bevent_s *BEVENT_create(struct beventloop_s *eloop);
void BEVENT_move_to_inactive(struct bevent_s *bevent);
void BEVENT_process_events(struct bevent_s *bevent);

#endif

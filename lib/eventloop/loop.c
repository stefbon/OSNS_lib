/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-event.h"
#include "libosns-threads.h"

#include "loop.h"
#include "bevent.h"
#include "ctx.h"
#include "backend.h"

static struct event_shared_signal_s *esignal_eventloop=NULL;
static unsigned char initdone=0;
static unsigned int refcount=0;

static struct beventloop_s beventloop_main;

static void BEVENTLOOP_module_init()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(esignal)==0) {

	refcount++;

	if (initdone==0) {

	    initdone=1;
	    esignal_eventloop=EVENT_signal_create_custom();
	    BEVENT_ctx_init_fallback();
	    BACKEND_fill_system_event_values();

	}

	EVENT_signal_unlock(esignal);

    }

}

static void BEVENTLOOP_module_clear()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(esignal)==0) {

	if (refcount) refcount--;

	if (refcount==0) {

	    initdone=0;
	    if (esignal_eventloop) EVENT_signal_clear(&esignal_eventloop);

	}

	EVENT_signal_unlock(esignal);

    }

}

void BEVENTLOOP_init()
{
    struct beventloop_s *eloop=&beventloop_main;

    BEVENTLOOP_module_init();

    memset(eloop, 0, sizeof(struct beventloop_s));

    eloop->flags = BEVENTLOOP_FLAG_MAIN;

    if (esignal_eventloop) {

	eloop->esignal=esignal_eventloop;

    } else {

	eloop->esignal=EVENT_signal_get_default();

    }

    LIST_header_init(&eloop->bevents, 0);
    LIST_header_init(&eloop->inactive, 0);
    BEVENTLOOP_backend_init(eloop);
    eloop->flags |= BEVENTLOOP_FLAG_INIT;

}

int BEVENTLOOP_start()
{
    return BEVENTLOOP_backend_start(&beventloop_main);
}

void BEVENTLOOP_stop()
{
    struct beventloop_s *eloop=&beventloop_main;

    if (EVENT_signal_set_flag(eloop->esignal, &eloop->flags, BEVENTLOOP_FLAG_STOP)) {

	logoutput_debug("%s: set stop flag", __FUNCTION__);

#ifdef __linux__

	if (eloop->backend.epoll.fd>=0) {

	    close(eloop->backend.epoll.fd);
	    eloop->backend.epoll.fd=-1;

	}

#endif

    }

}

void BEVENTLOOP_clear()
{
    struct beventloop_s *eloop=&beventloop_main;

    BEVENTLOOP_clear_list(eloop, 0);
    BEVENTLOOP_clear_list(eloop, 1);

}

unsigned char BEVENTLOOP_wait_to_start(struct timespec_s *timeout)
{
    struct beventloop_s *eloop=&beventloop_main;
    struct event_shared_signal_s *esignal=eloop->esignal;
    struct timespec_s expire=TIME_INIT;
    unsigned char success=0;

    TIME_now(&expire);
    TIME_plus(&expire, timeout);

    if (EVENT_signal_wait_flag_set(esignal, &eloop->flags, (BEVENTLOOP_FLAG_START | BEVENTLOOP_FLAG_STOP), &expire)==1) success=1;
    return success;
}

void BEVENTLOOP_clear_list(struct beventloop_s *eloop, unsigned char inactive)
{
    struct list_header_s *header=(inactive ? &eloop->inactive : &eloop->bevents);
    struct list_element_s *list=LIST_header_remove_first(header);

    while (list) {
	struct bevent_s *bevent=(struct bevent_s *)((char *) list - offsetof(struct bevent_s, list));
	struct bevent_ctx_s *bctx=bevent->bctx;

	if (bctx) BEVENT_ctx_replace_with_fallback(bctx, 0);
	free(bevent);

	list=LIST_header_remove_first(&eloop->bevents);

    }

}

struct beventloop_s *BEVENTLOOP_get_default_loop()
{
    return &beventloop_main;
}

struct event_shared_signal_s *BEVENTLOOP_get_event_shared_signal(struct beventloop_s *eloop)
{
    return eloop->esignal;
}

void BEVENTLOOP_send_task(struct beventloop_s *eloop, unsigned int task)
{}

/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-io.h"
#include "libosns-event.h"
#include "libosns-threads.h"

#include "loop.h"
#include "bevent.h"
#include "ctx.h"
#include "bae.h"
#include "queue.h"
#include "backend.h"

static void beventloop_lock_header_for_change(struct event_shared_signal_s *esignal, struct list_header_s *header, unsigned char remove, struct list_element_s *list, unsigned int *p_lock, unsigned int lockflag)
{

    EVENT_signal_lock_flag(esignal, p_lock, lockflag);

    if (remove) {

	LIST_element_remove(list);

    } else {

	LIST_header_add_last(header, list);

    }

    EVENT_signal_unlock_flag(esignal, p_lock, lockflag);

}

struct bevent_s *BEVENT_create(struct beventloop_s *eloop)
{
    struct bevent_s *bevent=NULL;

    logoutput_debug("%s", __FUNCTION__);

    bevent=malloc(sizeof(struct bevent_s));

    if (bevent==NULL) {

        logoutput_debug("%s: unable to allocate bevent", __FUNCTION__);
        return NULL;

    }

    memset(bevent, 0, sizeof(struct bevent_s));

    bevent->status=0;
    bevent->lock=0;

    bevent->esignal=eloop->esignal;
    bevent->eloop=eloop;
    LIST_element_init(&bevent->list, NULL);
    TIME_set(&bevent->unblocked, 0, 0);
    bevent->bctx=NULL;

    /* events to watch for */

    bevent->events.events=0;
    bevent->events.mode=0;
    LIST_header_init(&bevent->events.header, 0);

    /* reported events */

    bevent->revents.events=0;
    bevent->revents.threadid=0;
    LIST_header_init(&bevent->revents.header, 0);

    /* all possible events */

    for (unsigned int i=0; i<BEVENT_EVENT_INDEX_COUNT; i++) BAE_init(&bevent->bae[i], i);

    /* add to list */

    beventloop_lock_header_for_change(eloop->esignal, &eloop->bevents, 0, &bevent->list, &eloop->lock, BEVENTLOOP_LOCK_BEVENTS);

    logoutput_debug("%s: out", __FUNCTION__);
    return bevent;

}

void BEVENT_move_to_inactive(struct bevent_s *bevent)
{

    if (bevent==NULL) {

	logoutput_debug("%s: bevent not defined ... cannot continue", __FUNCTION__);
	return;

    }

    if (EVENT_signal_set_flag(bevent->esignal, &bevent->status, BEVENT_STATUS_FLAG_INACTIVE)) {
	struct beventloop_s *eloop=bevent->eloop;

	beventloop_lock_header_for_change(eloop->esignal, &eloop->bevents, 1, &bevent->list, &eloop->lock, BEVENTLOOP_LOCK_BEVENTS);
	beventloop_lock_header_for_change(eloop->esignal, &eloop->inactive, 0, &bevent->list, &eloop->lock, BEVENTLOOP_LOCK_INACTIVE);

    }

}

void BEVENT_process_events(struct bevent_s *bevent)
{
    LOCAL_threads_put_job(0, BEVENT_process_events_thread, (void *) bevent);
}

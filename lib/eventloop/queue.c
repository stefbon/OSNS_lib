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
#include "backend.h"

void BEVENT_release_thread(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct bevent_s *bevent=bctx->bevent;
    struct event_shared_signal_s *esignal=NULL;

    if (bevent==NULL) return;
    esignal=bevent->esignal;

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS)) {
	struct bevent_array_element_s *bae=&bevent->bae[arg->index];

	if ((bevent->revents.threadid==pthread_self()) && (bae->status & BAE_STATUS_FLAG_RLIST)) {

	    bae->status &= ~(BAE_STATUS_FLAG_RLIST | BAE_STATUS_FLAG_PENDING);
	    LIST_element_remove(&bae->rlist);

	}

    	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);

    }

    /* if there are events reported or events listed */

    if (bevent->revents.header.count || bevent->revents.events) BEVENT_process_events(bevent);

}

/* process the reported events list per bevent */

void BEVENT_process_events_thread(void *ptr)
{
    struct bevent_s *bevent=(struct bevent_s *) ptr;
    struct event_shared_signal_s *esignal=bevent->esignal;
    struct beventloop_s *eloop=bevent->eloop;
    struct bevent_array_element_s *bae=NULL;
    struct bevent_ctx_s *bctx=NULL;
    unsigned int index=0;
    unsigned int event=0;
    struct bevent_argument_s arg=BEVENT_ARGUMENT_INIT;

    arg.loop=eloop;

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS)) {
	struct list_element_s *list=NULL;

	/* test there is another thread active for this bevent */

	if (bevent->revents.threadid) {

	    if (bevent->revents.threadid!=pthread_self()) {

		EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);
		return;

	    }

	} else {

	    bevent->revents.threadid=pthread_self();

	}

	processreportedevents:

	list=LIST_header_get_first(&bevent->revents.header);

	/* get the bevent action event, leave it on the reported events list for new events */

	if (list==NULL) {

	    EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);
	    return;

	}

	bae=(struct bevent_array_element_s *)((char *)list - offsetof(struct bevent_array_element_s, rlist));
	bae->status |= BAE_STATUS_FLAG_PENDING;
	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);

    }

    index=bae->index;
    event=BACKEND_get_system_event_value(index);
    arg.index=index;

    processeventcb:

    /* run the corresponding cb in bevent ctx */

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX)) {

    	bctx=bevent->bctx;
	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX);

    }

    (* bctx->cb[index])(bctx, &arg);

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS)) {

	if (bae->status & BAE_STATUS_FLAG_RLIST) {

	    /* check this event is reported again */

    	    if (bevent->revents.events & event) {

        	logoutput_debug("%s: event %u reported ... again", __FUNCTION__, event);
        	bevent->revents.events &= ~event;
        	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);
        	goto processeventcb;

	    }

	    /* not reported: remove from reported events list */

	    bae->status &= ~(BAE_STATUS_FLAG_RLIST | BAE_STATUS_FLAG_PENDING);
	    LIST_element_remove(&bae->rlist);

	    if (bevent->revents.header.count) goto processreportedevents;

	}

    	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);

    }

}

unsigned char BEVENT_bevent_queue_events(struct beventloop_s *eloop, struct bevent_s *bevent)
{
    struct event_shared_signal_s *esignal=bevent->esignal;
    unsigned char count=0;

    /* test every event applies, ready when no events anymore */

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_EVENTS)) {
        struct list_element_s *list=LIST_header_get_first(&bevent->events.header);

        while (list && bevent->revents.events) {
            struct bevent_array_element_s *bae=(struct bevent_array_element_s *)((char *) list - offsetof(struct bevent_array_element_s, elist));

            count += (* bae->action)(bae, bevent);
            list=LIST_element_get_next(list);

        }

        EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_EVENTS);

    }

    /* return the number of events actually queued
        if this is zero (cause events are already present in the events list) then no need to start a thread */

    return count;
}

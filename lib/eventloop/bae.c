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
#include "backend.h"

/* action: a callback what to do with the event */

static unsigned char BAE_action_ignore(struct bevent_array_element_s *bae, struct bevent_s *bevent)
{
    return 0;
}

static unsigned char BAE_action_default(struct bevent_array_element_s *bae, struct bevent_s *bevent)
{
    uint32_t event=BACKEND_get_system_event_value_raw(bae->index); /* what events "belong" to this index/number? */
    struct event_shared_signal_s *esignal=NULL;

    /* is reported event to be handled here ? */

    if ((bevent->revents.events & event)==0) return 0;
    esignal=bevent->esignal;

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS)) {

	if (bae->status & BAE_STATUS_FLAG_RLIST) {
	    struct bevent_ctx_s *bctx;

	    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX)) {

    		bctx=bevent->bctx;
		EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX);

	    }

	    if (bctx->signal) {
		struct bevent_argument_s arg=BEVENT_ARGUMENT_INIT;

		/* this event is already on the reported events list */

		arg.loop=bevent->eloop;
		arg.index=bae->index;

		/* signal */

		(* bctx->signal)(bctx, &arg);

	    } else {

		bevent->revents.events |= event; /* set event */

	    }

	    EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);
	    return 0;

	}

	/* add to reported events list */

        bae->status |= BAE_STATUS_FLAG_RLIST;
	LIST_header_add_last(&bevent->revents.header, &bae->rlist);

	/* unset bits on reported events */
	bevent->revents.events &= ~event;

	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);

    }

    /* take action */
    return 1;
}

static unsigned char BAE_action_closeerror(struct bevent_array_element_s *bae, struct bevent_s *bevent)
{
    uint32_t event=BACKEND_get_system_event_value_raw(bae->index); /* what events "belong" to this index/number? */
    struct event_shared_signal_s *esignal=NULL;
    struct bevent_ctx_s *bctx;

    /* is reported event to be handled here ? */

    if ((bevent->revents.events & event)==0) return 0;
    esignal=bevent->esignal;

    /* use signal if available */

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX)) {

    	bctx=bevent->bctx;
	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_BCTX);

    }

    if (bctx->signal) {
	struct bevent_argument_s arg=BEVENT_ARGUMENT_INIT;

	/* this event is already on the reported events list */

	arg.loop=bevent->eloop;
	arg.index=bae->index;

	/* signal */

	(* bctx->signal)(bctx, &arg);
	return 0;

    }

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS)) {

	if (bae->index==BEVENT_EVENT_INDEX_CLOSE) bevent->status |= BEVENT_STATUS_FLAG_CLOSED;

	if (bevent->revents.header.count==0) {

	    /* there are no events queued on the revents list so queue one
		if there are they will process this event */

	    /* add to reported events list */

    	    bae->status |= BAE_STATUS_FLAG_RLIST;
	    LIST_header_add_last(&bevent->revents.header, &bae->rlist);

	    /* unset bits on reported events */
	    bevent->revents.events &= ~event;

	}

	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_REVENTS);

    }

    /* take action */
    return 1;
}

static unsigned char BAE_action_writeable(struct bevent_array_element_s *bae, struct bevent_s *bevent)
{
    uint32_t event=BACKEND_get_system_event_value_raw(bae->index); /* what events "belong" to this index/number? */
    struct event_shared_signal_s *esignal=NULL;

    if ((bevent->revents.events & event)==0) return 0;
    bevent->revents.events &= ~event; /* unset bits on reported events */
    esignal=bevent->esignal;

    /* unset non writeable flag and signal */

    if (EVENT_signal_lock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_NON_WRITEABLE)) {

	TIME_now(&bevent->unblocked);
	bevent->status &= ~BEVENT_STATUS_FLAG_NON_WRITEABLE;

	EVENT_signal_unlock_flag(esignal, &bevent->lock, BEVENT_STATUS_LOCK_NON_WRITEABLE);
    }

    /* do not start a thread */
    return 0;
}

void BAE_init(struct bevent_array_element_s *bae, unsigned int index)
{

    LIST_element_init(&bae->elist, NULL); /* list with events to listen to */
    LIST_element_init(&bae->rlist, NULL); /* list with reported events */
    bae->index=index;
    bae->status=0;
    bae->lock=0;

    if (index==BEVENT_EVENT_INDEX_WRITEABLE) {

        bae->action=BAE_action_writeable;

    } else if ((index==BEVENT_EVENT_INDEX_ERROR) || (index==BEVENT_EVENT_INDEX_CLOSE)) {

	bae->action=BAE_action_closeerror;

    } else {

        bae->action=BAE_action_default;

    }

}

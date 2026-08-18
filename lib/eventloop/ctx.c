/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-io.h"
#include "libosns-event.h"
#include "libosns-threads.h"

#include "loop.h"
#include "bevent.h"
#include "backend.h"

static struct bevent_ctx_s bctx_fallback;

static void bevent_ctx_cb_noop(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    logoutput_debug("%s: index %u", __FUNCTION__, arg->index);
}

void BEVENT_ctx_init(struct bevent_ctx_s *bctx, struct io_object_backend_s *io_backend, unsigned char isfallback)
{

    if (bctx==NULL) return;
    memset(bctx, 0, sizeof(struct bevent_ctx_s));

    bctx->flags=(isfallback ? BEVENT_CTX_FLAG_FALLBACK : 0);
    bctx->status=0;
    bctx->bevent=NULL;
    bctx->io_backend=io_backend;

    for (unsigned int i=0; i<BEVENT_EVENT_INDEX_COUNT; i++) bctx->cb[i]=bevent_ctx_cb_noop;
    bctx->signal=NULL;

}

void BEVENT_ctx_init_fallback()
{
    BEVENT_ctx_init(&bctx_fallback, NULL, 1);
}

void BEVENT_ctx_replace_with_fallback(struct bevent_ctx_s *bctx, unsigned char move2inactive)
{
    struct beventloop_s *eloop=NULL; 

    if (bctx->bevent==NULL) {

	logoutput_debug("%s: cannot continue ... bevent not defined", __FUNCTION__);

    } else if (bctx->bevent->eloop==NULL) {

	logoutput_debug("%s: cannot continue ... eloop not set", __FUNCTION__);

    }

    eloop=bctx->bevent->eloop;

    if (EVENT_signal_lock_flag(eloop->esignal, &eloop->lock, BEVENTLOOP_LOCK_BCTX)) {
	struct bevent_s *bevent=bctx->bevent;

	bevent->bctx=&bctx_fallback;
	bctx->bevent=NULL;

        EVENT_signal_unlock_flag(eloop->esignal, &eloop->lock, BEVENTLOOP_LOCK_BCTX);
	if (move2inactive) BEVENT_move_to_inactive(bevent);

    }

}

struct bevent_ctx_s *BEVENT_ctx_get_fallback()
{
    return &bctx_fallback;
}

static void bevent_ctx_set_cb_hlpr(struct bevent_s *bevent, struct bevent_ctx_s *bctx, unsigned int event_bit, unsigned int index, bctx_cb_t *cb, unsigned int *p_event_bits)
{
    struct bevent_array_element_s *bae=&bevent->bae[index];
    uint32_t event=BACKEND_get_system_event_value(index);

    if (event==0) return;

    if (cb) {

        bctx->cb[index]=cb;
        bevent->events.events |= event;
        if (LIST_element_is_listed(&bae->elist)==0) LIST_header_add_last(&bevent->events.header, &bae->elist);

    } else {

        /* disable */

        bctx->cb[index]=bevent_ctx_cb_noop;
        bevent->events.events &= ~event;
        LIST_element_remove(&bae->elist);

    }

    *p_event_bits &= ~event_bit;

}

void BEVENT_ctx_set_cb(struct bevent_ctx_s *bctx, unsigned int event_bits, bctx_cb_t *cb)
{
    struct bevent_s *bevent=(bctx) ? bctx->bevent : NULL;

    if (bevent==NULL) {

        logoutput_debug("%s: bevent not set ... cannot continue", __FUNCTION__);
        return;

    }

    if (event_bits & BEVENT_EVENT_BIT_CLOSE) bevent_ctx_set_cb_hlpr(bevent, bctx, BEVENT_EVENT_BIT_CLOSE, BEVENT_EVENT_INDEX_CLOSE, cb, &event_bits);
    if (event_bits & BEVENT_EVENT_BIT_ERROR) bevent_ctx_set_cb_hlpr(bevent, bctx, BEVENT_EVENT_BIT_ERROR, BEVENT_EVENT_INDEX_ERROR, cb, &event_bits);
    if (event_bits & BEVENT_EVENT_BIT_READABLE) bevent_ctx_set_cb_hlpr(bevent, bctx, BEVENT_EVENT_BIT_READABLE, BEVENT_EVENT_INDEX_READABLE, cb, &event_bits);
    if (event_bits & BEVENT_EVENT_BIT_PRI) bevent_ctx_set_cb_hlpr(bevent, bctx, BEVENT_EVENT_BIT_PRI, BEVENT_EVENT_INDEX_PRI, cb, &event_bits);

    if (event_bits) logoutput_debug("%s: bits %u not used ", __FUNCTION__, event_bits);

}

void BEVENT_ctx_set_signal(struct bevent_ctx_s *bctx, bctx_cb_t *cb)
{
    bctx->signal=cb;
}

unsigned char BEVENT_ctx_attach_to_eventloop(struct beventloop_s *eloop, struct bevent_ctx_s *bctx)
{

    if (bctx==NULL) {

        logoutput_debug("%s: cannot continue ... bevent ctx not defined", __FUNCTION__);
        return 0;

    } else if (bctx->io_backend==NULL) {

        logoutput_debug("%s: cannot continue ... object backend not defined", __FUNCTION__);
        return 0;

    } else if (bctx->bevent) {

        logoutput_debug("%s: cannot continue ... bevent already attached to ctx", __FUNCTION__);
        return 0;

    }

    logoutput_debug("%s", __FUNCTION__);

    if (eloop==NULL) eloop=BEVENTLOOP_get_default_loop();

    if (EVENT_signal_lock_flag(eloop->esignal, &eloop->lock, BEVENTLOOP_LOCK_BCTX)) {
	struct bevent_s *bevent=NULL;

	bevent=BEVENT_create(eloop);

	if (bevent) {

    	    bctx->bevent=bevent;
    	    bevent->bctx=bctx;

	}

	EVENT_signal_unlock_flag(eloop->esignal, &eloop->lock, BEVENTLOOP_LOCK_BCTX);
        return 1;

    }

    return 0;

}

void BEVENT_ctx_detach(struct bevent_ctx_s *bctx)
{
    struct beventloop_s *eloop=NULL;
    struct bevent_s *bevent=NULL;

    if ((bctx==NULL) || (bctx->bevent==0) || (bctx->flags & BEVENT_CTX_FLAG_FALLBACK)) return;

    bevent=bctx->bevent;
    eloop=bevent->eloop;

    if (eloop==NULL) return;
    BEVENT_ctx_replace_with_fallback(bctx, 1);

    /* remove from eventloop */

    BEVENT_ctx_del(eloop, bctx);
    BEVENT_move_to_inactive(bevent);

    /* send task to eventloop to clean the inactive list */

    BEVENTLOOP_send_task(bevent->eloop, BEVENTLOOP_TASK_CLEAN_INACTIVE);

}

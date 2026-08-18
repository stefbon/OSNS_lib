/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-event.h"
#include "libosns-eventloop.h"
#include "libosns-threads.h"
#include "libosns-io.h"
#include "libosns-fs.h"

#include "osns/osns.h"

static struct list_header_s subscriptions;

static void cb_noop(struct osns_ctx_s *octx, unsigned int mask, struct osns_event_s *event, void *ptr)
{}

void OSNS_event_subscription_init(struct osns_event_subscription_s *oes)
{

    if (oes) {

        oes->name="--notset--";
        LIST_element_init(&oes->list, NULL);
        oes->mask=0;
        oes->ptr=NULL;
        oes->cb=cb_noop;

    }

}

void OSNS_event_process(struct osns_ctx_s *octx, struct osns_event_s *event)
{
    struct osns_event_ctx_s *oec=NULL;
    unsigned int eventmask=0;

    if ((event==NULL) || (octx->event_ctx==NULL) || (octx->event_ctx->subscriptions==NULL)) return;
    oec=octx->event_ctx;

    logoutput_debug("%s: event type %u", __FUNCTION__, event->type);

    EVENT_signal_lock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);

    eventmask=(1 << event->type);

    if (eventmask & oec->mask) {
        struct list_element_s *list=NULL;

        list=LIST_header_get_first(oec->subscriptions);

        while (list) {
            struct osns_event_subscription_s *oes=(struct osns_event_subscription_s *)((char *) list - offsetof(struct osns_event_subscription_s, list));

            if (oes->mask & eventmask) (* oes->cb)(octx, eventmask, event, oes->ptr);
            list=LIST_element_get_next(list);

        }

    }

    EVENT_signal_unlock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);
}

void OSNS_event_subscribe(struct osns_ctx_s *octx, struct osns_event_subscription_s *oes)
{
    struct osns_event_ctx_s *oec=NULL;
    unsigned int eventmask=0;

    if ((octx==NULL) || (oes==NULL) || (octx->event_ctx==NULL) || (octx->event_ctx->subscriptions==NULL)) return;
    oec=octx->event_ctx;

    EVENT_signal_lock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);
    LIST_header_add_last(oec->subscriptions, &oes->list);
    oec->mask |= oes->mask;
    logoutput_debug("%s: subscribe %s", __FUNCTION__, ((oes->name) ? oes->name : "--unknown--"));
    EVENT_signal_unlock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);

}

void OSNS_event_unsubscribe(struct osns_ctx_s *octx, struct osns_event_subscription_s *oes)
{
    struct osns_event_ctx_s *oec=NULL;
    struct list_element_s *list=NULL;
    unsigned int mask=0;

    if ((octx==NULL) || (oes==NULL) || (octx->event_ctx==NULL) || (octx->event_ctx->subscriptions==NULL)) return;
    oec=octx->event_ctx;

    EVENT_signal_lock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);
    LIST_element_remove(&oes->list);
    logoutput_debug("%s: unsubscribe %s", __FUNCTION__, ((oes->name) ? oes->name : "--unknown--"));

    /* recalc event mask */

    list=LIST_header_get_first(oec->subscriptions);

    while (list) {
        struct osns_event_subscription_s *oes=(struct osns_event_subscription_s *)((char *) list - offsetof(struct osns_event_subscription_s, list));

        mask |= oes->mask;
        list=LIST_element_get_next(list);

    }

    oec->mask=mask;

    EVENT_signal_unlock_flag(octx->esignal, &oec->lock, OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS);

}

void OSNS_event_init_subscriptions(struct osns_event_ctx_s *oec)
{

    if (oec==NULL) return;
    logoutput_debug("%s", __FUNCTION__);

    memset(oec, 0, sizeof(struct osns_event_ctx_s));
    oec->lock=0;
    oec->subscriptions=&subscriptions;
    LIST_header_init(&subscriptions, 0);
    oec->mask=0;

}

void OSNS_event_clear_subscriptions(struct osns_event_ctx_s *oec)
{
    struct list_element_s *list=NULL;

    if (oec==NULL) return;
    logoutput_debug("%s", __FUNCTION__);

    list=LIST_header_remove_first(oec->subscriptions);

    while (list) {
        struct osns_event_subscription_s *oes=(struct osns_event_subscription_s *)((char *) list - offsetof(struct osns_event_subscription_s, list));

        logoutput_debug("%s: remove %s", __FUNCTION__, ((oes->name) ? oes->name : "--unknown--"));
        list=LIST_header_remove_first(oec->subscriptions);

    }

    oec->mask=0;
    oec->subscriptions=NULL;

}

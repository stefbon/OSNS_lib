/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_EVENT_H
#define OSNS_EVENT_H

/* Prototypes */

void OSNS_event_subscription_init(struct osns_event_subscription_s *oes);

void OSNS_event_process(struct osns_ctx_s *octx, struct osns_event_s *event);

void OSNS_event_subscribe(struct osns_ctx_s *octx, struct osns_event_subscription_s *oes);
void OSNS_event_unsubscribe(struct osns_ctx_s *octx, struct osns_event_subscription_s *oes);

void OSNS_event_init_subscriptions(struct osns_event_ctx_s *oec);
void OSNS_event_clear_subscriptions(struct osns_event_ctx_s *oec);


#endif

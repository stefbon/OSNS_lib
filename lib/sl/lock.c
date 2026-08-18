/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-lsut.h"
#include "libosns-datatypes.h"

#include "sl.h"

/* lock node self */

unsigned char SL_node_set_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr)
{
    return EVENT_signal_lock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_SELF);
}

void SL_node_unset_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr)
{
    EVENT_signal_unlock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_SELF);
}

/* lock node get nect */

unsigned char SL_node_set_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    return EVENT_signal_lock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_GET_NEXT);
}

void SL_node_unset_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    EVENT_signal_unlock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_GET_NEXT);
}

/* lock node get prev */

unsigned char SL_node_set_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    return EVENT_signal_lock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_GET_PREV);
}

void SL_node_unset_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    EVENT_signal_unlock_flag(sl->esignal, &slr->node->lock, SL_NODE_READLOCK_GET_PREV);
}

/* lock next node self */

unsigned char SL_node_set_readlock_self_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    return EVENT_signal_lock_flag(sl->esignal, &slr->next->lock, SL_NODE_READLOCK_SELF);
}

void SL_node_unset_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr)
{
    EVENT_signal_unlock_flag(sl->esignal, &slr->next->lock, SL_NODE_READLOCK_SELF);
}

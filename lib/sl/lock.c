/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"

#include "sl.h"

/* lock node self */

unsigned char SL_node_set_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;
    return (node ? EVENT_signal_lock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_STANDARD) : 0);
}

void SL_node_unset_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;

    if (node) EVENT_signal_unlock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_STANDARD);
}

/* lock node get nect */

unsigned char SL_node_set_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;

    return (node ? EVENT_signal_lock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_RIGHT) : 0);
}

void SL_node_unset_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;

    if (node) EVENT_signal_unlock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_RIGHT);
}

/* lock node get prev */

unsigned char SL_node_set_readlock_get_prev(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;

    return (node ? EVENT_signal_lock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_LEFT) : 0);
}

void SL_node_unset_readlock_get_prev(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *node=slr->node;

    if (node) EVENT_signal_unlock_flag(sl->esignal, &node->lock, SL_NODE_LOCK_SCOPE_LEFT);
}

/* lock next node self */

unsigned char SL_node_set_readlock_self_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *next=slr->next;

    return (next ? EVENT_signal_lock_flag(sl->esignal, &next->lock, SL_NODE_LOCK_SCOPE_STANDARD) : 0);
}

void SL_node_unset_readlock_self_next(struct sl_s *sl, struct sl_find_result_s *slr)
{
    struct sl_node_s *next=slr->next;

    if (next) EVENT_signal_unlock_flag(sl->esignal, &next->lock, SL_NODE_LOCK_SCOPE_STANDARD);
}

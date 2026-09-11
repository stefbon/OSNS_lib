/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"

#include "sl.h"

static int compare_default(struct list_element_s *list, void *lookupdata, void *ptr)
{
    return 0;
}

void SL_init(struct sl_s *sl, unsigned int distance, struct list_header_s *header, struct event_shared_signal_s *esignal)
{

    sl->esignal=(esignal ? esignal : EVENT_signal_get_default());
    sl->distance=(distance ? distance : SL_DISTANCE_DEFAULT);

    LOCKING_init(&sl->locking, sl->esignal);
    sl->compare=compare_default;
    sl->ptr=NULL;
    sl->height=0;
    sl->ncount=NULL;

    sl->node.type=SL_NODE_TYPE_HEAD;
    sl->node.lock=0;
    sl->node.list.header=header;
    sl->node.count=0;
    sl->node.junction=NULL;

}

void SL_set_compare(struct sl_s *sl, int (* compare_cb)(struct list_element_s *list, void *lookupdata, void *ptr), void *ptr)
{
    sl->compare=compare_cb;
    sl->ptr=ptr;
}


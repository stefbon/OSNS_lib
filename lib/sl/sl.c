/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include <math.h>

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"

#include "sl.h"
#include "lock.h"

static struct sl_node_s *SL_get_next_node(struct sl_node_s *node, unsigned int level, unsigned char next)
{
    struct sl_junction_s *slj=(next ? node->junction[level].next : node->junction[level].prev);

    /* there is always a next or prev */

    return (struct sl_node_s *)((char *) slj - (level * sizeof(struct sl_junction_s)) - offsetof(struct sl_node_s, junction));
}

static unsigned int SL_calculate_level(struct sl_s *sl)
{
    unsigned int level=0;
    unsigned int multiplier=sl->distance;

    for (unsigned int i=0; i<sl->height + 1; i++) {

	/* enough nodes on this level ? */

	if ((multiplier * sl->ncount[i].count) > sl->node.list.header->count) break;
	level++;
	multiplier *= sl->distance;

    }

    return level;
}

static void SL_resize_head_junction(struct sl_s *sl, unsigned int height)
{
    struct sl_junction_s *keep=sl->node.junction;
    struct sl_node_count_s *ncount=sl->ncount;

    dorealloc:

    sl->node.junction=realloc(keep, height * sizeof(struct sl_junction_s));
    sl->ncount=realloc(sl->ncount, height * sizeof(struct sl_node_count_s));

    /* realloc may use another memory address */

    if (keep != sl->node.junction) {

	if (sl->node.junction==NULL) {

	    logoutput_debug("%s: error ... unable to reallocate junction (size=%u)", __FUNCTION__, height * sizeof(struct sl_junction_s));
	    goto dorealloc;
	}

	for (unsigned int i=0; i<sl->height; i++) {
	    struct sl_node_s *np=NULL;

	    np=SL_get_next_node(&sl->node, i, 1);
	    np->junction[i].next=&sl->node.junction[i];

	    np=SL_get_next_node(&sl->node, i, 0);
	    np->junction[i].prev=&sl->node.junction[i];
	}

    }

    if (height>sl->height) {

	/* initialize new junctions */

	for (unsigned int i=sl->height; i<height; i++) {

	    sl->node.junction[i].next=&sl->node.junction[i];
	    sl->node.junction[i].prev=&sl->node.junction[i];
	    sl->node.junction[i].step=sl->node.list.header->count;

	    sl->ncount[i].count=0;

	}

    }

    sl->height=height;

}

static void SL_node_insert(struct sl_s *sl, struct sl_node_s *node, struct list_element_s *list, unsigned int step)
{
    unsigned int height=0;
    struct sl_node_s *sln=NULL;
    struct sl_junction_s *slj=NULL;

    /* check the step is big enough */

    if ((step < sl->distance) && ((node->junction[0].step - step) < sl->distance)) return;

    height=SL_calculate_level(sl);
    if (height==0) return;

    /* TODO:

    1. prevent SL to get disbalance
    2. add global lock when resize head

    */

    sln=malloc(sizeof(struct sl_node_s));
    slj=malloc(height * sizeof(struct sl_junction_s));

    if ((sln==NULL) || (slj==NULL)) {

	if (sln) free(sln);
	if (slj) free(slj);
	return;

    }

    memset(sln, 0, sizeof(struct sl_node_s));
    memset(slj, 0, height * sizeof(struct sl_junction_s));

    sln->type=SL_NODE_TYPE_BETWEEN;
    sln->list.list=NULL;
    sln->count=height;
    sln->junction=slj;

    /* resize head junction if required */

    if (height>sl->height) SL_resize_head_junction(sl, height);

    /* balance locally */

    /* too much space left of */

    while ((2 * step) > (node->junction[0].step + 1)) {

	list=LIST_element_get_prev(list);
	step--;

    } 

    /* too much space right of */

    while ((2 * step + 1) < (node->junction[0].step)) {

	list=LIST_element_get_next(list);
	step++;

    }

    sln->list.list=list;

    for (unsigned int i=0; i<height; i++) {
	struct sl_node_s *next=SL_get_next_node(node, i, 1);

	slj[i].next=&next->junction[i];
	next->junction[i].prev=&slj[i];
	slj[i].step=node->junction[i].step + 1 - step;

	slj[i].prev=&node->junction[i];
	node->junction[i].next=&slj[i];
	node->junction[i].step=step;

	sl->ncount[i].count++;

	if (node->count<=i) node=SL_get_next_node(node, i, 0);

    }


}

static void SL_find_shared(struct sl_s *sl, void *lookupdata, struct sl_find_result_s *slr)
{
    struct sl_node_s *next=NULL;
    unsigned int level=0;
    int result=0;
    unsigned char tmp=0;

    slr->node=&sl->node;
    slr->level=slr->node->count-1;
    slr->type=SL_NODE_LOCK_SCOPE_RIGHT;

    tmp=SL_node_set_readlock_self(sl, slr);
    tmp=SL_node_set_readlock_get_prev(sl, slr);

    /* check first the boundaries: before the first or after the last */

    /* try last */

    slr->list=LIST_header_get_last(sl->node.list.header);
    result=(* sl->compare)(slr->list, lookupdata, sl->ptr);

    if (result<=0) {

	/* list is smaller than lookupdata -> after list */

	if (result==0) {

	    slr->code=SL_FIND_RESULT_CODE_EXACT;
	    SL_node_unset_readlock_get_prev(sl, slr);

	} else {

	    slr->code=SL_FIND_RESULT_CODE_AFTER;

	}

	return;

    }

    SL_node_unset_readlock_get_prev(sl, slr);

    /* try first
	head node already self locked */

    tmp=SL_node_set_readlock_get_next(sl, slr);

    slr->list=LIST_header_get_first(sl->node.list.header);
    result=(* sl->compare)(slr->list, lookupdata, sl->ptr);
    slr->type=SL_NODE_LOCK_SCOPE_LEFT;

    if (result>=0) {

	/* list is bigger than lookupdata -> before list */

	if (result==0) {

	    slr->code=SL_FIND_RESULT_CODE_EXACT;
	    SL_node_unset_readlock_get_next(sl, slr);

	} else {

	    slr->code=SL_FIND_RESULT_CODE_BEFORE;

	}

	return;

    }

    SL_node_unset_readlock_get_next(sl, slr);

    /* start from left to right (==first to last) */

    slr->code=SL_FIND_RESULT_CODE_AFTER;
    slr->type=SL_NODE_LOCK_SCOPE_STANDARD;

    search:

    /* TODO: decide here to start at the head of the list or at the tail
	needed: an indication the entry looked for is closer at the tail than the head */

    if (SL_node_set_readlock_get_next(sl, slr)) {

	slr->next=SL_get_next_node(slr->node, slr->level, 1);

	if (SL_node_set_readlock_self_next(sl, slr)==0) {

	    logoutput_debug("%s: unable to lock next self", __FUNCTION__);
	    SL_node_unset_readlock_get_next(sl, slr);
	    goto search;

	}

	slr->list=((slr->next->type==SL_NODE_TYPE_BETWEEN) ? slr->next->list.list : LIST_header_get_last(sl->node.list.header));
	result=(* sl->compare)(slr->list, lookupdata, sl->ptr);

    } else {

	logoutput_debug("%s: unable to lock get next", __FUNCTION__);
	slr->error=SL_ERROR_UNABLE_TO_LOCK;

    }

    if (result<0) {

	/* list "smaller" then lookupdata -> before, try next node: skip */

	/* release lock on node to move to next */

	SL_node_unset_readlock_get_next(sl, slr);
	SL_node_unset_readlock_self(sl, slr);

	slr->node=slr->next;
	slr->next=NULL;

	goto search;

    } else if (result==0) {

	/* release lock on node to move to next */

	SL_node_unset_readlock_get_next(sl, slr);
	SL_node_unset_readlock_self(sl, slr);

	slr->node=slr->next;
	slr->next=NULL;
	slr->code=SL_FIND_RESULT_CODE_EXACT;
	return;

    } else {

	SL_node_unset_readlock_self_next(sl, slr);

	/* result > 0: next is too far, go one level down */

	if (slr->level>0) {

	    slr->level--;
	    goto search;

	}

    }

}

static unsigned char SL_find_exact(struct sl_s *sl, void *lookupdata, struct sl_find_result_s *slr)
{
    unsigned char result=0;

    SL_find_shared(sl, lookupdata, slr);

    if (slr->code==SL_FIND_RESULT_CODE_EXACT) {

	slr->step=0;
	return 1;

    } else {
	struct list_element_s *list=(slr->code==SL_FIND_RESULT_CODE_AFTER ? LIST_element_get_next(slr->list) : LIST_element_get_prev(slr->list));
	int resultcompare=0;

	while (list) {

	    resultcompare=(* sl->compare)(list, lookupdata, sl->ptr);

	    if (resultcompare==0) {

		/* exact match */

		slr->list=list;
		slr->step++;
		result=1;
		goto out;

	    } else if (slr->code==SL_FIND_RESULT_CODE_AFTER && (resultcompare<0)) {

		/* still before */

		slr->list=list;
		slr->step++;
		list=LIST_element_get_next(list);

	    } else if (slr->code==SL_FIND_RESULT_CODE_BEFORE && (resultcompare>0)) {

		/* still after */

		slr->list=list;
		slr->step++;
		list=LIST_element_get_prev(list);

	    } else {

		/* when here not found, stepped too far */
		break;

	    }

	}

    }

    out:

    if (slr->code==SL_FIND_RESULT_CODE_BEFORE) {
	struct sl_node_s *prev=SL_get_next_node(slr->node, 0, 0);

	slr->step=prev->junction[0].step - slr->step;

    }

    if (result==1) slr->code=SL_FIND_RESULT_CODE_EXACT;
    return result;

}

unsigned char SL_find(struct sl_s *sl, void *lookupdata, struct list_element_s **p_list)
{
    struct sl_find_result_s slr=SL_FIND_RESULT_INIT;

    return SL_find_exact(sl, lookupdata, &slr);
}

unsigned char SL_insert(struct sl_s *sl, void *lookupdata, struct list_element_s *list)
{
    struct sl_find_result_s slr=SL_FIND_RESULT_INIT;

    if (SL_find_exact(sl, lookupdata, &slr)) {

	logoutput_debug("%s: cannot insert ... already exist", __FUNCTION__);
	return 0;

    }

    if (slr.code==SL_FIND_RESULT_CODE_BEFORE) {

	/* insert before list */

	if (slr.node->type==SL_NODE_TYPE_HEAD) {

	    /* before start of list */

	    LIST_header_add_first(slr.node->list.header, list);

	    /* TODO:
		add extra node if distance to next node gets too big */

	} else {

	    LIST_element_add_before(slr.list, list);

	    /* TODO:
		add extra node if distance to next node gets too big */

	    SL_node_insert(sl, slr.node, list, slr.step);


	}

    } else if (slr.code==SL_FIND_RESULT_CODE_AFTER) {

	if (slr.node->type==SL_NODE_TYPE_HEAD) {

	    LIST_header_add_last(slr.node->list.header, list);

	    /* TODO: add extra node if step gets too big */

	} else {

	    LIST_element_add_after(slr.list, list);

	    /* TODO:
		add extra node if distance to next node gets too big */

	    SL_node_insert(sl, slr.node, list, slr.step);

	}

    }

    return 1;
}

unsigned char SL_remove(struct sl_s *sl, void *lookupdata, struct list_element_s *list)
{

    /* TODO */

    



    
}

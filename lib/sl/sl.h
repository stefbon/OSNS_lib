/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef SL_SL_H
#define SL_SL_H

#define SL_DISTANCE_DEFAULT		5

struct sl_junction_s {
    uint64_t				step;
    struct sl_junction_s		*next;
    struct sl_junction_s		*prev;
};

struct sl_node_count_s {
    unsigned int			count;
};

#define SL_NODE_TYPE_HEAD		1
#define SL_NODE_TYPE_BETWEEN		2

#define SL_NODE_LOCK_TYPE_READ		4
#define SL_NODE_LOCK_TYPE_WRITE		2


struct sl_node_s {
    unsigned char			type;
    unsigned int			lockflags;
    unsigned int			lock;
    union sl_node_u {
	struct list_element_s		*list;
	struct list_header_s		*header;
    } list;
    unsigned int			count;
    struct sl_junction_s		*junction;
};

struct sl_s {
    struct event_shared_signal_s	*esignal;
    unsigned int 			distance;
    struct locking_s			locking;
    int					(* compare)(struct list_element_s *list, void *lookupdata, void *ptr);
    void				*ptr;
    unsigned int			height;
    struct sl_node_count_s		*ncount;
    struct sl_node_s			node;
};

#define SL_FIND_RESULT_CODE_BEFORE	1
#define SL_FIND_RESULT_CODE_EXACT	2
#define SL_FIND_RESULT_CODE_AFTER	3

#define SL_NODE_LOCK_SCOPE_LEFT		1
#define SL_NODE_LOCK_SCOPE_STANDARD	2
#define SL_NODE_LOCK_SCOPE_RIGHT	3

#define SL_ERROR_UNABLE_TO_LOCK		1

struct sl_find_result_s {
    unsigned char			type;
    unsigned int			lock;
    struct sl_node_s			*node;
    struct sl_node_s			*next;
    unsigned char			scope;
    unsigned int			level;
    unsigned char			code;
    unsigned int			error;
    struct list_element_s 		*list;
};

#define SL_FIND_RESULT_INIT		{SL_NODE_LOCK_TYPE_READ, 0, NULL, NULL, SL_NODE_LOCK_SCOPE_STANDARD, 0, 0, 0, NULL}

#endif


#ifndef LIB_HASHTABLE_TABLE_H
#define LIB_HASHTABLE_TABLE_H

#include "libosns-list.h"
#include "libosns-time.h"

#define LIB_HASHTABLE_FLAG_CUSTOM_ESIGNAL               1
#define LIB_HASHTABLE_FLAG_DEFAULT_ESIGNAL              2
#define LIB_HASHTABLE_FLAG_TABLE_ALLOCATED		4

#define LIB_UNIQUE_ID_TYPE_UINT8                        1
#define LIB_UNIQUE_ID_TYPE_UINT16                       2
#define LIB_UNIQUE_ID_TYPE_UINT32                       3
#define LIB_UNIQUE_ID_TYPE_UINT64                       4

struct unique_id_s {
    unsigned char                                       type;
    uint64_t					        id064;
};

struct hashtable_s {
    unsigned int                                        flags;
    struct list_header_s                                *table;
    unsigned int                                        size;
    struct event_shared_signal_s                        *esignal;
    struct unique_id_s                                  id;
    void                                                (* increaseid)(struct hashtable_s *htable);
    unsigned int                                        (* hashfunction)(struct hashtable_s *htable, struct unique_id_s *id);
    unsigned char                                       (* compare)(struct hashtable_s *htable, struct list_element_s *list, struct unique_id_s *id, void *ptr);
    void                                                (* add)(struct hashtable_s *htable, struct list_element_s *list);
    void                                                (* remove)(struct hashtable_s *htable, struct list_element_s *list);
    unsigned char                                       (* find)(struct hashtable_s *htable, struct unique_id_s *id, void (* cb_found)(struct list_element_s *l, void *ptr), void *ptr);
};

/* prototypes */



void HASHTABLE_walk(struct hashtable_s *htable, void (* cb_clear)(struct list_element_s *list, void *ptr), void *ptr);
void HASHTABLE_clear(struct hashtable_s *htable);
int HASHTABLE_init(struct hashtable_s *htable, struct list_header_s *h, unsigned int size, struct event_shared_signal_s *esignal, unsigned int flags, unsigned char idtype);
void HASHTABLE_set_compare_function(struct hashtable_s *htabhe, unsigned char (* compare)(struct hashtable_s *htable, struct list_element_s *l, struct unique_id_s *id, void *ptr));

void HASHTABLE_add(struct hashtable_s *htable, struct list_element_s *l);
void HASHTABLE_remove(struct hashtable_s *htable, struct list_element_s *l);

unsigned char HASHTABLE_find(struct hashtable_s *htable, struct unique_id_s *id, void (* cb_found)(struct list_element_s *l, void *ptr), void *ptr);

#endif

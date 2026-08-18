/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_LOCK_H
#define LIB_LOCK_H

struct locking_s {
    struct event_shared_signal_s	*esignal;
    unsigned int			lock;
    struct list_element_s		*writer;
    struct list_header_s		writers;
};

#define LOCK_TYPE_READ			1
#define LOCK_TYPE_WRITE			2

struct lock_s {
    unsigned char			type;
    unsigned int			lock;
    unsigned long			threadid;
    struct list_element_s		list;
};

/* prototypes */

void LOCKING_init(struct locking_s *locking, struct event_shared_signal_s *esignal);
void LOCK_init(struct lock_s *lock, unsigned char type);

unsigned char LOCK_set_readlock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout);
void LOCK_unset_readlock(struct locking_s *locking, struct lock_s *lock);
unsigned char LOCK_upgrade_readlock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout);

unsigned char LOCK_set_writelock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout);
void LOCK_unset_writelock(struct locking_s *locking, struct lock_s *lock);

#endif

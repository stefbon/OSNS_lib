/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-event.h"
#include "libosns-lsut.h"

#include "lock.h"

void LOCKING_init(struct locking_s *locking, struct event_shared_signal_s *esignal)
{

    memset(locking, 0, sizeof(struct locking_s));

    locking->esignal=esignal;
    locking->lock=0;
    locking->writer=NULL;
    LIST_header_init(&locking->writers, 0);
}

void LOCK_init(struct lock_s *lock, unsigned char type)
{
    lock->type=type;
    lock->threadid=(unsigned long) pthread_self();
    LIST_element_init(&lock->list);
}

unsigned char LOCK_set_readlock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout)
{
    struct event_shared_signal_s *esignal=NULL;

    if ((lock==NULL) || (lock->type != LOCK_TYPE_READ) || (lock->threadid != pthread_self())) return 0;
    esignal=locking->esignal;

    EVENT_signal_lock(esignal);

    /* wait for any write lock to disappear */

    while (locking->writers.count) {

        int tmp=EVENT_signal_wait(esignal, timeout);

        if (tmp) {

	    EVENT_signal_unlock(esignal);
	    return 0;

	}

    }

    locking->lock += 4;
    lock->lock = 4;
    EVENT_signal_unlock(lesignal);
    return 1;

}

void LOCK_unset_readlock(struct locking_s *locking, struct lock_s *lock)
{
    struct event_shared_signal_s *esignal=NULL;

    if ((lock==NULL) || (lock->type != LOCK_TYPE_READ)  || (lock->threadid != pthread_self())) return;
    esignal=locking->esignal;

    EVENT_signal_lock(esignal);
    locking->lock-=lock->lock;
    lock->lock=0;
    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);
}

static unsigned char lock_set_writelock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout)
{

    LIST_header_add_last(&locking->writers, &lock->list);

    while (locking->writers.count>1) {

        int tmp=EVENT_signal_wait(esignal, timeout);
        if (tmp) {

	    LIST_element_remove(&lock->list);
	    EVENT_signal_unlock(locking->esignal);
	    return 0;

	}

    }

    locking->writer=&lock->list;
    locking->lock=2;
    lock->lock=2;
    return 1;

}

unsigned char LOCK_upgrade_readlock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout)
{
    struct event_shared_signal_s *esignal=NULL;
    unsigned char result=0;

    if ((lock==NULL) || (lock->type != LOCK_TYPE_READ) || (lock->lock!=4) || (lock->threadid != pthread_self())) return 0;
    esignal=locking->esignal;

    EVENT_signal_lock(esignal);

    locking->lock-=lock->lock;
    lock->lock=0;
    lock->type=LOCK_TYPE_WRITE;

    result=lock_set_writelock(locking, lock, timeout);

    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);
    return result;

}

unsigned char LOCK_set_writelock(struct locking_s *locking, struct lock_s *lock, struct timespec_s *timeout)
{
    struct event_shared_signal_s *esignal=NULL;
    unsigned char result=0;

    if ((lock==NULL) || (lock->type != LOCK_TYPE_WRITE) || (lock->lock!=0) || (lock->threadid != pthread_self())) return 0;
    esignal=locking->esignal;

    EVENT_signal_lock(esignal);
    result=lock_set_writelock(locking, lock, timeout);
    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);

    return result;

}

void LOCK_unset_writelock(struct locking_s *locking, struct lock_s *lock)
{
    struct event_shared_signal_s *esignal=NULL;

    if ((lock==NULL) || (lock->type != LOCK_TYPE_WRITE) || (lock->lock!=2) || (lock->threadid != pthread_self())) return;
    EVENT_signal_lock(locking->esignal);

    if (lock->list.h==&locking->writers) LIST_element_remove(&lock->list);
    if (locking->wriiter==&lock->list) locking->writer=NULL;

    locking->lock-=lock->lock;
    lock->lock=0;
    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);

}

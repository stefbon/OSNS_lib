/*
  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Stef Bon <stefbon@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-event.h"

#include "lock.h"

/* LOCK bit functions */

/* HEADER MODIFY ... */

unsigned char LIST_header_lock(struct list_header_s *h, unsigned int lockflag)
{
    EVENT_signal_lock_flag(h->esignal, &h->lockflags, lockflag);
    return 1;
}

void LIST_header_unlock(struct list_header_s *h, unsigned int lockflag)
{
    EVENT_signal_unlock_flag(h->esignal, &h->lockflags, lockflag);
}

/* ELEMENT NEXT/PREV ... */

unsigned char LIST_element_lock_op(struct list_header_s *h, struct list_element_s *e, unsigned int lockflag)
{
    EVENT_signal_lock_flag(h->esignal, &e->lockflags, lockflag);
    return 1;
}

void LIST_element_unlock_op(struct list_header_s *h, struct list_element_s *e, unsigned int lockflag)
{
    EVENT_signal_unlock_flag(h->esignal, &e->lockflags, lockflag);
}


/* LOCK read/write functions */

unsigned char LIST_header_set_read_lock(struct list_header_s *h, struct timespec_s *timeout)
{

    lockstart:

    EVENT_signal_lock(h->esignal);

    /* wait for any write lock to disappear */

    while ((h->lockflags % 4) > 0) {

        int tmp=EVENT_signal_wait(h->esignal, timeout);

    }

    h->lockflags += 4; /* need read access: increase the lock with 4 */
    EVENT_signal_unlock(h->esignal);

    return 1;

}

unsigned char LIST_header_unset_read_lock(struct list_header_s *h)
{
    unsigned char unlocksuccess=0;

    EVENT_signal_lock(h->esignal);

    if (h->lockflags>3) {

        h->lockflags -= 4;
        unlocksuccess=1;
        EVENT_signal_broadcast(h->esignal);

    }

    EVENT_signal_unlock(h->esignal);
    return unlocksuccess;
}

unsigned char LIST_header_upgrade_read_lock(struct list_header_s *h, struct timespec_s *timeout)
{

    EVENT_signal_lock(h->esignal);

    if (h->lockflags % 3) {

        if (h->lockflags % 1) {

            /* there is already another thread trying to get write access
            .... the only thing to do is to stop here and retry
            otherwise a deadlock may occur ... */

            h->lockflags -= 4;
            EVENT_signal_broadcast(h->esignal);
            goto errorunlock;

        }

        h->lockflags -= 4;
        h->lockflags += 1; /* make sure this thread is the first to get the next writelock */

        while (h->lockflags & 2) {

            int tmp=EVENT_signal_wait(h->esignal, timeout);
            if (tmp==ETIMEDOUT) goto errorunlock;

        }

    } else if (h->lockflags / 4) {

        h->lockflags += 1; /* make sure this thread is the first to get the next writelock */

        while ((h->lockflags / 4) > 0) {

            int tmp=EVENT_signal_wait(h->esignal, timeout);
            if (tmp==ETIMEDOUT) goto errorunlock;

        }

    }

    /* now exclusive access .... */

    h->lockflags = 2;
    EVENT_signal_broadcast(h->esignal);
    EVENT_signal_unlock(h->esignal);

    return 1;

    errorunlock:
    EVENT_signal_unlock(h->esignal);
    return 0;
}

unsigned char LIST_header_set_write_lock(struct list_header_s *h, struct timespec_s *timeout)
{

    EVENT_signal_lock(h->esignal);

    if (h->lockflags & 3) {

        if (h->lockflags & 1) {

            /* there is already somebody trying to get a writelock */

            while (h->lockflags & 1) {

                int tmp=EVENT_signal_wait(h->esignal, timeout);
                if (tmp==ETIMEDOUT) goto errorunlock;

            }

        }

        h->lockflags |= 1;

        while (h->lockflags & 2) {

            int tmp=EVENT_signal_wait(h->esignal, timeout);
            if (tmp==ETIMEDOUT) goto errorunlock;

        }

    } else if (h->lockflags / 4) {

        /* there are readers: wait */

        h->lockflags |= 1;

        while (h->lockflags / 4) {

            int tmp=EVENT_signal_wait(h->esignal, timeout);
            if (tmp==ETIMEDOUT) goto errorunlock;

        }

    }

    h->lockflags = 2; /* now exclusive access */
    h->threadid = pthread_self(); /* set thread id of owner lock */
    EVENT_signal_broadcast(h->esignal);
    EVENT_signal_unlock(h->esignal);
    return 1;

    errorunlock:
    EVENT_signal_unlock(h->esignal);
    return 0;

}

unsigned char LIST_header_unset_write_lock(struct list_header_s *h)
{
    unsigned char unlocksuccess=0;

    /* only owner of lock can unset/release */

    EVENT_signal_lock(h->esignal);

    if ((h->threadid==pthread_self()) && (h->lockflags & 3)) {

        h->lockflags=0;
        EVENT_signal_broadcast(h->esignal);
        h->threadid=0;
        unlocksuccess=1;

    }

    EVENT_signal_unlock(h->esignal);
    return unlocksuccess;
}

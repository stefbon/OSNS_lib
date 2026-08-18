/*
  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Stef Bon <stefbon@gmail.com>

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

#include <pthread.h>

#include "libosns-log.h"
#include "signal.h"

#ifdef __linux__

static pthread_mutex_t			default_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t			default_cond=PTHREAD_COND_INITIALIZER;

#endif

/* DEFAULT SIGNAL */

static int event_signal_lock_default(struct event_shared_signal_s *s)
{
#ifdef __linux__
    return pthread_mutex_lock(s->backend.pthread.mutex);
#else 
    return -1;
#endif
}

static int event_signal_unlock_default(struct event_shared_signal_s *s)
{
#ifdef __linux__
    return pthread_mutex_unlock(s->backend.pthread.mutex);
#else 
    return -1;
#endif
}

static int event_signal_broadcast_default(struct event_shared_signal_s *s)
{
#ifdef __linux__
    return pthread_cond_broadcast(s->backend.pthread.cond);
#else 
    return -1;
#endif
}

static int event_signal_condwait_default(struct event_shared_signal_s *s)
{
#ifdef __linux__
    return pthread_cond_wait(s->backend.pthread.cond, s->backend.pthread.mutex);
#else 
    return -1;
#endif
}

static int event_signal_condtimedwait_default(struct event_shared_signal_s *s, struct timespec_s *t)
{
#ifdef __linux__
    struct timespec expire={.tv_sec=t->st_sec, .tv_nsec=t->st_nsec};
    return pthread_cond_timedwait(s->backend.pthread.cond, s->backend.pthread.mutex, &expire);
#else 
    return -1;
#endif
}

struct event_shared_signal_s default_shared_signal_initializer = {
    .flags				= 0,
    .lock				= event_signal_lock_default,
    .unlock				= event_signal_unlock_default,
    .broadcast			        = event_signal_broadcast_default,
    .condwait			        = event_signal_condwait_default,
    .condtimedwait			= event_signal_condtimedwait_default,
#ifdef __linux__
    .backend.pthread.mutex		= &default_mutex,
    .backend.pthread.cond		= &default_cond,
#else
    .backend.ptr			= NULL,
#endif
};

struct event_shared_signal_s *EVENT_signal_get_default()
{
    return &default_shared_signal_initializer;
}

/* CUSTOM */

struct event_shared_signal_s custom_shared_signal_initializer = {

    .flags				= EVENT_SHARED_SIGNAL_FLAG_CUSTOM,
    .lock				= event_signal_lock_default,
    .unlock				= event_signal_unlock_default,
    .broadcast			        = event_signal_broadcast_default,
    .condwait			        = event_signal_condwait_default,
    .condtimedwait			= event_signal_condtimedwait_default,
#ifdef __linux__
    .backend.pthread.mutex		= NULL,
    .backend.pthread.cond		= NULL,
#else
    .backend.ptr			= NULL,
#endif
};

void EVENT_signal_set_custom(struct event_shared_signal_s *esignal, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    memcpy(esignal, &custom_shared_signal_initializer, sizeof(struct event_shared_signal_s));
    esignal->backend.pthread.mutex = mutex;
    esignal->backend.pthread.cond = cond;
}

static void freehlpr(void **p_ptr)
{
    void *ptr=((p_ptr) ? *p_ptr : NULL);
    free(ptr);
    *p_ptr=NULL;
}

/* CREATE / CLEAR / FREE */

struct event_shared_signal_s *EVENT_signal_create_custom()
{
    struct event_shared_signal_s *esignal=malloc(sizeof(struct event_shared_signal_s));
    pthread_mutex_t *mutex=malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *cond=malloc(sizeof(pthread_cond_t));

    if (esignal && mutex && cond) {

	memset(esignal, 0, sizeof(struct event_shared_signal_s));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_init(cond, NULL);
	EVENT_signal_set_custom(esignal, mutex, cond);
	esignal->flags = EVENT_SHARED_SIGNAL_FLAG_ALLOC | EVENT_SHARED_SIGNAL_FLAG_ALLOC_MUTEX | EVENT_SHARED_SIGNAL_FLAG_ALLOC_COND;

    } else {

	freehlpr((void **) &esignal);
	freehlpr((void **) &mutex);
	freehlpr((void **) &cond);

    }

    return esignal;
}

void EVENT_signal_clear(struct event_shared_signal_s **p_s)
{
    struct event_shared_signal_s *esignal=(p_s ? *p_s : NULL);

    if (esignal) {

	if (esignal->flags & EVENT_SHARED_SIGNAL_FLAG_ALLOC_MUTEX) {

	    pthread_mutex_destroy(esignal->backend.pthread.mutex);
	    freehlpr((void **)&esignal->backend.pthread.mutex);
	    esignal->flags &= ~EVENT_SHARED_SIGNAL_FLAG_ALLOC_MUTEX;

	}

	if (esignal->flags & EVENT_SHARED_SIGNAL_FLAG_ALLOC_COND) {

	    pthread_cond_destroy(esignal->backend.pthread.cond);
	    freehlpr((void **)&esignal->backend.pthread.cond);
	    esignal->flags &= ~EVENT_SHARED_SIGNAL_FLAG_ALLOC_COND;

	}

	if (esignal->flags & EVENT_SHARED_SIGNAL_FLAG_ALLOC) {

	    freehlpr((void **)p_s);

	}

    }

}

/* GENERALIZED functions */

int EVENT_signal_lock(struct event_shared_signal_s *s)
{
    return (* s->lock)(s);
}

int EVENT_signal_unlock(struct event_shared_signal_s *s)
{
    return (* s->unlock)(s);
}

int EVENT_signal_broadcast(struct event_shared_signal_s *s)
{
    return (* s->broadcast)(s);
}

int EVENT_signal_wait(struct event_shared_signal_s *s, struct timespec_s *t)
{
    return (t) ? (* s->condtimedwait)(s, t) : (* s->condwait)(s);
}

void EVENT_signal_broadcast_locked(struct event_shared_signal_s *s)
{
    EVENT_signal_lock(s);
    EVENT_signal_broadcast(s);
    EVENT_signal_unlock(s);
}

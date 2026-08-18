/*
  2010, 2011, 2012, 2013, 2014, 2015 Stef Bon <stefbon@gmail.com>

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

#ifndef _LIB_EVENT_SIGNAL_H
#define _LIB_EVENT_SIGNAL_H

#include "libosns-time.h"
#include "libosns-error.h"

#include <pthread.h>


#define EVENT_SHARED_SIGNAL_FLAG_ALLOC			1
#define EVENT_SHARED_SIGNAL_FLAG_ALLOC_MUTEX		2
#define EVENT_SHARED_SIGNAL_FLAG_ALLOC_COND	        4
#define EVENT_SHARED_SIGNAL_FLAG_CUSTOM                 8

struct event_shared_signal_s {
    unsigned int					flags;
    pthread_mutex_t					*mutex;
    pthread_cond_t					*cond;
    int							(* lock)(struct event_shared_signal_s *s);
    int							(* unlock)(struct event_shared_signal_s *s);
    int							(* broadcast)(struct event_shared_signal_s *s);
    int							(* condwait)(struct event_shared_signal_s *s);
    int							(* condtimedwait)(struct event_shared_signal_s *s, struct timespec_s *expire);
    union signal_backend_u {
#ifdef __linux__
	struct signal_pthread_s {
	    pthread_mutex_t				*mutex;
	    pthread_cond_t				*cond;
	} pthread;
#endif
	void						*ptr;
    } backend;
};

extern struct event_shared_signal_s default_shared_signal_initializer;
#define EVENT_SHARED_SIGNAL_DEFAULT_INIT default_shared_signal_initializer

/* prototypes */

struct event_shared_signal_s *EVENT_signal_get_default();

void EVENT_signal_set_custom(struct event_shared_signal_s *esignal, pthread_mutex_t *mutex, pthread_cond_t *cond);
struct event_shared_signal_s *EVENT_signal_create_custom();

void EVENT_signal_clear(struct event_shared_signal_s **p_s);

int EVENT_signal_lock(struct event_shared_signal_s *s);
int EVENT_signal_unlock(struct event_shared_signal_s *s);
int EVENT_signal_broadcast(struct event_shared_signal_s *s);
int EVENT_signal_wait(struct event_shared_signal_s *s, struct timespec_s *t);

void EVENT_signal_broadcast_locked(struct event_shared_signal_s *s);

#endif

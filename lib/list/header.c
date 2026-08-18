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

#include "list.h"
#include "header.h"
#include "lock.h"

#include "insert.h"
#include "next.h"
#include "remove.h"

static pthread_mutex_t list_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t list_cond=PTHREAD_COND_INITIALIZER;
static struct event_shared_signal_s list_esignal;
static unsigned char initdone=0;

static void LIST_module_init()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(esignal)==0) {

	if (initdone==0) {

	    initdone=1;
	    EVENT_signal_set_custom(&list_esignal, &list_mutex, &list_cond);

	}

	EVENT_signal_unlock(esignal);

    }

}

/* HEADER init */

static void LIST_header_init_headtail_elements(struct list_header_s *h, struct list_element_s *e, unsigned char tail)
{
    unsigned char lock=((h->flags & LIST_HEADER_FLAG_LOCKBIT) ? 1 : 0);
    LIST_element_init(e, h);
    e->ops=(tail) ? LIST_element_get_ops_tail(lock) : LIST_element_get_ops_head(lock);
}

void LIST_header_init(struct list_header_s *h, unsigned char lock)
{
    struct list_element_s *head=NULL;
    struct list_element_s *tail=NULL;

    LIST_module_init();

    if (h==NULL) {

	logoutput_warning("%s: header empty", __FUNCTION__);
	return;

    } else if (h->flags & LIST_HEADER_FLAG_INIT) {

        logoutput_warning("%s: header already initialized?", __FUNCTION__);

    }

    h->flags = (LIST_HEADER_FLAG_INIT | (lock ? LIST_HEADER_FLAG_LOCKBIT : 0));

    h->lockflags=0;
    h->count=0;
    h->esignal=&list_esignal;

    head=&h->head;
    tail=&h->tail;

    LIST_header_init_headtail_elements(h, head, 0);
    LIST_header_init_headtail_elements(h, tail, 1);

    head->n=tail;
    head->p=tail;
    tail->p=head;
    tail->n=head;

}

void LIST_header_set_esignal(struct list_header_s *h, struct event_shared_signal_s *esignal)
{
    if (h && esignal) h->esignal=esignal;
}

/* user functions */

struct list_element_s *LIST_header_get_first(struct list_header_s *h)
{
    struct list_element_s *e=&h->head;
    return (* e->ops->get_next)(e);
}

struct list_element_s *LIST_header_get_last(struct list_header_s *h)
{
    struct list_element_s *e=&h->tail;
    return (* e->ops->get_prev)(e);
}

struct list_element_s *LIST_header_remove_first(struct list_header_s *h)
{
    struct list_element_s *e=LIST_header_get_first(h);
    if (e) LIST_element_remove(e);
    return e;
}

struct list_element_s *LIST_header_remove_last(struct list_header_s *h)
{
    struct list_element_s *e=LIST_header_get_last(h);
    if (e) LIST_element_remove(e);
    return e;
}

void LIST_header_add_last(struct list_header_s *h, struct list_element_s *e)
{
    struct list_element_s *tail=&h->tail;
    (* tail->ops->insert_before)(tail, e);
}

void LIST_header_add_first(struct list_header_s *h, struct list_element_s *e)
{
    struct list_element_s *head=&h->head;
    (* head->ops->insert_after)(head, e);
}

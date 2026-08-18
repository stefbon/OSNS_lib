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
#include "lock.h"

struct list_element_s *cb_list_element_get_next_locked(struct list_element_s *e)
{
    struct list_element_s *n=NULL;
    struct list_header_s *h=e->h;

    if (h==NULL) return NULL;

    if (LIST_element_lock_op(h, e, LIST_OP_LOCK_TYPE_NEXT)) {

        n=e->n;
        LIST_element_unlock_op(h, e, LIST_OP_LOCK_TYPE_NEXT);

    }

    return (* n->ops->get_self)(n);
}

struct list_element_s *cb_list_element_get_next(struct list_element_s *e)
{
    struct list_element_s *n=NULL;
    struct list_header_s *h=e->h;

    if (h==NULL) return NULL;
    n=e->n;
    return (* n->ops->get_self)(n);
}

struct list_element_s *cb_list_element_get_prev_locked(struct list_element_s *e)
{
    struct list_element_s *p=NULL;
    struct list_header_s *h=e->h;

    if (h==NULL) return NULL;

    if (LIST_element_lock_op(h, e, LIST_OP_LOCK_TYPE_PREV)) {

        p=e->p;
        LIST_element_unlock_op(h, e, LIST_OP_LOCK_TYPE_PREV);

    }

    return (* p->ops->get_self)(p);
}

struct list_element_s *cb_list_element_get_prev(struct list_element_s *e)
{
    struct list_element_s *p=NULL;
    struct list_header_s *h=e->h;

    if (h==NULL) return NULL;
    p=e->p;
    return (* p->ops->get_self)(p);
}

struct list_element_s *cb_list_element_get_next_head(struct list_element_s *e)
{
    struct list_header_s *h=e->h;
    return cb_list_element_get_next(e);
}

struct list_element_s *cb_list_element_get_prev_tail(struct list_element_s *e)
{
    struct list_header_s *h=e->h;
    return cb_list_element_get_prev(e);
}

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

static struct list_element_s *cb_list_element_get_self(struct list_element_s *e)
{
    return e;
}

static struct list_element_s *cb_list_element_get_null(struct list_element_s *e)
{
    return NULL;
}

static void cb_list_element_do_null(struct list_element_s *e)
{
}

struct list_element_ops_s head_element_ops = {
    .name                                       = "H",
    .get_self                                   = cb_list_element_get_null,
    .get_next                                   = cb_list_element_get_next_head,
    .get_prev                                   = cb_list_element_get_null,
    .remove                                     = cb_list_element_do_null,
    .insert_after                               = cb_list_element_insert_after,
    .insert_before                              = cb_list_element_insert_null,
};

struct list_element_ops_s head_element_ops_locked = {
    .name                                       = "H",
    .get_self                                   = cb_list_element_get_null,
    .get_next                                   = cb_list_element_get_next_head,
    .get_prev                                   = cb_list_element_get_null,
    .remove                                     = cb_list_element_do_null,
    .insert_after                               = cb_list_element_insert_after_locked,
    .insert_before                              = cb_list_element_insert_null,
};



struct list_element_ops_s *LIST_element_get_ops_head(unsigned char lock)
{
    return (lock ? &head_element_ops_locked : &head_element_ops);
}

struct list_element_ops_s tail_element_ops = {
    .name                                       = "T", 
    .get_self                                   = cb_list_element_get_null,
    .get_next                                   = cb_list_element_get_null,
    .get_prev                                   = cb_list_element_get_prev_tail,
    .remove                                     = cb_list_element_do_null,
    .insert_after                               = cb_list_element_insert_null,
    .insert_before                              = cb_list_element_insert_before,
};

struct list_element_ops_s tail_element_ops_locked = {
    .name                                       = "T", 
    .get_self                                   = cb_list_element_get_null,
    .get_next                                   = cb_list_element_get_null,
    .get_prev                                   = cb_list_element_get_prev_tail,
    .remove                                     = cb_list_element_do_null,
    .insert_after                               = cb_list_element_insert_null,
    .insert_before                              = cb_list_element_insert_before_locked,
};

struct list_element_ops_s *LIST_element_get_ops_tail(unsigned char lock)
{
    return (lock ? &tail_element_ops_locked : &tail_element_ops);
}

/* default ops for in between elements */

struct list_element_ops_s default_element_ops = {
    .name                                       = "D",
    .get_self                                   = cb_list_element_get_self,
    .get_next                                   = cb_list_element_get_next,
    .get_prev                                   = cb_list_element_get_prev,
    .remove                                     = cb_list_element_remove,
    .insert_after                               = cb_list_element_insert_after,
    .insert_before                              = cb_list_element_insert_before,
};

struct list_element_ops_s default_element_ops_locked = {
    .name                                       = "DL",
    .get_self                                   = cb_list_element_get_self,
    .get_next                                   = cb_list_element_get_next_locked,
    .get_prev                                   = cb_list_element_get_prev_locked,
    .remove                                     = cb_list_element_remove_locked,
    .insert_after                               = cb_list_element_insert_after_locked,
    .insert_before                              = cb_list_element_insert_before_locked,
};

struct list_element_s LIST_element_initializer = {
    .h                                          = NULL,
    .n                                          = NULL,
    .p                                          = NULL,
    .ops                                        = &default_element_ops,
    .use                                        = 0,
    .lockflags                                  = 0,
};

void LIST_element_init(struct list_element_s *e, struct list_header_s *h)
{

    e->h=h;
    e->n=NULL;
    e->p=NULL;

    if (h) {

	e->ops=((h->flags & LIST_HEADER_FLAG_LOCKBIT) ? &default_element_ops_locked : &default_element_ops);

    } else {

	e->ops=NULL;

    }

    e->use=0;
    e->lockflags=0;

}

/* user functions */

struct list_element_s *LIST_element_get_next(struct list_element_s *e)
{
    return (* e->ops->get_next)(e);
}

struct list_element_s *LIST_element_get_prev(struct list_element_s *e)
{
    return (* e->ops->get_prev)(e);
}

void LIST_element_add_after(struct list_element_s *p, struct list_element_s *e)
{
    (* p->ops->insert_after)(p, e);
}

void LIST_element_add_before(struct list_element_s *n, struct list_element_s *e)
{
    (* n->ops->insert_before)(n, e);
}

void LIST_element_remove(struct list_element_s *e)
{
    (* e->ops->remove)(e);
}

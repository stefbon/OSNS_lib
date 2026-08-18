/*

  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Stef Bon <stefbon@gmail.com>

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

#ifndef _LIB_LIST_LIST_H
#define _LIB_LIST_LIST_H

#include "libosns-error.h"
#include "libosns-time.h"

#define LIST_HEADER_FLAG_INIT                                   1
#define LIST_HEADER_FLAG_LOCKBIT				2

#define LIST_OP_LOCK_TYPE_NEXT                                  1
#define LIST_OP_LOCK_TYPE_PREV                                  2
#define LIST_OP_LOCK_TYPE_SELF                                  4

struct list_header_s;
struct list_element_s;
struct event_shared_signal_s;

struct list_element_ops_s {
    const char                                                  *name;
    struct list_element_s	                                *(* get_self)(struct list_element_s *e);
    struct list_element_s	                                *(* get_next)(struct list_element_s *e);
    struct list_element_s	                                *(* get_prev)(struct list_element_s *e);
    void                                                        (* remove)(struct list_element_s *e);
    void			                                (* insert_after)(struct list_element_s *a, struct list_element_s *e);
    void			                                (* insert_before)(struct list_element_s *b, struct list_element_s *e);
};

struct list_element_s {
    struct list_header_s	                                *h;
    struct list_element_s 	                                *n;
    struct list_element_s 	                                *p;
    struct list_element_ops_s	                                *ops;
    unsigned int                                                use;
    unsigned int                                                lockflags;
};

struct list_header_s {
    unsigned int		                                flags;
    unsigned int                                                lockflags;
    pthread_t                                                   threadid;
    uint64_t			                                count;
    struct event_shared_signal_s                                *esignal;
    struct list_element_s 	                                head;
    struct list_element_s 	                                tail;
};

struct list_element_ops_s *LIST_element_get_ops_tail(unsigned char lock);
struct list_element_ops_s *LIST_element_get_ops_head(unsigned char lock);

extern struct list_element_s LIST_element_initializer;
#define	LIST_ELEMENT_INIT		                        LIST_element_initializer

void LIST_element_init(struct list_element_s *e, struct list_header_s *h);

void LIST_element_add_after(struct list_element_s *p, struct list_element_s *e);
void LIST_element_add_before(struct list_element_s *n, struct list_element_s *e);
void LIST_element_remove(struct list_element_s *e);

struct list_element_s *LIST_element_get_next(struct list_element_s *e);
struct list_element_s *LIST_element_get_prev(struct list_element_s *e);

#endif

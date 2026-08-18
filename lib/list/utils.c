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

struct list_element_s *LIST_search_list_forw(struct list_header_s *h, int (* condition)(struct list_element_s *list, void *ptr), void *ptr)
{
    struct list_element_s *e=LIST_header_get_first(h);

    while (e) {

	if (condition(e, ptr)==1) break;
	e=(* e->ops->get_next)(e);

    }

    return e;
}

struct list_element_s *LIST_search_list_back(struct list_header_s *h, int (* condition)(struct list_element_s *list, void *ptr), void *ptr)
{
    struct list_element_s *e=LIST_header_get_last(h);

    while (e) {

	if (condition(e, ptr)==1) break;
	e=(* e->ops->get_prev)(e);

    }

    return e;
}

unsigned char LIST_element_is_first(struct list_element_s *e)
{
    struct list_header_s *h=e->h;
    return (h && (e->p==&h->head)) ? 1 : 0;
}

unsigned char LIST_element_is_last(struct list_element_s *e)
{
    struct list_header_s *h=e->h;
    return (h && (e->n==&h->tail)) ? 1 : 0;
}

unsigned char LIST_element_is_listed(struct list_element_s *e)
{
    struct list_header_s *h=e->h;
    return (h) ? 1 : 0;
}

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

#ifndef _LIB_LIST_UTILS_H
#define _LIB_LIST_UTILS_H

/* prototypes */

struct list_element_s *LIST_search_list_forw(struct list_header_s *h, int (* condition)(struct list_element_s *list, void *ptr), void *ptr);
struct list_element_s *LIST_search_list_back(struct list_header_s *h, int (* condition)(struct list_element_s *list, void *ptr), void *ptr);

unsigned char LIST_element_is_first(struct list_element_s *e);
unsigned char LIST_element_is_last(struct list_element_s *e);
unsigned char LIST_element_is_listed(struct list_element_s *e);

#endif

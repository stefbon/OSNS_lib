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
#ifndef LIB_LIST_INSERT_H
#define LIB_LIST_INSERT_H

/* prototypes */

void cb_list_element_insert_after_locked(struct list_element_s *p, struct list_element_s *e);
void cb_list_element_insert_after(struct list_element_s *p, struct list_element_s *e);

void cb_list_element_insert_before_locked(struct list_element_s *n, struct list_element_s *e);
void cb_list_element_insert_before(struct list_element_s *n, struct list_element_s *e);

void cb_list_element_insert_null(struct list_element_s *n, struct list_element_s *e);

#endif

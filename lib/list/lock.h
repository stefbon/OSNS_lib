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
#ifndef _LIB_LIST_LOCK_H
#define _LIB_LIST_LOCK_H

#include "list.h"

/* prototypes */

unsigned char LIST_header_lock(struct list_header_s *h, unsigned int lockflag);
void LIST_header_unlock(struct list_header_s *h, unsigned int lockflag);

unsigned char LIST_element_lock_op(struct list_header_s *h, struct list_element_s *e, unsigned int lockflag);
void LIST_element_unlock_op(struct list_header_s *h, struct list_element_s *e, unsigned int lockflag);

unsigned char LIST_header_set_read_lock(struct list_header_s *h, struct timespec_s *timeout);
unsigned char LIST_header_unset_read_lock(struct list_header_s *h);
unsigned char LIST_header_upgrade_read_lock(struct list_header_s *h, struct timespec_s *timeout);
unsigned char LIST_header_set_write_lock(struct list_header_s *h, struct timespec_s *timeout);
unsigned char LIST_header_unset_write_lock(struct list_header_s *h);

#endif

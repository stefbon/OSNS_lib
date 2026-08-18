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

#ifndef _LIB_IO_BACKEND_H
#define _LIB_IO_BACKEND_H

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-misc.h"
#include "libosns-error.h"

/* Prototypes */

struct io_object_backend_s;

void IO_object_backend_init(struct io_object_backend_s *b, unsigned char type);

unsigned char IO_object_backend_close(struct io_object_backend_s *b);
unsigned char IO_object_backend_is_open(struct io_object_backend_s *b);

void IO_object_backend_copy(struct io_object_backend_s *ba, struct io_object_backend_s *bb);

unsigned char IO_object_backend_get_error(struct io_object_backend_s *b, struct error_s *error);
unsigned char IO_object_backend_set_non_blocking(struct io_object_backend_s *b, unsigned char enable);
unsigned char IO_object_backend_set_cloexec(struct io_object_backend_s *b, unsigned char enable);

int IO_object_backend_get_unix_fd(struct io_object_backend_s *b);
unsigned char IO_object_backend_set_unix_fd(struct io_object_backend_s *b, int fd);

#endif

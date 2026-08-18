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

#ifndef LIB_EVENT_LOCK_H
#define LIB_EVENT_LOCK_H

#include "signal.h"

/* prototypes */

unsigned char EVENT_signal_lock_flag(struct event_shared_signal_s *signal, unsigned int *p_flags, unsigned int flag);
void EVENT_signal_unlock_flag(struct event_shared_signal_s *signal, unsigned int *p_flags, unsigned int flag);

unsigned char EVENT_signal_unlock_wait(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag, struct timespec_s *expire, struct error_s *error);

#endif

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

#ifndef _LIB_EVENT_SET_H
#define _LIB_EVENT_SET_H

#include "signal.h"

/* prototypes */

unsigned int EVENT_signal_set_flag(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag);
unsigned int EVENT_signal_unset_flag(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag);

int EVENT_signal_wait_flag_set(struct event_shared_signal_s *signal, unsigned int *p_flags, unsigned int flag, struct timespec_s *expire);
int EVENT_signal_wait_flag_unset(struct event_shared_signal_s *signal, unsigned int *p_flags, unsigned int flag, struct timespec_s *expire);

#endif

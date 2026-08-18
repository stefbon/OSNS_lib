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

#include "libosns-basic-system-headers.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-event.h"

#include "signal.h"

/* set a flag, if already set wait for it to be unset */

unsigned char EVENT_signal_lock_flag(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag)
{
    EVENT_signal_lock(esignal);
    while (*p_flags & flag) { int result=EVENT_signal_wait(esignal, NULL);};
    *p_flags |= flag;
    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);
    return 1;
}

/* unset a flag */

void EVENT_signal_unlock_flag(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag)
{
    EVENT_signal_lock(esignal);
    *p_flags &= ~flag;
    EVENT_signal_broadcast(esignal);
    EVENT_signal_unlock(esignal);
}

/* wait for unlocking of flag to check a change */

unsigned char EVENT_signal_unlock_wait(struct event_shared_signal_s *esignal, unsigned int *p_flags, unsigned int flag, struct timespec_s *expire, struct error_s *error)
{
    unsigned char status=0;

    EVENT_signal_lock(esignal);

    status=(*p_flags & flag);

    check:

    /* stop if status==1 and flag in p_flags is unset */

    if (((status==0) || (*p_flags & flag)) && (ERROR_get_errnum(error)==0)) {

	if (EVENT_signal_wait(esignal, expire)) {

	    status=0; /* timedout: no success */
	    goto unlock;

	}

	status=(*p_flags & flag);
	goto check;

    } else {

	status=(ERROR_get_errnum(error)==0) ? 1 : 0;

    }

    unlock:
    EVENT_signal_unlock(esignal);
    return status;
}

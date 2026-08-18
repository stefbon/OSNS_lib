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

#ifndef LIB_SYSTEM_SIGNAL_H
#define LIB_SYSTEM_SIGNAL_H

#include "libosns-eventloop.h"

#define SYSTEM_SIGNAL_CODE_ABRT				1
#define SYSTEM_SIGNAL_CODE_ALRM				2
#define SYSTEM_SIGNAL_CODE_FPE				3
#define SYSTEM_SIGNAL_CODE_HUP				4
#define SYSTEM_SIGNAL_CODE_ILL				5
#define SYSTEM_SIGNAL_CODE_INT				6
#define SYSTEM_SIGNAL_CODE_PIPE				7
#define SYSTEM_SIGNAL_CODE_QUIT				8
#define SYSTEM_SIGNAL_CODE_SEGV				9
#define SYSTEM_SIGNAL_CODE_TERM				10
#define SYSTEM_SIGNAL_CODE_USR1				11
#define SYSTEM_SIGNAL_CODE_USR2				12

union system_signal_type_u {
    struct signal_type_io_s {
	uint32_t			fd;
	uint32_t			events;
    } io;
    struct signal_type_chld_s {
	uint32_t			uid;
	int                             code;
	int				status;
	uint64_t			utime;
	uint64_t			stime;
    } chld;
    struct signal_type_kill_s {
	uint32_t			uid;
    } kill;
    struct signal_type_usr_s {
	uint32_t			uid;
    } usr;
};

/* Prototypes */

int SYSTEM_signal_monitor_start(struct beventloop_s *eloop, struct event_shared_signal_s *esignal, void (* system_signal_event_cb)(unsigned int signo, pid_t pid, union system_signal_type_u *type, void *ptr), void *ptr);
void SYSTEM_signal_monitor_stop();

#endif

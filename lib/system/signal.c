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

#include <sys/signalfd.h>

#include "libosns-main.h"
#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-threads.h"
#include "libosns-eventloop.h"
#include "libosns-event.h"

#include "signal.h"

static unsigned char init_done=0;

#define SYSTEM_SIGNAL_MONITOR_STATUS_THREAD                             1

struct system_signal_monitor_s {
    unsigned int                                                        status;
    struct event_shared_signal_s                                        *esignal;
    struct list_header_s                                                header;
    struct io_object_s                                                  object;
    struct bevent_ctx_s                                                 bctx;
    void                                                                (* system_signal_event_cb)(unsigned int signo, pid_t pid, union system_signal_type_u *type, void *ptr);
    void                                                                *ptr;
};

/* default cb's for various system signals ... they do nothing */

static void system_signal_event_cb_default(unsigned int signo, pid_t pid, union system_signal_type_u *type, void *ptr)
{}

static struct system_signal_monitor_s monitor;

struct system_signal_event_s {
    struct list_element_s                                               list;
    uint32_t				                                signo;
    uint32_t			                                        pid;
    union system_signal_type_u                                          type;
};

/* size of receive buffer ... under Linux the size of one signalfd siginfo is 128 ... set buffer size to 8 times ... arbitrary */

static unsigned int maxcount_events=128;

static struct system_signal_event_s *SYSTEM_signal_monitor_get_event()
{
    struct list_element_s *list=LIST_header_remove_first(&monitor.header);
    return (list) ? (struct system_signal_event_s *)((char *) list - offsetof(struct system_signal_event_s, list)) : NULL;
}

static unsigned char SYSTEM_signal_monitor_remove_first()
{
    struct system_signal_event_s *sse=SYSTEM_signal_monitor_get_event();
    unsigned char ssefound=(sse) ? 1 : 0;

    free(sse);
    return ssefound;
}

static void system_signal_process_events_thread(void *ptr)
{
    struct system_signal_event_s *sse=SYSTEM_signal_monitor_get_event();
    struct event_shared_signal_s *esignal=monitor.esignal;

    if (sse==NULL) return;

    processsystemevents:

    (* monitor.system_signal_event_cb)(sse->signo, sse->pid, &sse->type, monitor.ptr);
    free(sse);

    EVENT_signal_lock(esignal);

    sse=SYSTEM_signal_monitor_get_event();

    if (sse) {

        EVENT_signal_unlock(esignal);
        goto processsystemevents;

    }

    monitor.status &= ~SYSTEM_SIGNAL_MONITOR_STATUS_THREAD;
    EVENT_signal_unlock(esignal);

}

/* handle events from the events queue ... by default start a thread */

static void system_signal_event_handler()
{
    struct event_shared_signal_s *esignal=monitor.esignal;
    unsigned int dostartthread=0;

    EVENT_signal_lock(esignal);

    if ((monitor.status & SYSTEM_SIGNAL_MONITOR_STATUS_THREAD)==0) {

        dostartthread=1;
        monitor.status |= SYSTEM_SIGNAL_MONITOR_STATUS_THREAD;

    }

    EVENT_signal_unlock(esignal);

    if (dostartthread) LOCAL_threads_put_job(0, system_signal_process_events_thread, NULL);

}

static struct system_signal_event_s *system_signal_event_create(struct signalfd_siginfo *fdsi)
{
    struct system_signal_event_s *sse=malloc(sizeof(struct system_signal_event_s));

    if (sse) {

	memset(sse, 0, sizeof(struct system_signal_event_s));
        LIST_element_init(&sse->list, NULL);
        sse->pid=fdsi->ssi_pid;
        sse->signo=fdsi->ssi_signo;

	switch (fdsi->ssi_signo) {

	    case SIGHUP:
	    case SIGTERM:
	    case SIGINT:
	    case SIGSTOP:
	    case SIGABRT:
	    case SIGQUIT:

                sse->type.kill.uid=fdsi->ssi_uid;
		break;

	    case SIGUSR1:
	    case SIGUSR2:

                sse->type.usr.uid=fdsi->ssi_uid;
		break;

	    case SIGIO:

                sse->type.io.fd=fdsi->ssi_fd;
	        sse->type.io.events=fdsi->ssi_band;
                break;

	    case SIGCHLD:

	        sse->type.chld.uid=fdsi->ssi_uid;
                sse->type.chld.code=fdsi->ssi_code;
		sse->type.chld.status=fdsi->ssi_status;
		sse->type.chld.utime=fdsi->ssi_utime;
		sse->type.chld.stime=fdsi->ssi_stime;
		break;

	    default:

		logoutput_debug("%s: signo %u not supported", __FUNCTION__, fdsi->ssi_signo);

	}

        if (monitor.header.count > maxcount_events) {

            unsigned char tmp=SYSTEM_signal_monitor_remove_first();

        }

        LIST_header_add_last(&monitor.header, &sse->list);
        system_signal_event_handler();

    }

    return sse;

}

/* eventloop event cb's */

static void SYSTEM_signal_io_read_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    int bytesread=-1;

#ifdef __linux__
    int fd=IO_object_get_unix_fd(&monitor.object);
    struct signalfd_siginfo fdsi[8];

    readfdsi:

    bytesread=read(fd, &fdsi, 8 * sizeof(struct signalfd_siginfo));

    if (bytesread > 0) {
        unsigned int count=(bytesread / sizeof(struct signalfd_siginfo));

        for (unsigned int i=0; i<count; i++) {

            system_signal_event_create(&fdsi[i]);

        }

        if (count==8) goto readfdsi;

    } else if (bytesread==-1) {

        logoutput_debug("%s: errcode %u fd %u", __FUNCTION__, errno, IO_object_get_unix_fd(&monitor.object));

    }

#else

    errno=ENOSYS;

#endif

}

static void SYSTEM_signal_io_close_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    logoutput_debug("%s", __FUNCTION__);
}

static void SYSTEM_signal_io_error_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    logoutput_debug("%s", __FUNCTION__);
}

/* main functions */

int SYSTEM_signal_monitor_start(struct beventloop_s *eloop, struct event_shared_signal_s *esignal, void (* system_signal_event_cb)(unsigned int signo, pid_t pid, union system_signal_type_u *type, void *ptr), void *ptr)
{
    int result=-1;
    struct event_shared_signal_s *global_esignal=EVENT_signal_get_default();

    EVENT_signal_lock(global_esignal);

    if (init_done) {

        logoutput_debug("%s: already initialized", __FUNCTION__);
        EVENT_signal_unlock(global_esignal);
        return 0;

    }

    init_done=1;
    EVENT_signal_unlock(global_esignal);

    logoutput_debug("%s: mount monitor initialized", __FUNCTION__);

    monitor.status = 0;
    LIST_header_init(&monitor.header, 0);

    IO_object_init(&monitor.object, IO_OBJECT_TYPE_SYSTEM);
    BEVENT_ctx_init(&monitor.bctx, monitor.object.backend, 0);

    monitor.system_signal_event_cb=(system_signal_event_cb) ? system_signal_event_cb : system_signal_event_cb_default;
    monitor.esignal=((esignal) ? esignal : EVENT_signal_get_default());
    monitor.ptr=ptr;

#ifdef __linux__

    sigset_t sigset;

    if (sigfillset(&sigset)==-1) {

        logoutput_debug("%s: unable to fill signal set ... errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));
        goto out;

    }

    if (sigprocmask(SIG_SETMASK, &sigset, NULL)==0) {
        int fd=-1;

        fd = signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);

        if (fd>=0) {

            logoutput_debug("%s: signal monitor started .. (fd=%i)", __FUNCTION__, fd);

        } else {

            logoutput_debug("%s: unable to start signal monitor ... errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));

        }

        IO_object_set_unix_fd(&monitor.object, fd);

    } else {

        logoutput_debug("%s: unable to set defaults for signal monitor ... errcode=%u (%s)", __FUNCTION__, errno, strerror(errno));

    }

#endif

    if (IO_object_is_open(&monitor.object)) {
        struct bevent_ctx_s *bctx=&monitor.bctx;

        if (BEVENT_ctx_attach_to_eventloop(NULL, bctx)>=0) {

            /* set cb's */

            BEVENT_ctx_set_cb(bctx, (BEVENT_EVENT_BIT_READABLE | BEVENT_EVENT_BIT_PRI), SYSTEM_signal_io_read_event);
            BEVENT_ctx_set_cb(bctx, BEVENT_EVENT_BIT_CLOSE, SYSTEM_signal_io_close_event);
            BEVENT_ctx_set_cb(bctx, BEVENT_EVENT_BIT_ERROR, SYSTEM_signal_io_error_event);

            /* add */

            int tmp=BEVENT_ctx_add(NULL, bctx, 0, 0);
            result=1;

        }

    }

    out:

    if (result!=1) {

        if (IO_object_close(&monitor.object)) logoutput_debug("%s: io backend closed", __FUNCTION__);

    }

    return result;

}

void SYSTEM_signal_monitor_stop()
{

    struct event_shared_signal_s *global_esignal=EVENT_signal_get_default();

    EVENT_signal_lock(global_esignal);

    if (init_done==1) {

        BEVENT_ctx_detach(&monitor.bctx);
        if (IO_object_close(&monitor.object)) logoutput_debug("%s: io backend closed", __FUNCTION__);

        /* remove and free any signal events on the queue */

        while (SYSTEM_signal_monitor_remove_first()) {};

    }

    EVENT_signal_unlock(global_esignal);

}

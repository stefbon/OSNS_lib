/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-eventloop.h"
#include "libosns-threads.h"
#include "libosns-event.h"

#include "loop.h"
#include "bevent.h"
#include "queue.h"

#ifdef __linux__

#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>

#define EPOLL_EVENT_BUFFER_LENGTH               32

struct system_event_values_s {
    uint32_t						events[BEVENT_EVENT_INDEX_COUNT];
    uint32_t						modes[BEVENT_MODE_INDEX_COUNT];
};

static struct system_event_values_s system_values;

void BACKEND_fill_system_event_values()
{

    system_values.events[BEVENT_EVENT_INDEX_READABLE]=EPOLLIN;
    system_values.events[BEVENT_EVENT_INDEX_WRITEABLE]=EPOLLOUT;
    system_values.events[BEVENT_EVENT_INDEX_ERROR]=EPOLLERR;
    system_values.events[BEVENT_EVENT_INDEX_CLOSE]=(EPOLLHUP | EPOLLRDHUP);
    system_values.events[BEVENT_EVENT_INDEX_PRI]=EPOLLPRI;

    system_values.modes[BEVENT_MODE_INDEX_EDGE]=EPOLLET;
    system_values.modes[BEVENT_MODE_INDEX_ONESHOT]=EPOLLONESHOT;

}

uint32_t BACKEND_get_system_event_value(unsigned int index)
{
    return (index < BEVENT_EVENT_INDEX_COUNT ? system_values.events[index] : 0);
}

uint32_t BACKEND_get_system_event_value_raw(unsigned int index)
{
    return system_values.events[index];
}

uint32_t BACKEND_get_system_mode_value(unsigned int index)
{
    return (index < BEVENT_MODE_INDEX_COUNT ? system_values.modes[index] : 0);
}

uint32_t BACKEND_get_system_mode_value_raw(unsigned int index)
{
    return system_values.modes[index];
}

static int BCTX_backend_epoll_ctl(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, int epoll_opcode)
{
    struct io_object_backend_s *io_backend=((bctx) ? bctx->io_backend : NULL);
    int fd=(io_backend ? IO_object_backend_get_unix_fd(io_backend) : -1);
    int result=-1;

    if (fd==-1) {

        logoutput_warning("%s: unix fd not set (already closed?)", __FUNCTION__);
        return -1;

    }

    logoutput_debug("%s: add unix fd %u to eventloop", __FUNCTION__, fd);

    if (eloop==NULL) eloop=BEVENTLOOP_get_default_loop();

    if (epoll_opcode==EPOLL_CTL_DEL) {

	/* last argument is not required when deleting */

	result=epoll_ctl(eloop->backend.epoll.fd, EPOLL_CTL_DEL, fd, NULL);

    } else {
	struct bevent_s *bevent=NULL;
	struct epoll_event epev;

	if (bctx->bevent==NULL) {

	    logoutput_warning("%s: unable to add/mod fd %i to eloop ... not attached or already closed", __FUNCTION__, fd);
	    return -1;

	}

	bevent=bctx->bevent;
	epev.events=(bevent->events.events | bevent->events.mode);
	epev.data.ptr=(void *) bevent;
	result=epoll_ctl(eloop->backend.epoll.fd, epoll_opcode, fd, &epev);

    }

    if (result==-1) {

	logoutput_debug("%s: fd %u error %u (%s)", __FUNCTION__, (unsigned int) fd, errno, strerror(errno));
	return -1;

    }

    logoutput_debug("%s: fd %u opcode %u", __FUNCTION__, (unsigned int) fd, epoll_opcode);
    return 0;

}

static void BEVENT_ctx_set_mode(struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how)
{

    mode &= BEVENT_MODE_ALL_BITS;

    if (bctx->bevent) {

	if (how==BEVENT_CTX_MODE_SET) {

	    bctx->bevent->events.mode=mode;

	} else if (how==BEVENT_CTX_MODE_DISABLE) {

	    bctx->bevent->events.mode&=~mode;

	} else if (how==BEVENT_CTX_MODE_ENABLE) {

	    bctx->bevent->events.mode|=mode;

	} else {

	    logoutput_debug("%s: how opcode %u not supported", __FUNCTION__, how);

	}

    }

}

int BEVENT_ctx_add(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how)
{

    if (mode==0) how=BEVENT_CTX_MODE_SET;
    BEVENT_ctx_set_mode(bctx, mode, how);

    return BCTX_backend_epoll_ctl(eloop, bctx, EPOLL_CTL_ADD);
}

int BEVENT_ctx_mod(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how)
{

    if (mode==0) how=BEVENT_CTX_MODE_SET;
    BEVENT_ctx_set_mode(bctx, mode, how);

    return BCTX_backend_epoll_ctl(eloop, bctx, EPOLL_CTL_MOD);
}

int BEVENT_ctx_del(struct beventloop_s *eloop, struct bevent_ctx_s *bctx)
{
    return BCTX_backend_epoll_ctl(eloop, bctx, EPOLL_CTL_DEL);
}

void BEVENTLOOP_backend_init(struct beventloop_s *eloop)
{
    eloop->flags |= BEVENTLOOP_FLAG_EPOLL;
    eloop->backend.epoll.fd=-1;

    /* open epoll */

    eloop->backend.epoll.fd=epoll_create(EPOLL_CLOEXEC);

    if (eloop->backend.epoll.fd==-1) {

	logoutput_debug("%s: error %u (%s) calling epoll_create", __FUNCTION__, errno, strerror(errno));
	return;

    }

    logoutput_debug("%s: using epoll fd %u", __FUNCTION__, eloop->backend.epoll.fd);

}

int BEVENTLOOP_backend_start(struct beventloop_s *eloop)
{
    struct epoll_event aevents[EPOLL_EVENT_BUFFER_LENGTH];
    int result=0;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);

    EVENT_signal_set_flag(eloop->esignal, &eloop->flags, BEVENTLOOP_FLAG_START);

    while ((eloop->flags & BEVENTLOOP_FLAG_STOP)==0) {

	int tmp=epoll_pwait(eloop->backend.epoll.fd, aevents, EPOLL_EVENT_BUFFER_LENGTH, -1, &mask);

	if (tmp>0) {

	    for (unsigned int i=0; i<tmp; i++) {
		struct bevent_s *bevent=(struct bevent_s *) aevents[i].data.ptr;

        	/* only process events not already reported before */

        	aevents[i].events &= ~bevent->revents.events;

        	if (aevents[i].events) {

            	    bevent->revents.events |= aevents[i].events;
            	    if (BEVENT_bevent_queue_events(eloop, bevent)) BEVENT_process_events(bevent);

        	}

	    }

	} else if (tmp==0) {

	    logoutput_debug("%s: eventloop woken upp (signal?)", __FUNCTION__);

	} else if (tmp==-1) {

	    logoutput_debug("%s: error %u (%s) calling epoll_wait", __FUNCTION__, errno, strerror(errno));
	    result=-1;
	    break;

	}

    }

    EVENT_signal_unset_flag(eloop->esignal, &eloop->flags, BEVENTLOOP_FLAG_START);
    return result;

}

void BEVENTLOOP_backend_close(struct beventloop_s *eloop)
{

    if (eloop->backend.epoll.fd>=0) {

	close(eloop->backend.epoll.fd);
	eloop->backend.epoll.fd=-1;

    }

}

#else

int BEVENT_ctx_add(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how);
{
    return -1;
}

int BEVENT_ctx_mod(struct beventloop_s *eloop, struct bevent_ctx_s *bctx, unsigned int mode, unsigned int how)
{
    return -1;
}

int BEVENT_ctx_del(struct beventloop_s *eloop, struct bevent_ctx_s *bctx)
{
    return -1;
}

void BEVENTLOOP_backend_init(struct beventloop_s *eloop)
{}

int BEVENTLOOP_backend_start(struct beventloop_s *eloop)
{
    return -1;
}

void BEVENTLOOP_backend_close(struct beventloop_s *eloop)
{}

#endif

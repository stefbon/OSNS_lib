/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include <sys/sysmacros.h>
#include <fcntl.h>

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-eventloop.h"
#include "libosns-event.h"

#include "monitor.h"
#include "utils.h"
#include "read.h"
#include "export.h"

#ifdef __linux__
#define MOUNTINFO_FILE "/proc/self/mountinfo"
#endif

static struct mount_monitor_s standard_monitor;
static unsigned int refcount=0;

/* dummy callbacks */

static void dummy_change_cb(unsigned char added, struct mountinfo_export_s *me, void *ptr)
{
    logoutput_debug("dummy_update: %s %.*s %.*s at %.*s", (added ? "added" : "removed"), me->fields[MOUNTINFO_FIELD_SOURCE].length, me->fields[MOUNTINFO_FIELD_SOURCE].str, me->fields[MOUNTINFO_FIELD_FS].length, me->fields[MOUNTINFO_FIELD_FS].str, me->fields[MOUNTINFO_FIELD_MOUNTPOINT].length, me->fields[MOUNTINFO_FIELD_MOUNTPOINT].str);
}

void MOUNTMONITOR_init(struct event_shared_signal_s *esignal, unsigned int mask_added, unsigned int mask_removed, unsigned int flags, void *ptr)
{
    struct event_shared_signal_s *global_esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(global_esignal)) {

	refcount++;

	if (refcount>1) {

	    EVENT_signal_unlock(global_esignal);
	    return;

	}

	EVENT_signal_unlock(global_esignal);

    }

    standard_monitor.status=MOUNT_MONITOR_STATUS_FLAG_INIT;
    standard_monitor.thread=0;
    standard_monitor.flags=(flags & MOUNT_MONITOR_FLAG_IGNORE_SYSTEM_FS);
    standard_monitor.mask_added=(mask_added ? mask_added : MOUNTINFO_EXPORT_MASK_ALL);
    standard_monitor.mask_removed=(mask_removed ? mask_removed : MOUNTINFO_EXPORT_MASK_ALL);
    standard_monitor.generation=0;
    standard_monitor.esignal=esignal ? esignal : EVENT_signal_get_default();

    standard_monitor.cb=dummy_change_cb;

    FS_object_init(&standard_monitor.fso);
    BEVENT_ctx_init(&standard_monitor.bctx, &standard_monitor.fso.backend, 0);

    LIST_header_init(&standard_monitor.mountlines, 0);

    standard_monitor.current=NULL;
    standard_monitor.previous=NULL;
    IO_buffer_init(&standard_monitor.buffer[0]);
    IO_buffer_init(&standard_monitor.buffer[1]);

}

int MOUNTMONITOR_open()
{
    int fd=-1;

#ifdef __linux__

    fd=open(MOUNTINFO_FILE, O_RDONLY);

    if (fd>=0) {

        logoutput_debug("%s: open file %s (unix fd %u)", __FUNCTION__, MOUNTINFO_FILE, (unsigned int)fd);
        FS_object_set_unix_fd(&standard_monitor.fso, fd);

    }

#endif

    return fd;

}

void MOUNTMONITOR_close()
{

    if (FS_object_close(&standard_monitor.fso)) {

	logoutput_debug("%s: close monitor", __FUNCTION__);

    }

}

void MOUNTMONITOR_set_cb(void (* cb)(unsigned char added, struct mountinfo_export_s *me, void *ptr))
{
    standard_monitor.cb=(cb ? cb : dummy_change_cb);
}

/* io event cb's */

static void MOUNTINFO_io_read_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    struct mount_monitor_s *mm=(struct mount_monitor_s *)((char *) bctx - offsetof(struct mount_monitor_s, bctx));

    MOUNTINFO_read_mount_table(mm);
}

static void MOUNTINFO_io_close_event(struct bevent_ctx_s *bctx, struct bevent_argument_s *arg)
{
    logoutput_debug("%s", __FUNCTION__);
}

int MOUNTMONITOR_add_to_eventloop(struct beventloop_s *loop)
{
    int result=-1;
    struct bevent_ctx_s *bctx=&standard_monitor.bctx;

    if (BEVENT_ctx_attach_to_eventloop(loop, bctx)) {

        /* set cb's */

        BEVENT_ctx_set_cb(bctx, (BEVENT_EVENT_BIT_PRI | BEVENT_EVENT_BIT_ERROR), MOUNTINFO_io_read_event);
	BEVENT_ctx_set_cb(bctx, BEVENT_EVENT_BIT_CLOSE, MOUNTINFO_io_close_event);

        /* add */

	BEVENT_ctx_add(NULL, bctx, 0, 0);
        result=1;

    }

    return result;

}

/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include <sys/sysmacros.h>
#include <fcntl.h>

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-eventloop.h"
#include "libosns-threads.h"
#include "libosns-event.h"

#include "monitor.h"
#include "utils.h"
#include "read.h"
#include "export.h"

#ifdef __linux__
#define MOUNTINFO_FILE "/proc/self/mountinfo"
#endif

extern unsigned char mountinfo_export_mount_worker(struct mount_monitor_s *monitor, struct mountinfo_list_s *mlist, unsigned int mask, struct mountinfo_export_s *mexport);

/* read mountid and parentid from data
    (this is only the first part of the data/line) */

static unsigned int dstr_read_mountid(struct mountinfo_line_s *ml, struct dstr_s *data)
{
    char buffer[64];
    unsigned int length=0;

    /* with scanf and relatives there is no way to determine the position in the read buffer after a successfull read
	therefore I use the snprintf to reproduce the exact same first part of the buffer
	and snprintf gives a size == position in read buffer
	..... it's also possible to look for the first starting slash ... */

    if (sscanf(data->str, "%u %u %u:%u", &ml->mountid, &ml->parentid, &ml->major, &ml->minor) != 4) {

        logoutput_error("%s: error %u sscanf", __FUNCTION__, errno);
	return 0;

    }

    length = (unsigned int) snprintf(buffer, 64, "%u %u %u:%u ", ml->mountid, ml->parentid, ml->major, ml->minor);
    logoutput_debug("%s: found mountid %s", __FUNCTION__, buffer);
    DSTR_shift_raw(data, length);
    return length;

}

/* utility to read the next string delimited by a space from data */

static unsigned int dstr_read_mountinfo_str(struct mountinfo_line_s *mline, struct dstr_s *data, struct mountinfo_str_s *mstr, unsigned char eol)
{
    unsigned int length=0;
    char *start=data->str;
    char *sep=memchr(start, ' ', data->length);

    if (sep || eol) { /* when eol there is possibly no terminating space */

        length=((sep) ? (sep - start) : data->length);

        mstr->pos=(unsigned int)(start - mline->data.str); /* index in a buffer/array */
        mstr->length=length;
        DSTR_shift_raw(data, length);


    }

    return length;

}

/* utility to change position in data */

static void dstr_goto_pos(struct mountinfo_line_s *mline, struct dstr_s *data, char *pos)
{

    if (pos > data->str) {

        if (pos < (data->str + data->length)) {
            unsigned int bytes2shift=(unsigned int)(pos - data->str);

            DSTR_shift_raw(data, bytes2shift);

        }

    } else if (pos < data->str) {

        if (pos >= mline->data.str) {
            unsigned int bytes2shift=(unsigned int)(data->str - pos);

            /* no DSTR function to do a shift back */

            data->str -= bytes2shift;
            data->length += bytes2shift;

        }

    }

}

/* utility which detect spaces to skip */

unsigned int dstr_skip_spaces(struct dstr_s *data)
{
    unsigned int ctr=0;

    while (data->length && isspace(*data->str)) {

        data->str++;
        data->length--;
        ctr++;

    }

    return ctr;

}

/* read the rest of the fields like root, mountpoint etc from data */

static unsigned char mountinfo_process_mountinfo_line(struct mountinfo_line_s *mline, struct dstr_s *data)
{
    char *sep=NULL;
    unsigned char result=0;
    unsigned int count=0;

    /* read the first three fields:
        root, mount path, options  */

    for (unsigned int i=0; i<3; i++) {

        if (dstr_read_mountinfo_str(mline, data, &mline->fields[i], 0)==0) {

	    logoutput_error("%s: unable to read field %u", __FUNCTION__, i);
	    goto out;

        }

        /*logoutput_debug("%s: i %u mstr pos %u length %u", __FUNCTION__, i, mline->fields[i].pos, mline->fields[i]. length);*/
        count=dstr_skip_spaces(data);

    }

    /* optional */

    sep=memchr(data->str, '-', data->length);

    if (sep==NULL) {

        logoutput_error("%s: the '-' seperator not found ... cannot continue", __FUNCTION__);
        goto out;

    }

    if (sep > data->str) {

        /* there are optional options */

        if (dstr_read_mountinfo_str(mline, data, &mline->fields[MOUNTINFO_FIELD_OPTIONAL], 0)==0) {

	    logoutput_error("%s: unable to read optional parameters", __FUNCTION__);
	    goto out;

        }

    }

    /* shift to one position right of the "-" seperator */

    dstr_goto_pos(mline, data, sep + 1);
    count=dstr_skip_spaces(data);

    /* read the rest:
        filesystem, source */

    for (unsigned int i=4; i<6; i++) {

        if (dstr_read_mountinfo_str(mline, data, &mline->fields[i], 0)==0) {

	    logoutput_error("%s: unable to read filesystem", __FUNCTION__);
	    goto out;

        }

        // logoutput_debug("%s: i %u mstr pos %u length %u", __FUNCTION__, i, mline->fields[i].pos, mline->fields[i]. length);
        count=dstr_skip_spaces(data);

    }

    /* super options (and note: last option so eol is set) */

    if (dstr_read_mountinfo_str(mline, data, &mline->fields[MOUNTINFO_FIELD_SUPEROPTIONS], 1)==0) {

	logoutput_error("%s: unable to read super options", __FUNCTION__);
	goto out;

    }

    if (mountinfo_filesystem_is_system_related(mline)) mline->flags |= MOUNTINFO_LINE_FLAG_SYSTEM;
    result=1;    /* when here: success */

    out:
    return result;
}

static void mountinfo_monitor_process_change_shared(struct mount_monitor_s *monitor, struct mountinfo_list_s *mlist, unsigned char mask, unsigned char new)
{
    struct mountinfo_export_s mexport;

    mountinfo_export_init(&mexport);
    if (mountinfo_export_mount_worker(monitor, mlist, mask, &mexport)) (* monitor->cb)(new, &mexport, monitor->ptr);
    mountinfo_export_clear(&mexport);

}

static void mountinfo_monitor_process_remove(struct mount_monitor_s *monitor, struct mountinfo_list_s *mlist)
{

    mountinfo_monitor_process_change_shared(monitor, mlist, monitor->mask_removed, 0);
    LIST_element_remove(&mlist->list);
    free(mlist);

}

static void mountinfo_monitor_process_add(struct mount_monitor_s *monitor, struct mountinfo_list_s *mlist)
{
    mountinfo_monitor_process_change_shared(monitor, mlist, monitor->mask_added, 1);
}

static void mountinfo_monitor_add_mline(struct mount_monitor_s *monitor, struct mountinfo_line_s *mline, struct list_element_s *list)
{
    struct mountinfo_list_s *mlist2add=NULL;

    if (mountinfo_list_create(mline, &mlist2add, monitor->generation)) {

        if (list) {

            LIST_element_add_before(list, &mlist2add->list);

        } else {

            LIST_header_add_last(&monitor->mountlines, &mlist2add->list);

        }

        if (((monitor->flags & MOUNT_MONITOR_FLAG_IGNORE_SYSTEM_FS)==0) || ((mline->flags & MOUNTINFO_LINE_FLAG_SYSTEM)==0)) mountinfo_monitor_process_add(monitor, mlist2add);

    } else {

        logoutput_debug("%s: unable to create a mountinfo list element ... ", __FUNCTION__);

    }

}

static unsigned char mountinfo_read_line_from_current_without_previous(struct mountinfo_data_s *mdata, struct mountinfo_line_s *mline, int *p_diff)
{
    struct dstr_s data=DSTR_INIT;

    if (mountinfo_data_read_line(mdata, mline)==0) {

        logoutput_debug("%s: unable to read line ... cannot continue", __FUNCTION__);
        return 0;

    }

    DSTR_set_str_raw(&data, &mline->data, 0); /* use a temporary dstr to read the fields (to safely move through the line) */

    if (dstr_read_mountid(mline, &data)==0) {

        logoutput_debug("%s: unable to read mountid ... ignoring", __FUNCTION__);
        return 0;

    } else if (mountinfo_process_mountinfo_line(mline, &data)==0) {

        logoutput_debug("%s: unable to read line data ... ignoring", __FUNCTION__);
        return 0;

    }

    *p_diff=1;
    return 1;
}


static unsigned char mountinfo_read_line_from_current(struct mountinfo_data_s *mdata, struct mountinfo_line_s *mline, struct mountinfo_list_s *mlist, int *p_diff)
{
    struct dstr_s data=DSTR_INIT;

    if (mountinfo_data_read_line(mdata, mline)==0) {

        logoutput_debug("%s: unable to read line ... cannot continue", __FUNCTION__);
        return 0;

    }

    DSTR_set_str_raw(&data, &mline->data, 0); /* use a temporary dstr to read the fields (to safely move through the line) */

    if (DSTR_cmp_str(&mlist->line.data, &data, 1, 0, 0)) {

        *p_diff=0;

    } else {

        if (dstr_read_mountid(mline, &data)==0) {

            logoutput_debug("%s: unable to read mountid ... ignoring", __FUNCTION__);
            return 0;

        } else if (mountinfo_process_mountinfo_line(mline, &data)==0) {

            logoutput_debug("%s: unable to read line data ... ignoring", __FUNCTION__);
            return 0;

        }

        logoutput_debug("%s: found line with length %u with mountid %u", __FUNCTION__, mline->data.length, mline->mountid);

        /* not the same ... why ... find out here */

        if (mlist->line.mountid < mline->mountid) {

            *p_diff=-1;

        } else if (mlist->line.mountid==mline->mountid) {

            *p_diff=0;

        } else {

            *p_diff=1;

        }

    }

    return 1;

}

int MOUNTINFO_io_read_mountinfo_from_system(struct mount_monitor_s *monitor)
{
    int result=-1;
    unsigned char buffer_index=0;
    struct io_buffer_s *mib=NULL;

    /* switch between the two available buffers */

    if (monitor->current==NULL) {

        /* at init */
        monitor->current=&monitor->buffer[0];

    }

    mib=monitor->current;


#ifdef __linux__

    int bytesread=0;
    unsigned int size=512;
    FILE *mfile=NULL;

    /* create a temporary FILE stream to read the mountinfo file */

    mfile=fopen(MOUNTINFO_FILE, "r");

    if (mfile==NULL) {

        logoutput_debug("%s: unable to fopen linux %s ... errcode %u (%s)", __FUNCTION__, MOUNTINFO_FILE, errno, strerror(errno));
        return -1;

    }

    allocatebuffer:

    if (mib->size<size) {

        if (IO_buffer_allocate(mib, size)<=0) {

            logoutput_debug("%s: unable to allocate %u bytes", __FUNCTION__, size);
            goto outclose;

        }

    }

    readmountinfo:

    bytesread=(int) fread(mib->ptr, 1, mib->size, mfile);

    if (bytesread==-1) {

        logoutput_debug("%s: unable to read bytes from %s ... errcode %u (%s)", __FUNCTION__, MOUNTINFO_FILE, errno, strerror(errno));
        mib->bytesread=0;

    } else if (bytesread==0) {

        logoutput_debug("%s: zero bytes read from %s", __FUNCTION__, MOUNTINFO_FILE);
        mib->bytesread=0;
        result=0;

    } else {

        if (bytesread==mib->size) {

            /* possibly data truncated due the buffer size is too small ... */

            size = mib->size + 512;
            rewind(mfile); /* reset read pointer/offset to beginning to start reading over again ... otherwise the next read will start where it left */
            goto allocatebuffer;

        }

        logoutput_debug("%s: %u bytes read from %s", __FUNCTION__, (unsigned int) bytesread, MOUNTINFO_FILE );
        mib->bytesread=(unsigned int) bytesread;
        result=1; /* success */

    }

    outclose:

    if (mfile) {

        fclose(mfile);
        mfile=NULL;

    }

#endif

    return result;

}


void MOUNTINFO_read_mount_table(struct mount_monitor_s *monitor)
{
    struct mountinfo_data_s mdata;
    struct mountinfo_line_s mline;
    struct list_element_s *list=NULL;
    struct io_buffer_s *iob=NULL;

    logoutput_debug("%s", __FUNCTION__);

    EVENT_signal_lock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);

    if (monitor->thread) {

        monitor->status |= MOUNT_MONITOR_STATUS_FLAG_CHANGED;
        EVENT_signal_unlock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);
        return;

    }

    monitor->thread=1;
    EVENT_signal_unlock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);

    readdatafromsystem:

    if (MOUNTINFO_io_read_mountinfo_from_system(monitor)<1) {

	logoutput_debug("%s: unable to read mountinfo data", __FUNCTION__);
	goto outunlock;

    }

    iob=monitor->current;

    if ((iob==NULL) || (IO_buffer_data_available(iob)==0)) {

	logoutput_debug("%s: no data available", __FUNCTION__);
	goto outunlock;

    }

    /* compare with data kept to compare ... if available ...
        if there is no difference then no need to compare line for line
        (and leave everything the same) */

    if (monitor->previous && IO_buffer_cmp(iob, monitor->previous)) {

        logoutput_debug("%s: data not changed ... no need to read line for line", __FUNCTION__);
        goto outunlock;

    }

    unsigned char tmp=LIST_header_set_write_lock(&monitor->mountlines, NULL);

    monitor->generation++;
    mountinfo_data_init(&mdata, iob);
    mountinfo_line_init(&mline);
    list=LIST_header_get_first(&monitor->mountlines);

    /* compare the current and the previous mountinfo

        - this is done by reading a line from the current mountinfo and compare with a line from the previous mountinfo 

        - this comparing is first done by memcmp, if they do not differ, take the next line in both current and previous

        - in general there are four different cases when looking a line is available in both current and previous:


                                        previous
                                available               not available
                            _____________________________________
                            |
        current available   |   compare                 added
                            |
            not available   |   removed                 stop
                            |
                            |
*/

    while (list || (mdata.eof==0)) {
        int diff=0;
        struct mountinfo_list_s *mlist=NULL;

        if (list) {

            /* previous available */

            mlist=(struct mountinfo_list_s *)((char *)list - offsetof(struct mountinfo_list_s, list));

            if (mdata.eof) {

                /* current not available -> removed */

                diff=-1;

            } else {

                /* current available ... compare with the corresponding line in the previous mountinfo */

                readlinefromcurrent:
                if (mountinfo_read_line_from_current(&mdata, &mline, mlist, &diff)==0) break;

            }

        } else {

            if (mdata.eof) break;
            if (mountinfo_read_line_from_current_without_previous(&mdata, &mline, &diff)==0) break;

        }

        /* test lines are the same ... if nothing has changed skip this line */

        processdiff:

        if (diff==-1) {

            /* mountid from previous is smaller than the one found in current ...
                which means it's removed */

            list=LIST_element_get_next(list);
            mountinfo_monitor_process_remove(monitor, mlist);

        } else if (diff==0) {

            /* lines are the same */

            DSTR_set_bytes_raw(&mlist->line.data, mline.data.str, mline.data.length, 0);
            mlist->generation=monitor->generation;
            list=LIST_element_get_next(list);

        } else {

            mountinfo_monitor_add_mline(monitor, &mline, list);

        }

    }

    /* release the write lock for read access (from ctx) */
    tmp=LIST_header_unset_write_lock(&monitor->mountlines);

    out:

    /* switch the current and previous .... */

    monitor->previous=monitor->current;
    monitor->current=((monitor->current==&monitor->buffer[0]) ? &monitor->buffer[1] : &monitor->buffer[0]);

    outunlock:

    EVENT_signal_lock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);

    if (monitor->status & MOUNT_MONITOR_STATUS_FLAG_CHANGED) {

        /* if changed (in the meantime while reading and processing the data) jump back */

        monitor->status &= ~MOUNT_MONITOR_STATUS_FLAG_CHANGED;
        EVENT_signal_unlock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);
        goto readdatafromsystem;

    }

    monitor->thread=0;
    EVENT_signal_unlock_flag(monitor->esignal, &monitor->status, MOUNT_MONITOR_STATUS_FLAG_THREAD);

}

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

void mountinfo_export_init(struct mountinfo_export_s *mexport)
{

    mexport->flags=0;
    mexport->mask=0;

    mexport->mountid=0;
    mexport->parentid=0;
    mexport->major=0;
    mexport->minor=0;

    for (unsigned int i=0; i<7; i++) DSTR_init(&mexport->fields[i]);

}

void mountinfo_export_clear(struct mountinfo_export_s *mexport)
{
    for (unsigned int i=0; i<7; i++) DSTR_clear(&mexport->fields[i]);
}

static unsigned char mountinfo_export_dstr_hlpr(struct dstr_s *field, struct dstr_s *data, struct mountinfo_str_s *mstr)
{

    if (DSTR_alloc_str_raw(field, mstr->length + 1, 1)) {

        MOUNTINFO_unescape_str_into_buffer(mstr, data, field->str);
        field->length=strlen(field->str);
        return 1;

    }

    return 0;

}

unsigned char mountinfo_export_mount_worker(struct mount_monitor_s *monitor, struct mountinfo_list_s *mlist, unsigned int mask, struct mountinfo_export_s *mexport)
{
    struct mountinfo_line_s *mline=&mlist->line;
    unsigned char result=1;

    if (mask==0) mask=MOUNTINFO_EXPORT_MASK_ALL;

    mexport->mountid=mlist->line.mountid;
    mexport->parentid=mlist->line.parentid;
    mexport->major=mlist->line.major;
    mexport->minor=mlist->line.minor;

    for (unsigned int i=0; i<7; i++) {
        unsigned int maskbit=(1 << i);

        if (mask & maskbit) {

            if (mountinfo_export_dstr_hlpr(&mexport->fields[i], &mline->data, &mlist->line.fields[i])==0) {

                logoutput_debug("%s: unable to allocate export field %u with %u bytes", __FUNCTION__, i, mlist->line.fields[i].length);
                result=0;
                break;

            }

            mexport->mask |= maskbit;
            mask &= ~maskbit;
            if (mask==0) break;

        }

    }

    return result;

}

unsigned char mountinfo_export_mount(struct mount_monitor_s *monitor, unsigned int mountid, unsigned int mask, struct mountinfo_export_s *mexport)
{
    unsigned char tmp=LIST_header_set_read_lock(&monitor->mountlines, NULL);
    struct list_element_s *list=LIST_header_get_first(&monitor->mountlines);
    struct mountinfo_list_s *mlist=NULL;
    unsigned char result=0;

    while (list) {

        mlist=(struct mountinfo_list_s *)((char *)list - offsetof(struct mountinfo_list_s, list));
        if (mlist->line.mountid==mountid) break;
        list=LIST_element_get_next(list);
        mlist=NULL;

    }

    if (mlist==NULL) {

        logoutput_debug("%s: no mountinfo line found with mount id %u", __FUNCTION__, mountid);

    } else {

        result=mountinfo_export_mount_worker(monitor, mlist, mask, mexport);

    }

    tmp=LIST_header_unset_read_lock(&monitor->mountlines);
    return result;

}

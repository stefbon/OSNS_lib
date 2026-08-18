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

void mountinfo_str_init(struct mountinfo_str_s *mstr)
{
    mstr->pos=0;
    mstr->length=0;
}

void mountinfo_data_init(struct mountinfo_data_s *md, struct io_buffer_s *iob)
{

    DSTR_set_bytes(&md->data, iob->ptr, iob->bytesread, 0);

    md->linelength=0;
    md->eof=0;

}

void mountinfo_line_init(struct mountinfo_line_s *mline)
{

    DSTR_init(&mline->data);

    mline->mountid=0;
    mline->parentid=0;
    mline->major=0;
    mline->minor=0;

    for (unsigned int i=0; i<7; i++) mountinfo_str_init(&mline->fields[i]);
}

void mountinfo_field_to_dstr(struct mountinfo_line_s *ml, unsigned int fieldnr, struct dstr_s *stra)
{
    DSTR_set_bytes(stra, &ml->data.str[ml->fields[fieldnr].pos], ml->fields[fieldnr].length, 0);
}

unsigned int mountinfo_data_read_line(struct mountinfo_data_s *md, struct mountinfo_line_s *ml)
{
    char *sep=NULL;

    if ((md->data.str==NULL) || (md->data.length==0)) return 0;

    /* if there is already a line shift first */

    if (md->linelength>0) {

        DSTR_shift(&md->data, md->linelength + 1);
        md->linelength=0; /* not known  ... not yet */

    }

    md->linelength=md->data.length; /* take the rest for now */

    /* serach for character 10 (==0x0A, newline) */

    sep=memchr(md->data.str, 10, md->linelength);
    if (sep) md->linelength=(unsigned int)(sep - md->data.str);
    if ((md->linelength+1) >= md->data.length) md->eof=1;

    DSTR_set_bytes(&ml->data, md->data.str, md->linelength, 0);

    return md->linelength;
}

unsigned char mountinfo_list_create(struct mountinfo_line_s *ml, struct mountinfo_list_s **p_mlist, uint64_t generation)
{
    struct mountinfo_list_s *mlist=malloc(sizeof(struct mountinfo_list_s));

    if (mlist) {

        memset(mlist, 0, sizeof(struct mountinfo_list_s));

        LIST_element_init(&mlist->list, NULL);
        mlist->generation=generation;
        mountinfo_line_init(&mlist->line);
        memcpy(&mlist->line, ml, sizeof(struct mountinfo_line_s));

        *p_mlist=mlist;
        return 1;

    }

    return 0;

}

void mountinfo_remove_list(struct mount_monitor_s *monitor)
{
    struct list_element_s *list=NULL;
    unsigned char tmp=LIST_header_set_write_lock(&monitor->mountlines, NULL);

    list=LIST_header_remove_first(&monitor->mountlines);

    while (list) {
        struct mountinfo_list_s *mlist=(struct mountinfo_list_s *)((char *)list - offsetof(struct mountinfo_list_s, list));

        free(mlist);
        list=LIST_header_remove_first(&monitor->mountlines);

    }

    tmp=LIST_header_unset_write_lock(&monitor->mountlines);

}

/* unescape */

static void cb_save_unescaped(unsigned char tchar, unsigned int ctr, unsigned char escaped, void *ptr)
{
    char *buffer=(char *) ptr;
    buffer[ctr]=tchar;
}

void MOUNTINFO_unescape_str_into_buffer(struct mountinfo_str_s *mstr, struct dstr_s *data, char *buffer)
{
    char *start=&data->str[mstr->pos];
    unsigned int length=mstr->length;

    // logoutput_debug("%s: mstr pos %u length %u data length %u", __FUNCTION__, mstr->pos, mstr->length, data->length);

    if (TXT_util_has_escaped(start, length)) {

        TXT_util_unescape(start, length, cb_save_unescaped, (void *) buffer);

    } else {

        memcpy(buffer, start, length);

    }

}

#ifdef __linux__

char *systemfs[] = {"sysfs", "bdev", "proc", "cgroup", "cgroup2", "cpuset", "devtmpfs", "binfmt_misc", "configfs", "debugfs", "securityfs", "sockfs", "bpf", "pipefs", "ramfs", "hugetlbfs", "devpts", "mqueue", "pstore", "fusectl", "tracefs"};
char *systemmountpoints[]={"/dev", "/sys", "/proc"};

unsigned char mountinfo_filesystem_is_system_related(struct mountinfo_line_s *ml)
{
    unsigned char result=0;
    struct dstr_s fs=DSTR_INIT;

    mountinfo_field_to_dstr(ml, MOUNTINFO_FIELD_FS, &fs);

    for (unsigned int i=0; i<(sizeof(systemfs)/sizeof(systemfs[0])); i++) {

	if (DSTR_cmp_bytes(&fs, systemfs[i], 0, 1, 0, 0)) {

	    result=1;
	    break;

	}

    }

    if (result==0) {
        struct mountinfo_str_s *mountpointstr=&ml->fields[MOUNTINFO_FIELD_MOUNTPOINT];
        unsigned int size=mountpointstr->length + 1;
        char buffer[size];
        unsigned int length=0;

        memset(buffer, 0, size);
        MOUNTINFO_unescape_str_into_buffer(mountpointstr, &ml->data, buffer);
        length=strlen(buffer);

        /* compare to the well known system related mount points */

        for (unsigned int i=0; i<(sizeof(systemmountpoints)/sizeof(systemmountpoints[0])); i++) {
            unsigned int tmp=strlen(systemmountpoints[i]);

            /* test the mountpoint equals and test it's a subdirectory */

            if (((length==tmp) || ((length>tmp) && (buffer[tmp]=='/'))) && (memcmp(buffer, systemmountpoints[i], tmp)==0)) {

                result=1;
                break;

            }

        }

    }

    return result;

}

#else

unsigned char mountinfo_filesystem_is_system_related(struct mountinfo_line_s *ml)
{
    return 0;
}

#endif

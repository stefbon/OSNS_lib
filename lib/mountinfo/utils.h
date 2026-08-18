/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_MOUNTINFO_UTILS_H
#define LIB_MOUNTINFO_UTILS_H

struct mountinfo_str_s {
    unsigned int                pos;
    unsigned int                length;
};

#define MOUNTINFO_LINE_FLAG_SYSTEM      1

struct mountinfo_line_s {
    unsigned int                flags;
    struct dstr_s               data;
    unsigned int                mountid;
    unsigned int                parentid;
    unsigned int                major;
    unsigned int                minor;
    struct mountinfo_str_s      fields[7];
};

struct mountinfo_data_s {
    struct dstr_s               data;
    unsigned int                linelength;
    unsigned char               eof;
};

struct mountinfo_list_s {
    struct list_element_s       list;
    uint64_t                    generation;
    struct mountinfo_line_s     line;
};

/* prototypes */

void mountinfo_data_init(struct mountinfo_data_s *md, struct io_buffer_s *iob);

void mountinfo_line_init(struct mountinfo_line_s *mline);
void mountinfo_field_to_dstr(struct mountinfo_line_s *ml, unsigned int fieldnr, struct dstr_s *stra);

unsigned int mountinfo_data_read_line(struct mountinfo_data_s *md, struct mountinfo_line_s *ml);

unsigned char mountinfo_list_create(struct mountinfo_line_s *ml, struct mountinfo_list_s **p_mlist, uint64_t generation);
void mountinfo_remove_list(struct mount_monitor_s *monitor);

void MOUNTINFO_unescape_str_into_buffer(struct mountinfo_str_s *mstr, struct dstr_s *data, char *buffer);

unsigned char mountinfo_filesystem_is_system_related(struct mountinfo_line_s *ml);

#endif

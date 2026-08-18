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

#ifndef LIB_MOUNTINFO_MONITOR_H
#define LIB_MOUNTINFO_MONITOR_H

#include "libosns-list.h"
#include "libosns-io.h"
#include "libosns-eventloop.h"
#include "libosns-fs.h"

#define MOUNTINFO_EXPORT_MASK_ROOT                      1
#define MOUNTINFO_EXPORT_MASK_MOUNTPOINT                2
#define MOUNTINFO_EXPORT_MASK_OPTIONS                   4
#define MOUNTINFO_EXPORT_MASK_OPTIONAL                  8
#define MOUNTINFO_EXPORT_MASK_FS                        16
#define MOUNTINFO_EXPORT_MASK_SOURCE                    32
#define MOUNTINFO_EXPORT_MASK_SUPEROPTIONS              64

#define MOUNTINFO_EXPORT_MASK_ALL                       ( MOUNTINFO_EXPORT_MASK_ROOT | MOUNTINFO_EXPORT_MASK_MOUNTPOINT | MOUNTINFO_EXPORT_MASK_OPTIONS | MOUNTINFO_EXPORT_MASK_OPTIONAL | MOUNTINFO_EXPORT_MASK_FS | MOUNTINFO_EXPORT_MASK_SOURCE | MOUNTINFO_EXPORT_MASK_SUPEROPTIONS )

#define MOUNTINFO_FIELD_ROOT                            0
#define MOUNTINFO_FIELD_MOUNTPOINT                      1
#define MOUNTINFO_FIELD_OPTIONS                         2
#define MOUNTINFO_FIELD_OPTIONAL                        3
#define MOUNTINFO_FIELD_FS                              4
#define MOUNTINFO_FIELD_SOURCE                          5
#define MOUNTINFO_FIELD_SUPEROPTIONS                    6

struct mountinfo_export_s {
    unsigned int                                        flags;
    unsigned int                                        mask;
    unsigned int                                        mountid;
    unsigned int                                        parentid;
    unsigned int                                        major;
    unsigned int                                        minor;
    struct dstr_s                                       fields[7];
};

#define MOUNT_MONITOR_DEFAULT_BUFFER_SIZE		1024

#define MOUNT_MONITOR_FLAG_IGNORE_SYSTEM_FS		1
#define MOUNT_MONITOR_FLAG_ALL                          MOUNT_MONITOR_FLAG_IGNORE_SYSTEM_FS

#define MOUNT_MONITOR_ACTION_ADDED			1
#define MOUNT_MONITOR_ACTION_REMOVED			2

#define MOUNT_MONITOR_STATUS_FLAG_INIT			1
#define MOUNT_MONITOR_STATUS_FLAG_CHANGED               2
#define MOUNT_MONITOR_STATUS_FLAG_READ                  4
#define MOUNT_MONITOR_STATUS_FLAG_MOUNT_ENTRIES         8
#define MOUNT_MONITOR_STATUS_FLAG_REMOVED_ENTRIES       16
#define MOUNT_MONITOR_STATUS_FLAG_THREAD                32

struct mountinfo_buffer_s {
    struct io_buffer_s                                  buffer;
    struct list_header_s			        header;
};

struct mount_monitor_s {
    unsigned int				        status;
    unsigned int                                        thread;
    unsigned int				        flags;
    unsigned int                                        mask_added;
    unsigned int                                        mask_removed;
    uint64_t					        generation;
    void                                                *ptr;
    struct event_shared_signal_s			*esignal;
    void 					        (* cb)(unsigned char added, struct mountinfo_export_s *mexport, void *ptr);
    struct fs_object_s                                  fso;
    struct bevent_ctx_s                                 bctx;
    struct list_header_s                                mountlines;
    struct io_buffer_s                                  *current;
    struct io_buffer_s                                  *previous;
    struct io_buffer_s                                  buffer[2];
};

/* prototypes */

void MOUNTINFO_monitor_init(struct event_shared_signal_s *esignal, unsigned int mask_added, unsigned int mask_removed, unsigned int flags, void *ptr);
unsigned char MOUNTINFO_monitor_open();
int MOUNTINFO_monitor_add_to_eventloop(struct beventloop_s *loop);
void MOUNTINFO_monitor_start();
void MOUNTINFO_monitor_close();

int MOUNT_monitor_read_data();

void MOUNTINFO_monitor_set_cb(void (* cb_custom)(unsigned char added, struct mountinfo_export_s *mexport, void *ptr));

unsigned char MOUNTINFO_export_mount(unsigned int mountid, unsigned int mask, struct mountinfo_export_s *mexport);

#endif

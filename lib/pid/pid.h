/*

  2010, 2011 Stef Bon <stefbon@gmail.com>

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
#ifndef LIB_PID_PID_H
#define LIB_PID_PID_H

#include "libosns-fs.h"
#include "libosns-error.h"
#include "libosns-datatypes.h"

#define PID_INFO_MASK_PID               1
#define PID_INFO_MASK_UID               2
#define PID_INFO_MASK_GID               4
#define PID_INFO_MASK_EXECUTABLE        8

struct pid_info_s {
    unsigned int                        mask;
    pid_t                               pid;
#ifdef __linux__
    uid_t                               uid;
    gid_t                               gid;
#endif
    struct fs_path_s                    executable;
};

/* Prototypes */

int PID_share_io(pid_t pid, unsigned int fd);
unsigned char PID_share_io_supported();

void PID_info_init(struct pid_info_s *info);
void PID_info_clear(struct pid_info_s *info);
unsigned int PID_get_info(pid_t pid, unsigned int mask, struct pid_info_s *info);

unsigned char PID_get_user_session(pid_t pid, struct dstr_s *session);
unsigned char PID_get_user_slice(pid_t pid, struct dstr_s *slice);
unsigned char PID_get_user_cgroup(pid_t pid, struct dstr_s *cgroup);

unsigned char PID_get_unique_pid(pid_t *p_pid);

#endif

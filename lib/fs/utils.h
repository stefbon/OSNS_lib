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

#ifndef LIB_FS_UTILS_H
#define LIB_FS_UTILS_H

#include "libosns-user.h"

#include "stat.h"

#define STAT_MODE_ROLE_USER			1
#define STAT_MODE_ROLE_GROUP			2
#define STAT_MODE_ROLE_OTHERS			4

#define STAT_MODE_PERM_READ			1
#define STAT_MODE_PERM_WRITE			2
#define STAT_MODE_PERM_EXEC			4

/* Prototypes */

unsigned int FS_util_translate_mode_from_role(unsigned int mode, unsigned char role, unsigned char perm);

unsigned char FS_util_enable_access_mode(struct fs_object_s *fsh, unsigned char type, void *ptr, unsigned char who, unsigned char action);
unsigned char FS_util_disable_access_mode(struct fs_object_s *fsh, unsigned char type, void *ptr, unsigned char who, unsigned char action);

unsigned char FS_util_set_owner(struct fs_object_s *fsh, unsigned char type, void *ptr, struct user_s *user);
unsigned char FS_util_set_group(struct fs_object_s *fsh, unsigned char type, void *ptr, struct group_s *group);

unsigned char FS_dentry_is_file(struct fs_dentry_s *dentry);
unsigned char FS_dentry_is_directory(struct fs_dentry_s *dentry);
unsigned char FS_dentry_is_socket(struct fs_dentry_s *dentry);
unsigned char FS_dentry_is_symlink(struct fs_dentry_s *dentry);

void FS_init_set_default(struct fs_init_s *init, unsigned char isfile);

int FS_path_mkdir(struct fs_path_s *basepath, struct fs_path_s *path, const char *(* get_name)(struct fs_path_s *path, mode_t *p_mode, unsigned int ctr, void *ptr), void *ptr);

#endif

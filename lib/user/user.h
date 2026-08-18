/*
  2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Stef Bon <stefbon@gmail.com>

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

#ifndef LIB_USER_USER_H
#define LIB_USER_USER_H

#include "group.h"

#define USER_SCOPE_LOCALHOST                    1

#define USER_FLAG_IS_SYSTEM_USER                1
#define USER_FLAG_IS_LOCAL_USER                 2
#define USER_FLAG_IS_SHARED_USER                4
#define USER_FLAG_IS_IMPORTED_USER              8

#define USER_NAME_TYPE_USERNAME                 1
#define USER_NAME_TYPE_FULL                     2
#define USER_NAME_TYPE_EMAIL                    3

#define USER_NAME_TYPE_DEFAULT                  USER_NAME_TYPE_USERNAME

#define USER_DIRECTORY_TYPE_HOME                0
#define USER_DIRECTORY_TYPE_DOCUMENTS           1
#define USER_DIRECTORY_TYPE_DOWNLOAD            2
#define USER_DIRECTORY_TYPE_MUSIC               3
#define USER_DIRECTORY_TYPE_VIDEO               4
#define USER_DIRECTORY_TYPE_PICTURES            5
#define USER_DIRECTORY_TYPE_DESKTOP             6

#define USER_DIRECTORY_FLAG_RELATIVE_TO_HOME    1
#define USER_DIRECTORY_FLAG_COPY                2
#define USER_DIRECTORY_FLAG_SET                 4

struct user_directory_s {
    unsigned char                               type;
    unsigned int                                flags;
    struct fs_path_s                            path;
};

struct user_s;

struct user_ops_s {
    int                                         (* get_name)(struct user_s *user, unsigned char type, struct dstr_s *name);
    int                                         (* get_groups)(struct user_s *user, int (* cb)(struct user_s *user, struct dstr_s *name, unsigned int flags));
    unsigned char                               (* get_unix_uid)(struct user_s *user, uid_t *p_uid);
    unsigned char                               (* get_unix_gid)(struct user_s *user, gid_t *p_gid);
    unsigned char                               (* get_directory)(struct user_s *user, unsigned char type, struct user_directory_s *ud, unsigned int flags);
};

#ifdef __linux__

#include <pwd.h>
#include <grp.h>

struct user_unix_s {
    unsigned int                                flags;
    struct passwd                               pwd;
};

#endif

struct user_s {
    unsigned int                                flags;
    struct user_ops_s                           *ops;
    // struct list_header_s                        directories;
    struct dstr_s                               buffer;
    union user_u {
#ifdef __linux__
        struct user_unix_s                      uunix;
#endif
        void                                    *ptr;
    } data;
};

/* prototypes */

void USER_get_system_range_uid();

struct user_s *USER_create();
void USER_init(struct user_s *user);
void USER_clear(struct user_s *user);

unsigned char USER_lookup_by_unique_uid(struct user_s *user, uid_t uid);
unsigned char USER_lookup_by_username(struct user_s *user, char *name);

int USER_get_name(struct user_s *user, unsigned char type, struct dstr_s *name);
int USER_get_groups(struct user_s *user, int (* cb)(struct user_s *u, struct dstr_s *name, unsigned int flags));
unsigned char USER_get_directory(struct user_s *user, unsigned char type, struct user_directory_s *ud, unsigned int flags);
unsigned char USER_get_unix_uid(struct user_s *user, uid_t *p_uid);
unsigned char USER_get_unix_gid(struct user_s *user, gid_t *p_gid);

unsigned char USER_is_root(struct user_s *user);
unsigned char USER_is_desktop_user(struct user_s *user);

void USER_directory_init(struct user_directory_s *ud);
void USER_directory_clear(struct user_directory_s *ud);

#endif

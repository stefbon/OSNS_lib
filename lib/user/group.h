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

#ifndef LIB_USER_GROUP_H
#define LIB_USER_GROUP_H

#define GROUP_NAME_TYPE_GROUPRNAME              1

#define GROUP_NAME_TYPE_DEFAULT                 GROUP_NAME_TYPE_GROUPRNAME

struct group_s;

struct group_ops_s {
    int                                         (* get_name)(struct group_s *group, unsigned char type, struct dstr_s *name);
    int                                         (* get_members)(struct group_s *group, int (* cb)(struct group_s *group, struct dstr_s *name, unsigned int flags));
    unsigned char                               (* get_unix_gid)(struct group_s *group, gid_t *p_gid);
};

#ifdef __linux__

#include <grp.h>

struct group_unix_s {
    unsigned int                                flags;
    struct group                                grp;
};

#endif

struct group_s {
    struct group_ops_s                          *ops;
    struct dstr_s                               buffer;
    union group_u {
#ifdef __linux__
        struct group_unix_s                     gunix;
#endif
        void                                    *ptr;
    } data;
};

/* prototypes */

struct group_s *GROUP_create();
void GROUP_init(struct group_s *group);
void GROUP_clear(struct group_s *group);

unsigned char GROUP_lookup_by_unique_gid(struct group_s *group, gid_t gid);
unsigned char GROUP_lookup_by_groupname(struct group_s *group, char *name);

int GROUP_get_name(struct group_s *group, unsigned char type, struct dstr_s *name);
int GROUP_get_members(struct group_s *group, int (* cb)(struct group_s *group, struct dstr_s *name, unsigned int flags));
unsigned char GROUP_get_unix_gid(struct group_s *group, gid_t *p_gid);

#endif

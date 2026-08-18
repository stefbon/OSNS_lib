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

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-datatypes.h"
#include "libosns-fs.h"
#include "libosns-system.h"

#include "group.h"

#ifdef __linux__

static int group_get_name_cb(struct group_s *group, unsigned char type, struct dstr_s *name)
{

    if ((group==NULL) || (name==NULL)) return -1;

    switch (type) {

        case GROUP_NAME_TYPE_DEFAULT:

            DSTR_set_bytes(name, group->data.gunix.grp.gr_name, 0, 0);
            break;

        default:

            DSTR_init(name);

    }

    return name->length;

}

static int group_get_members_cb(struct group_s *group, int (* cb)(struct group_s *group, struct dstr_s *member, unsigned int flags))
{
    char *name=group->data.gunix.grp.gr_mem[0];
    int result=0;

    while (name) {
        struct dstr_s member=DSTR_INIT;

        DSTR_set_bytes(&member, name, 0, 0);
        result=(* cb)(group, &member, 0);
        if (result==-1) break;
        name+=sizeof(name);

    }

    return result;

}

static unsigned char group_get_unix_gid_cb(struct group_s *group, gid_t *p_gid)
{
    if ((group==NULL) || (p_gid==NULL)) return 0;
    *p_gid=group->data.gunix.grp.gr_gid;
    return 1;
}

#else

static int group_get_name_cb(struct group_s *group, unsigned char type, struct dstr_s *name)
{
    return -1;
}

static int group_get_members_cb(struct group_s *group, int (* cb)(struct group_s *group, struct dstr_s *name, unsigned int flags))
{
    return -1;
}

static unsigned char group_get_unix_gid_cb(struct group_s *group, gid_t *p_gid)
{
    return 0;
}

#endif

static struct group_ops_s group_ops = {
    .get_name                                   = group_get_name_cb,
    .get_members                                = group_get_members_cb,
    .get_unix_gid                               = group_get_unix_gid_cb,
};

void GROUP_init(struct group_s *group)
{
    if (group==NULL) return;
    memset(group, 0, sizeof(struct group_s));
    group->ops=&group_ops;
    DSTR_init(&group->buffer);
}

void GROUP_clear(struct group_s *group)
{
    DSTR_clear(&group->buffer);
    GROUP_init(group);
}

struct group_s *GROUP_create()
{
    struct group_s *group=malloc(sizeof(struct group_s));

    if (group) GROUP_init(group);
    return group;

}

#ifdef __linux__

#define LOOKUP_GROUP_BY_GID                             1
#define LOOKUP_GROUP_BY_GROUPNAME                       2

struct lookup_group_hlpr_s {
    unsigned char                                       type;
    union lookup_group_hlpr_u {
        gid_t                                           gid;
        char                                            *groupname;
    } key;
};

static unsigned char GROUP_lookup_shared(struct group_s *group, struct lookup_group_hlpr_s *hlpr)
{
    unsigned char result=0;
    unsigned int bsize=128;
    struct group *getgr_result=NULL;

    if (group==NULL) return -1;
    DSTR_clear(&group->buffer);
    memset(&group->data, 0, sizeof(union group_u));

    while (getgr_result==NULL) {

        if (DSTR_alloc_str_raw(&group->buffer, bsize, 1)==0) {

            logoutput_debug("%s: unable to allocate %u bytes", __FUNCTION__, bsize);
            break;

        }

        if (hlpr->type==LOOKUP_GROUP_BY_GID) {

            if (getgrgid_r(hlpr->key.gid, &group->data.gunix.grp, group->buffer.str, group->buffer.length, &getgr_result)==0) {

                result=1;
                break;

            }

        } else if (hlpr->type==LOOKUP_GROUP_BY_GROUPNAME) {

            if (getgrnam_r(hlpr->key.groupname, &group->data.gunix.grp, group->buffer.str, group->buffer.length, &getgr_result)==0) {

                result=1;
                break;

            }

        } else {

            /* should not happen */
            break;

        }

        if (errno==ERANGE) {

            bsize+=128;

        } else {

            logoutput_debug("%s: failed to find a group ... error %u (%s)", __FUNCTION__, errno, strerror(errno));
            break;

        }

    }

    if (result==1) {

        logoutput_debug("%s: found group with groupname %s and gid %u", __FUNCTION__, getgr_result->gr_name, getgr_result->gr_gid);

    } else {

        DSTR_clear(&group->buffer);

    }

    return result;

}

#else

static unsigned char GROUP_lookup_shared(struct group_s *group, struct lookup_group_hlpr_s *hlpr)
{
    return 0;
}

#endif

unsigned char GROUP_lookup_by_unique_gid(struct group_s *group, gid_t gid)
{
    struct lookup_group_hlpr_s hlpr;

    hlpr.type=LOOKUP_GROUP_BY_GID;
    hlpr.key.gid=gid;

    return GROUP_lookup_shared(group, &hlpr);
}

unsigned char GROUP_lookup_by_groupname(struct group_s *group, char *name)
{
    struct lookup_group_hlpr_s hlpr;

    hlpr.type=LOOKUP_GROUP_BY_GROUPNAME;
    hlpr.key.groupname=name;

    return GROUP_lookup_shared(group, &hlpr);
}

int GROUP_get_name(struct group_s *group, unsigned char type, struct dstr_s *name)
{
    return (* group->ops->get_name)(group, type, name);
}

int GROUP_get_members(struct group_s *group, int (* cb)(struct group_s *group, struct dstr_s *name, unsigned int flags))
{
    return (* group->ops->get_members)(group, cb);
}

unsigned char GROUP_get_unix_gid(struct group_s *group, gid_t *p_gid)
{
    return (* group->ops->get_unix_gid)(group, p_gid);
}

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
#include "libosns-path.h"
#include "libosns-file.h"

#include "user.h"

#ifdef __linux__

#include <grp.h>
#include <pwd.h>

static unsigned int desktop_user_minimum_uid=1000;
static unsigned int desktop_user_maximum_uid=60000;

static int user_get_name_cb(struct user_s *user, unsigned char type, struct dstr_s *name)
{

    if ((user==NULL) || (name==NULL)) return -1;

    switch (type) {

        case USER_NAME_TYPE_DEFAULT:

            DSTR_set_bytes(name, user->data.uunix.pwd.pw_name, 0, 0);
            break;

        case USER_NAME_TYPE_FULL:

            DSTR_set_bytes(name, user->data.uunix.pwd.pw_gecos, 0, 0);

        default:

            DSTR_init(name);

    }

    return name->length;

}

static int user_get_groups_cb(struct user_s *user, int (* cb)(struct user_s *u, struct dstr_s *name, unsigned int flags))
{
    int length=32;
    int tmp=0;
    gid_t *agroups=NULL;
    int result=0;

    allocatearray:

    agroups=realloc(agroups, length * sizeof(gid_t));

    if (agroups==NULL) {

        logoutput_debug("%s: failed to allocate array with %u members", __FUNCTION__, length);
        return -1;

    }

    tmp=length;
    if (getgrouplist(user->data.uunix.pwd.pw_name, user->data.uunix.pwd.pw_gid, agroups, &tmp)==-1) {

        if (tmp > length) {

            /* there are more groups than the length of the array .... reallocate */

            length=tmp;
            goto allocatearray;

        }

        logoutput_debug("%s: unable to getgrouplist", __FUNCTION__);
        result=-1;
        goto out;

    }

    for (unsigned int i=0; i<tmp; i++) {

        if (agroups[i]) {
            struct group *grp=getgrgid(agroups[i]);

            if (grp) {
                struct dstr_s groupname=DSTR_INIT;

                DSTR_set_bytes(&groupname, grp->gr_name, 0, 0);
                result=(* cb)(user, &groupname, 0);
                if (result==-1) break;

            }

        }

    }

    out:

    free(agroups);
    return result;

}

static unsigned char user_get_unix_uid_cb(struct user_s *user, uid_t *p_uid)
{
    if ((user==NULL) || (p_uid==NULL)) return 0;
    *p_uid=user->data.uunix.pwd.pw_uid;
    return 1;
}

static unsigned char user_get_unix_gid_cb(struct user_s *user, uid_t *p_gid)
{
    if ((user==NULL) || (p_gid==NULL)) return 0;
    *p_gid=user->data.uunix.pwd.pw_gid;
    return 1;
}

static unsigned char user_get_directory_cb(struct user_s *user, unsigned char type, struct user_directory_s *ud, unsigned int flags)
{
    unsigned char result=0;

    if (type==USER_DIRECTORY_TYPE_HOME) {
        char *homedir=(user) ? user->data.uunix.pwd.pw_dir : NULL;
        unsigned int length=(homedir ? strlen(homedir) : 0);
        struct fs_path_s *path=&ud->path;

        if (length==0) return 0;
        logoutput_debug("%s: found home directory %s", __FUNCTION__, homedir);

        if (FS_path_copy(path, 'c', homedir, (flags & USER_DIRECTORY_FLAG_COPY ? 1 : 0))) {

            ud->type=type;
            ud->flags=0;
            result=1;

        } else {

            logoutput_debug("%s: failed to copy home directory", __FUNCTION__);

        }

    } else {

        logoutput_debug("%s: tyoe %u not supported", __FUNCTION__, type);

    }

    return result;

}

static void user_util_skip_spaces(struct dstr_s *stra, unsigned char checkheading, unsigned char checktrailing, unsigned char checktabs)
{

    if (checkheading) {

        while (stra->length && isspace(stra->str[0])) DSTR_shift_raw(stra, 1);

    }

    if (checktrailing) {

        while (stra->length && isspace(stra->str[stra->length - 1])) stra->length--;

    }

    if (checktabs) {
        unsigned int ctr=0;

        while (ctr<stra->length) {

            if (stra->str[ctr]=='\t') stra->str[ctr]=' ';
            ctr++;

        }

    }

}

static void utils_skip_heading_spaces(struct dstr_s *stra)
{
    while (stra->length && isspace(stra->str[0])) DSTR_shift_raw(stra, 1);
}

static unsigned char read_uidminmax_cb(struct dstr_s *line, void *ptr)
{
    unsigned int ctr=0;
    struct dstr_s option=DSTR_INIT;

    utils_skip_heading_spaces(line);

    increase:

    if ((ctr<line->length) && (isspace(line->str[ctr])==0)) {

	ctr++;
	goto increase;

    }

    if (ctr==line->length) return 0;

    DSTR_set_bytes_raw(&option, &line->str[ctr], (line->length - ctr), 0);

    if (DSTR_cmp_bytes(&option, "UID_MIN", 0, 1, 0, 0) || DSTR_cmp_bytes(&option, "UID_MAX", 0, 1, 0, 0)) {
	struct dstr_s value=DSTR_INIT;
	unsigned long tmp=0;

	DSTR_set_bytes_raw(&value, &line->str[ctr], line->length - ctr, 0);
        utils_skip_heading_spaces(&value);
	tmp=DSTR_convert_str_to_long(&value);

	if (DSTR_cmp_bytes(&option, "UID_MIN", 0, 1, 0, 0)) {

	    desktop_user_minimum_uid=tmp;
	    logoutput_debug("%s: found minimum uid %lu", __FUNCTION__, desktop_user_minimum_uid);

	} else {

	    desktop_user_maximum_uid=tmp;
	    logoutput_debug("%s: found maximum uid %lu", __FUNCTION__, desktop_user_maximum_uid);

	}

    }

    return 0;

}

void USER_get_system_range_uid()
{
    struct fs_path_s path=FS_PATH_INIT;

    FS_path_set(&path, 'c', "/etc/login.defs", 0);

    if (FILE_parse(&path, read_uidminmax_cb, NULL)==0) {

        logoutput_debug("%s: unable to open file %.*s", __FUNCTION__, path.start.length, path.start.str);

    }

}

#else

static int user_get_name_cb(struct user_s *user, unsigned char scope, unsigned char type, struct dstr_s *name)
{
    return -1;
}

static int user_get_groups_cb(struct user_s *user, int (* cb)(struct user_s *u, struct dstr_s *name, unsigned int flags))
{
    return -1;
}

static unsigned char user_get_unix_uid_cb(struct user_s *user, uid_t *p_uid)
{
    return 0;
}

static unsigned char user_get_unix_gid_cb(struct user_s *user, uid_t *p_gid)
{
    return 0;
}

static unsigned char user_get_directory_cb(struct user_s *user, unsigned char type, struct user_directory_s *ud, unsigned int flags)
{
    return 0;
}

void USER_get_system_range_uid()
{}

#endif

static struct user_ops_s user_ops = {
    .get_name                                   = user_get_name_cb,
    .get_groups                                 = user_get_groups_cb,
    .get_unix_uid                               = user_get_unix_uid_cb,
    .get_unix_gid                               = user_get_unix_gid_cb,
    .get_directory                              = user_get_directory_cb,
};

void USER_init(struct user_s *user)
{
    if (user==NULL) return;
    memset(user, 0, sizeof(struct user_s));
    user->ops=&user_ops;
    DSTR_init(&user->buffer);
}

void USER_clear(struct user_s *user)
{
    DSTR_clear(&user->buffer);
    USER_init(user);
}

struct user_s *USER_create()
{
    struct user_s *user=malloc(sizeof(struct user_s));

    if (user) USER_init(user);
    return user;

}

#ifdef __linux__

#define LOOKUP_USER_BY_UID                      1
#define LOOKUP_USER_BY_USERNAME                 2

struct lookup_user_hlpr_s {
    unsigned char                               type;
    union lookup_user_hlpr_u {
        uid_t                                   uid;
        char                                    *username;
    } key;
};

static unsigned char USER_lookup_shared(struct user_s *user, struct lookup_user_hlpr_s *hlpr)
{
    unsigned char result=0;
    unsigned int bsize=128;
    struct passwd *getpw_result=NULL;

    if (user==NULL) return -1;
    DSTR_clear(&user->buffer);
    memset(&user->data, 0, sizeof(union user_u));

    while (getpw_result==NULL) {

        if (DSTR_alloc_str_raw(&user->buffer, bsize, 1)==0) {

            logoutput_debug("%s: unable to allocate %u bytes", __FUNCTION__, bsize);
            break;

        }

        if (hlpr->type==LOOKUP_USER_BY_UID) {

            if (getpwuid_r(hlpr->key.uid, &user->data.uunix.pwd, user->buffer.str, user->buffer.length, &getpw_result)==0) {

                result=1;
                break;

            }

        } else if (hlpr->type==LOOKUP_USER_BY_USERNAME) {

            if (getpwnam_r(hlpr->key.username, &user->data.uunix.pwd, user->buffer.str, user->buffer.length, &getpw_result)==0) {

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

            logoutput_debug("%s: failed to find an user ... error %u (%s)", __FUNCTION__, errno, strerror(errno));
            break;

        }

    }

    if (result==1) {

        logoutput_debug("%s: found user with username %s and uid %u", __FUNCTION__, getpw_result->pw_name, getpw_result->pw_uid);

    } else {

        DSTR_clear(&user->buffer);

    }

    return result;

}

#else

static unsigned char USER_lookup_shared(struct user_s *user, struct lookup_user_hlpr_s *hlpr)
{
    return 0;
}

#endif

unsigned char USER_lookup_by_unique_uid(struct user_s *user, uid_t uid)
{
    struct lookup_user_hlpr_s hlpr;

    hlpr.type=LOOKUP_USER_BY_UID;
    hlpr.key.uid=uid;

    return USER_lookup_shared(user, &hlpr);
}

unsigned char USER_lookup_by_username(struct user_s *user, char *name)
{
    struct lookup_user_hlpr_s hlpr;

    hlpr.type=LOOKUP_USER_BY_USERNAME;
    hlpr.key.username=name;

    return USER_lookup_shared(user, &hlpr);
}

int USER_get_name(struct user_s *user, unsigned char type, struct dstr_s *name)
{
    if (user==NULL) return -1;
    return (* user->ops->get_name)(user, type, name);
}

int USER_get_groups(struct user_s *user, int (* cb)(struct user_s *u, struct dstr_s *name, unsigned int flags))
{
    if (user==NULL) return -1;
    return (* user->ops->get_groups)(user, cb);
}

unsigned char USER_get_directory(struct user_s *user, unsigned char type, struct user_directory_s *ud, unsigned int flags)
{
    if (user==NULL) return 0;
    return (* user->ops->get_directory)(user, type, ud, flags);
}

unsigned char USER_get_unix_uid(struct user_s *user, uid_t *p_uid)
{
    if (user==NULL) return 0;
    return (* user->ops->get_unix_uid)(user, p_uid);
}

unsigned char USER_get_unix_gid(struct user_s *user, gid_t *p_gid)
{
    if (user==NULL) return 0;
    return (* user->ops->get_unix_gid)(user, p_gid);
}


unsigned char USER_is_root(struct user_s *user)
{

#ifdef __linux__
    uid_t uid=(uid_t) -1;

    return ((USER_get_unix_uid(user, &uid) && (uid==0)) ? 1 : 0);

#else

    return 0;

#endif

}

unsigned char USER_is_desktop_user(struct user_s *user)
{

#ifdef __linux__
    uid_t uid=(uid_t) -1;

    return ((USER_get_unix_uid(user, &uid) && (uid>=desktop_user_minimum_uid)) ? 1 : 0);

#else

    return 0;

#endif

}

static void user_directory_init(struct user_directory_s *ud)
{
    memset(ud, 0, sizeof(struct user_directory_s));

    ud->type=0;
    ud->flags=0;
    FS_path_init(&ud->path);
}

void USER_directory_init(struct user_directory_s *ud)
{
    if (ud) user_directory_init(ud);
}

static void user_directory_clear(struct user_directory_s *ud)
{
    FS_path_clear(&ud->path);
    user_directory_init(ud);
}

void USER_directory_clear(struct user_directory_s *ud)
{
    if (ud) user_directory_clear(ud);
}

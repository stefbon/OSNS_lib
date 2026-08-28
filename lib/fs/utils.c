/*
  2010, 2011, 2012, 2103, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Stef Bon <stefbon@gmail.com>

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

#include <fcntl.h>
#include <sys/stat.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"
#include "libosns-fs.h"
#include "libosns-user.h"

#include "stat.h"
#include "utils.h"

unsigned int FS_util_translate_mode_from_role(unsigned int mode, unsigned char role, unsigned char perm)
{

#ifdef __linux__

    if (role & STAT_MODE_ROLE_USER) {

	if (perm & STAT_MODE_PERM_READ) mode |= S_IRUSR;
	if (perm & STAT_MODE_PERM_WRITE) mode |= S_IWUSR;
	if (perm & STAT_MODE_PERM_EXEC) mode |= S_IXUSR;

    }

    if (role & STAT_MODE_ROLE_GROUP) {

	if (perm & STAT_MODE_PERM_READ) mode |= S_IRGRP;
	if (perm & STAT_MODE_PERM_WRITE) mode |= S_IWGRP;
	if (perm & STAT_MODE_PERM_EXEC) mode |= S_IXGRP;

    }

    if (role & STAT_MODE_ROLE_OTHERS) {

	if (perm & STAT_MODE_PERM_READ) mode |= S_IROTH;
	if (perm & STAT_MODE_PERM_WRITE) mode |= S_IWOTH;
	if (perm & STAT_MODE_PERM_EXEC) mode |= S_IXOTH;

    }

#endif

    return (mode & ~S_IFMT);

}

static unsigned char fs_util_set_access_mode(struct fs_object_s *fso, unsigned char type, void *ptr, unsigned char who, unsigned char action, unsigned char enable)
{
    unsigned char result=0;
    struct fs_stat_s fst;

    if ((who==0) || (action==0)) return 0;
    memset(&fst, 0, sizeof(struct fs_stat_s));

    if (FS_fgetstat(fso, type, ptr, FS_STAT_MODE, &fst)>0) {
        uint16_t tmp=FS_stat_get_mode(&fst);
        uint16_t mode=FS_util_translate_mode_from_role(0, who, action);
        unsigned char dosetstat=0;

        if (enable) {

            /* test bits are not set already
                (if every bit is already set no need to set them) */

            if ((mode & tmp) < mode) {

                FS_stat_set_mode(&fst, (mode | tmp));
                dosetstat=1;

            }

        } else {

            /* test bits are set
                (if none is set it's not required to unset them) */

            if (mode & tmp) {

                FS_stat_set_mode(&fst, (tmp & ~mode));
                dosetstat=1;

            }

        }

        if (dosetstat) {
            unsigned maskset=FS_fsetstat(fso, type, ptr, FS_STAT_MODE, &fst);

            result=(maskset & FS_STAT_MODE) ? 1 : 0;

        }

    }

    return result;

}

unsigned char FS_util_enable_access_mode(struct fs_object_s *fso, unsigned char type, void *ptr, unsigned char who, unsigned char action)
{
    return fs_util_set_access_mode(fso, type, ptr, who, action, 1);
}

unsigned char FS_util_disable_access_mode(struct fs_object_s *fso, unsigned char type, void *ptr, unsigned char who, unsigned char action)
{
    return fs_util_set_access_mode(fso, type, ptr, who, action, 0		);
}

unsigned char FS_util_set_owner(struct fs_object_s *fso, unsigned char type, void *ptr, struct user_s *user)
{
    unsigned char result=0;

#ifdef __linux__
    uid_t uid=(uid_t) -1;

    if (USER_get_unix_uid(user, &uid)) {
        struct fs_stat_s fst;

        memset(&fst, 0, sizeof(struct fs_stat_s));
        FS_stat_set_uid(&fst, uid);
        unsigned int maskset=FS_fsetstat(fso, type, ptr, FS_STAT_UID, &fst);

        result=(maskset & FS_STAT_UID) ? 1 : 0;

    }

#endif

    return result;
}

unsigned char FS_util_set_group(struct fs_object_s *fso, unsigned char type, void *ptr, struct group_s *group)
{
    unsigned char result=0;

#ifdef __linux__
    gid_t gid=(gid_t) -1;

    if (GROUP_get_unix_gid(group, &gid)) {
        struct fs_stat_s fst;

        memset(&fst, 0, sizeof(struct fs_stat_s));
        FS_stat_set_gid(&fst, gid);
        uint64_t maskset=FS_fsetstat(fso, type, ptr, FS_STAT_GID, &fst);

        result=(maskset & FS_STAT_GID) ? 1 : 0;

    }

#endif

    return result;
}

unsigned char FS_dentry_is_file(struct fs_dentry_s *dentry)
{
    return (S_ISREG(dentry->type) ? 1 : 0);
}

unsigned char FS_dentry_is_directory(struct fs_dentry_s *dentry)
{
    return (S_ISDIR(dentry->type) ? 1 : 0);
}

unsigned char FS_dentry_is_socket(struct fs_dentry_s *dentry)
{
    return (S_ISSOCK(dentry->type) ? 1 : 0);
}

unsigned char FS_dentry_is_symlink(struct fs_dentry_s *dentry)
{
    return (S_ISLNK(dentry->type) ? 1 : 0);
}

#ifdef __linux__

#include <fcntl.h>
#include <sys/stat.h>

mode_t FS_mode_get_default(unsigned char isfile)
{
    return (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) | ((isfile) ? (S_IFREG) : (S_IFDIR | S_IXUSR | S_IXGRP |  S_IXOTH));
}

void FS_init_set_default(struct fs_init_s *init, unsigned char isfile)
{
    init->mode=FS_mode_get_default(isfile);
    init->dev=0;
}

#else

mode_t FS_mode_get_default(unsigned char isfile)
{
    return 0;
}

void FS_init_set_default(struct fs_init_s *init, unsigned char isfile)
{
}

#endif

static int fs_path_mkdir_check_exist_hlpr(struct fs_path_s *path, const char *name, mode_t mode)
{
    unsigned int length=path->start.length + 1 + strlen(name);
    char buffer[length + 1];
    int tmp=snprintf(buffer, length+1, "%.*s/%s", path->start.length, path->start.str, name);
    int result=0;

    if (access(buffer, F_OK)==0) {

        logoutput_debug("%s: path %s already exist", __FUNCTION__, buffer);

    } else {

        if (mode==0) mode=FS_mode_get_default(0);

        if (mkdir(buffer, mode)==0) {

            logoutput_debug("%s: path %s did not exist, created", __FUNCTION__, buffer);
            result=1;

        } else {

            logoutput_debug("%s: unable to create %s ... error %u (%s)", __FUNCTION__, buffer, errno, strerror(errno));
            result=-1;

        }

    }

    return result;

}

int FS_path_mkdir(struct fs_path_s *basepath, struct fs_path_s *path, const char *(* get_name)(struct fs_path_s *path, mode_t *p_mode, unsigned int ctr, void *ptr), void *ptr)
{
    int result=0;
    mode_t mode=0;
    const char *name=NULL;
    struct fs_path_s dummy=FS_PATH_INIT;
    unsigned int tmp=0;
    unsigned int ctr=0;

    if (path==NULL) path=&dummy;

    FS_path_append_init(path, FS_PATH_FLAG_BUFFER_ALLOC);
    tmp=FS_path_append(path, 'p', (void *) basepath, 0);

    while ((name=(* get_name)(path, &mode, ctr, ptr))) {

        result=fs_path_mkdir_check_exist_hlpr(path, name, mode);
        if (result==-1) break;
        tmp=FS_path_append(path, 'c', (void *) name, 1);
        ctr++;

    }

    if (path==&dummy) FS_path_clear(path);
    return result;

}

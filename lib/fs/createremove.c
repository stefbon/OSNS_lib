/*

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

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-io.h"

#include "fs.h"

static unsigned char fs_object_mkrm(struct fs_object_s *fso, const unsigned char type, void *ptr, const char *what, struct fs_init_s *init)
{
    unsigned char result=0;
    struct dstr_s data=DSTR_INIT;
    unsigned int length=FS_path_convert_type_ptr(type, ptr, &data);
    char name[length+1];

    logoutput_debug("%s", __FUNCTION__);

    if ((length==0) && ((strcmp(what, "mkdir")==0) || (strcmp(what, "mk")==0))) {

        logoutput_debug("%s: error %u doing %s on path %s (%s)", __FUNCTION__, EINVAL, what, name, strerror(EINVAL));
        return 0;

    }

    memcpy(name, data.str, length);
    name[length]='\0';

    {
        int fd=FS_object_get_unix_fd(fso);
        int tmp=-1;

        if (strcmp(what, "rm")==0) {

            tmp=unlinkat(fd, name, 0);

        } else if (strcmp(what, "rmdir")==0) {

            tmp=unlinkat(fd, name, AT_REMOVEDIR);

        } else if (strcmp(what, "mkdir")==0) {
            mode_t mode=(init ? init->mode : 0755);

            tmp=mkdirat(fd, name, mode);

        } else if (strcmp(what, "mk")==0) {
            mode_t mode=(init ? init->mode : 0644);
            dev_t dev=(init ? init->dev : S_IFREG);

            tmp=mknodat(fd, name, mode, dev);

        } else {

            errno=ENOSYS;

        }

        if (tmp==-1) {

	    logoutput_debug("%s: error %u doing %s on path %s (%s)", __FUNCTION__, errno, what, name, strerror(errno));

        } else {

            result=1;
	    logoutput_debug("%s: %s success path %s", __FUNCTION__, what, name);

        }

    }

    return result;
}

unsigned char FS_rm(struct fs_object_s *fso, const unsigned char type, void *ptr)
{
    return fs_object_mkrm(fso, type, ptr, "rm", NULL);
}

unsigned char FS_rmdir(struct fs_object_s *fso, const unsigned char type, void *ptr)
{
    return fs_object_mkrm(fso, type, ptr, "rmdir", NULL);
}

unsigned char FS_mk(struct fs_object_s *fso, const unsigned char type, void *ptr, struct fs_init_s *init)
{
    return fs_object_mkrm(fso, type, ptr, "mk", init);
}

unsigned char FS_mkdir(struct fs_object_s *fso, const unsigned char type, void *ptr, struct fs_init_s *init)
{
    return fs_object_mkrm(fso, type, ptr, "mkdir", init);
}

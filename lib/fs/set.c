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
#include <sys/uio.h>

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-fs.h"

#include "fs.h"

static unsigned char fs_object_set_openflag_cb(struct fs_object_s *fso, const char *what, unsigned char enable)
{
    unsigned int flag=0;
    unsigned int flags2unset=0;

    if ((strcmp(what, "readonly")==0) || (strcmp(what, "rdonly")==0)) {

        flag=O_RDONLY;
        if (enable) flags2unset=O_WRONLY | O_RDWR;

    } else if ((strcmp(what, "writeonly")==0) || (strcmp(what, "wronly")==0)) {

        flag=O_WRONLY;
        if (enable) flags2unset=O_RDONLY | O_RDWR;

    } else if ((strcmp(what, "readwrite")==0) || (strcmp(what, "rdwr")==0)) {

        flag=O_RDWR;
        if (enable) flags2unset=O_RDONLY | O_WRONLY;

    } else if (strcmp(what, "append")==0) {

        flag=O_APPEND;

    } else if (strcmp(what, "async")==0) {

        flag=O_ASYNC;

    } else if (strcmp(what, "create")==0) {

        flag=(O_CREAT | O_EXCL);

    } else if (strcmp(what, "directory")==0) {

        flag=O_DIRECTORY;

    } else if (strcmp(what, "dsync")==0) {

        flag=O_DSYNC;

    } else if (strcmp(what, "noatime")==0) {

        flag=O_NOATIME;

    } else if (strcmp(what, "nofollow")==0) {

        flag=O_NOFOLLOW;

    } else if (strcmp(what, "path")==0) {

        flag=O_PATH;

    } else if (strcmp(what, "sync")==0) {

        flag=O_SYNC;

    } else if (strcmp(what, "tmpfile")==0) {

        flag=O_TMPFILE;

    } else if ((strcmp(what, "truncate")==0) || (strcmp(what, "trunc")==0)) {

        flag=O_TRUNC;

    }

    if (flag || flags2unset) return io_object_set_flag_hlpr(&fso->openflags, enable, flag, flags2unset);
    logoutput_warning("%s: what %s not reckognized", __FUNCTION__, what);
    return 0;

}

static unsigned char fs_object_set_writeflag_cb(struct fs_object_s *fso, const char *what, unsigned char enable)
{
    unsigned int flag=0;

    if (strcmp(what, "dsync")==0) {

        flag=RWF_DSYNC;

    } else if (strcmp(what, "hipri")==0) {

        flag=RWF_HIPRI;

    } else if (strcmp(what, "sync")==0) {

        flag=RWF_SYNC;

    } else if (strcmp(what, "append")==0) {

        flag=RWF_APPEND;

    }

    if (flag) return io_object_set_flag_hlpr(&fso->writeflags, enable, flag, 0);
    logoutput_warning("%s: what %s not reckognized", __FUNCTION__, what);
    return 0;

}

static unsigned char fs_object_set_readflag_cb(struct fs_object_s *fso, const char *what, unsigned char enable)
{
    unsigned int flag=0;

    if (strcmp(what, "hipri")==0) {

        flag=RWF_HIPRI;

    } else if (strcmp(what, "nowait")==0) {

        flag=RWF_NOWAIT;

    }

    if (flag) return io_object_set_flag_hlpr(&fso->readflags, enable, flag, 0);
    logoutput_warning("%s: what %s not reckognized", __FUNCTION__, what);
    return 0;

}

unsigned char FS_object_set_flag(struct fs_object_s *fso, const char *which, const char *what, unsigned char enable)
{

    if ((fso==NULL) || (what==NULL) || (which==NULL)) {

        logoutput_debug("%s: invalid parameters", __FUNCTION__);
        return 0;

    }

    logoutput_debug("%s: which %s what %s", __FUNCTION__, which, what);

    if (strcmp(which, "open")==0) {

        return fs_object_set_openflag_cb(fso, what, enable);

    } else if (strcmp(which, "read")==0) {

        return fs_object_set_readflag_cb(fso, what, enable);

    } else if (strcmp(which, "write")==0) {

        return fs_object_set_writeflag_cb(fso, what, enable);

    }

    logoutput_debug("%s: which %s not supported", __FUNCTION__, which);
    return 0;

}


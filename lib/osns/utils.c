/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-event.h"
#include "libosns-list.h"
#include "libosns-system.h"

#include "libosns-eventloop.h"
#include "libosns-network.h"
#include "libosns-io.h"
#include "libosns-fs.h"
#include "libosns-socket.h"
#include "libosns-connection.h"
#include "libosns-pid.h"
#include "libosns-user.h"

#include "osns.h"

/* change the group of a file */

unsigned char OSNS_enable_groupaccess(struct fs_path_s *path, struct dstr_s *groupname)
{
    unsigned char result=0;
    struct group_s group;
    unsigned int length=(groupname) ? groupname->length : 0;
    char buffer[length + 1];

    if ((FS_path_valid(path)==0) || (length==0)) {

        logoutput_debug("%s: path and/or group not valid or defined", __FUNCTION__);
        return 0;

    }

    memcpy(buffer, groupname->str, length);
    buffer[length]='\0';

    GROUP_init(&group);

    if (GROUP_lookup_by_groupname(&group, buffer)) {

        if (FS_util_set_group(NULL, 'p', (void *) path, &group)==0) {

            logoutput_debug("%s: unable to set group to %s", __FUNCTION__, buffer);
            return 0;

        }

        if (FS_util_enable_access_mode(NULL, 'p', (void *) path, STAT_MODE_ROLE_GROUP, (STAT_MODE_PERM_READ | STAT_MODE_PERM_WRITE))==0) {

            logoutput_debug("%s: unable to set access", __FUNCTION__);
            return 0;

        }

        result=1;

    } else {

        logoutput_debug("%s: unable to get group for name %s", __FUNCTION__, buffer);

    }

    return result;

}


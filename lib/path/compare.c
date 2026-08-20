/*
  2010, 2011, 2012, 2103, 2014, 2015, 2016, 2017 Stef Bon <stefbon@gmail.com>

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

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"

#include "path.h"
#include "utils.h"
#include "seperator.h"

#include "compare.h"

/* hlpr function tp determine the second path is a subdirectory of the first path */

static unsigned char fs_path_compare_subdirectory(struct dstr_s *pa, struct dstr_s *pb, unsigned char flag, struct dstr_s *sub)
{
    unsigned int length=pa->length;

    logoutput_debug("%s: compare a %.*s with b %.*s", __FUNCTION__, pa->length, pa->str, pb->length, pb->str);

    if (length==pb->length) {

        return (flag & FS_PATH_COMPARE_FLAG_ALLOW_EQUAL) ? 1 : 0;

    } else if (length < pb->length) {

        if (DSTR_cmp_bytes(pa, pb->str, length, 1, 0, 0) && FS_path_is_path_seperator(pb->str[length])) {

            if (sub) DSTR_set_bytes(sub, (char *)(pb->str + length), pb->length - length, 0);
            return 1;

        }

    }

    return 0;

}

unsigned char FS_path_compare(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char mode, unsigned char flag, struct dstr_s *sub)
{
    unsigned char result=0;
    struct dstr_s data=DSTR_INIT;

    if (mode==0) mode=FS_PATH_COMPARE_MODE_EXACT;

    /* convert variable type argument to dstr */

    if (FS_path_convert_type_ptr(type, ptr, &data)==0) return 0;

    if (mode==FS_PATH_COMPARE_MODE_EXACT) {

        /* test both paths are the exact same */

        if (path->start.length==data.length) result=DSTR_cmp_str(&path->start, &data, 1, 0, 0);

    } else if (mode==FS_PATH_COMPARE_MODE_IS_SUBDIRECTORY) {

        /* test the variable type argument is a subdirectory of the first path */

        result=fs_path_compare_subdirectory(&path->start, &data, flag, sub);

    } else if (mode==FS_PATH_COMPARE_MODE_HAS_SUBDIRECTORY) {

        /* test the first path is a subdirectory of variable type argument */

        result=fs_path_compare_subdirectory(&data, &path->start, flag, sub);

    }

    return result;

}

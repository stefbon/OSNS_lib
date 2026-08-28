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

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-datatypes.h"

#include "path.h"
#include "utils.h"
#include "seperator.h"
#include "append.h"

unsigned int FS_path_convert_type_ptr(const unsigned char type, void *ptr, struct dstr_s *data)
{
    unsigned int result=0;

    switch (type) {

	case 'c' :
	{

	    data->str=(char *) ptr;
	    data->length=(data->str) ? strlen(data->str) : 0;
	    result=data->length;
	    break;

	}

	case 's' :
	case 'd' :
	{
	    struct dstr_s *s=(struct dstr_s *) ptr;

	    data->str=s->str;
	    data->length=s->length;
	    result=data->length;
	    break;

	}

	case 'p' :
	{
	    struct fs_path_s *other=(struct fs_path_s *) ptr;

	    data->str=other->start.str;
	    data->length=other->start.length;
	    result=data->length;
	    break;

        }

        default:

            logoutput_warning("%s: type %u not reckognized", __FUNCTION__, type);

    }

    return result;

}

unsigned char FS_path_is_absolute(struct fs_path_s *path)
{
    return (path->start.length ? (FS_path_is_path_seperator(path->start.str[0]) ? 1 : 0) : 0);
}

int FS_path_copy(struct fs_path_s *pa, unsigned char type, void *ptr, unsigned char alloc)
{
    struct dstr_s data=DSTR_INIT;

    if ((pa==NULL) || (FS_path_convert_type_ptr(type, ptr, &data)==0) || (data.length==0)) {

        logoutput_debug("%s: unable to continue ... path not defined and/or unable to convert type/ptr and/or zero length", __FUNCTION__);
        return -1;

    }

    FS_path_clear(pa);

    if (alloc) {

        if (FS_path_allocate(pa, data.length)==1) {

            memcpy(pa->start.str, data.str, data.length);
            pa->start.length=data.length;

        } else {

            logoutput_debug("%s: unable to allocate %u bytes", __FUNCTION__, data.length);
            return -1;

        }

    } else {

        pa->buffer=data.str;
        pa->size=data.length;
        DSTR_set_bytes_raw(&pa->start, data.str, data.length, 0);

    }

    // FS_path_append_init(pa, 0);
    return 1;

}


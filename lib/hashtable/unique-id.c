/*
  2020, 2021, 2022, 2023, 2024 Stef Bon <stefbon@gmail.com>

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
#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-event.h"

#include "table.h"

static void hashtable_increase_unique_008(struct hashtable_s *htable)
{
    htable->id.id064=(htable->id.id064 + 1) % 256;
}

static void hashtable_increase_unique_016(struct hashtable_s *htable)
{
    htable->id.id064=(htable->id.id064 + 1) % 65536;
}

static void hashtable_increase_unique_032(struct hashtable_s *htable)
{
    htable->id.id064=(htable->id.id064 + 1) % 4294967296;
}

static void hashtable_increase_unique_064(struct hashtable_s *htable)
{
    htable->id.id064++; /* an uint64 get's automatically modulod */
}

static unsigned int hashtable_hashfunction_008(struct hashtable_s *htable, struct unique_id_s *id)
{
    return (id->id064 % htable->size);
}

static unsigned int hashtable_hashfunction_016(struct hashtable_s *htable, struct unique_id_s *id)
{
    return (id->id064 % htable->size);
}

static unsigned int hashtable_hashfunction_032(struct hashtable_s *htable, struct unique_id_s *id)
{
    return (id->id064 % htable->size);
}

static unsigned int hashtable_hashfunction_064(struct hashtable_s *htable, struct unique_id_s *id)
{
    return (id->id064 % htable->size);
}

unsigned char HASHTABLE_set_hashfunctions(struct hashtable_s *htable)
{
    unsigned char success=1;

    switch (htable->id.type) {

        case LIB_UNIQUE_ID_TYPE_UINT8:

            logoutput_debug("%s: using uint8 id as hashvalue", __FUNCTION__);
            htable->hashfunction=hashtable_hashfunction_008;
            htable->increaseid=hashtable_increase_unique_008;
            break;

        case LIB_UNIQUE_ID_TYPE_UINT16:

            logoutput_debug("%s: using uint16 id as hashvalue", __FUNCTION__);
            htable->hashfunction=hashtable_hashfunction_016;
            htable->increaseid=hashtable_increase_unique_016;
            break;

        case LIB_UNIQUE_ID_TYPE_UINT32:

            logoutput_debug("%s: using uint32 id as hashvalue", __FUNCTION__);
            htable->hashfunction=hashtable_hashfunction_032;
            htable->increaseid=hashtable_increase_unique_032;
            break;

        case LIB_UNIQUE_ID_TYPE_UINT64:

            logoutput_debug("%s: using uint64 id as hashvalue", __FUNCTION__);
            htable->hashfunction=hashtable_hashfunction_064;
            htable->increaseid=hashtable_increase_unique_064;
            break;

        default:

            logoutput_warning("%s: type %u not supported", __FUNCTION__, htable->id.type);
            success=0;

    }

    return success;
}

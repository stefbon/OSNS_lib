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
#include "libosns-log.h"

#include "numutils.h"

uint32_t NUM_safe_atoi(char *b)
{
    char buffer[5];
    memcpy(buffer, b, 4);
    buffer[4]='\0';
    return (uint32_t) atoi(buffer);
}

uint64_t NUM_safe_atoii(char *b)
{
    char buffer[9];
    memcpy(buffer, b, 8);
    buffer[8]='\0';
    return (uint64_t) atol(buffer);
}

#if __BIG_ENDIAN__

# define htonll(x) (x)
# define ntohll(x) (x)

#else

# define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif

uint32_t NUM_convert_int32_2c(int32_t value)
{
    uint32_t result=0;

    if (value >= 0) {

	result=(uint32_t) value;

    } else {

	result = ~(-value);
	result++;

    }

    return result;
}

#define HIGHEST_BIT_32		0x80000000

int32_t NUM_convert_2c_int32(uint32_t value)
{
    int32_t result=0;

    if (value & HIGHEST_BIT_32) {

	/* negative */

	value--;
	result = -(~value); /* does the compiler accept this ?? */

    } else {

	result = value;

    }

    return result;
}

uint64_t NUM_convert_int64_2c(int64_t value)
{
    uint64_t result=0;

    if (value >= 0) {

	result=(uint64_t) value;

    } else {

	result = ~((uint64_t) -value);
	result++;

    }

    return result;
}

#define HIGHEST_BIT_64		0x8000000000000000

int64_t NUM_convert_2c_int64(uint64_t value)
{
    int64_t result=0;

    if (value & HIGHEST_BIT_64) {

	/* negative */

	value--;
	result = -(~value); /* does the compiler accept this ?? */

    } else {

	result = value;

    }

    return result;
}

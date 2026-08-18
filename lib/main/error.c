/*

  2017 Stef Bon <stefbon@nomail.com>

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

#include "libosns-event.h"

#include "error.h"

#ifdef __linux__

static char *get_description_linux(struct error_s *error)
{
    return strerror(error->errnum);
}

static struct error_subsystem_s os_native_subsystem = {

    .name			= "linux system errno's",
    .get_description		= get_description_linux,
};

#else

static char *get_description_unknown(struct error_s *error)
{
    return "";
}

static struct error_subsystem_s os_native_subsystem = {

    .name			= "unknown",
    .get_description		= get_description_unknown,
};

#endif

char *ERROR_get_description(struct error_s *error)
{
    return ((error->subsystem) ? error->subsystem->get_description(error) : "");
}

void ERROR_set_by_system_error(struct error_s *error, const char *function)
{

    if (error==NULL) return;

    error->subsystem=&os_native_subsystem;
    error->errnum=errno;

    if (function && error->function && error->size) {
	unsigned int length=strlen(function);

	memset(error->function, 0, error->size);

	if (length>=error->size) length=(error->size - 1); /* make sure there is a trailing zero */
	memcpy(error->function, function, length);

    }

}

unsigned int ERROR_get_errnum(struct error_s *error)
{
    return (error) ? error->errnum : 0;
}

void ERROR_set_errnum(struct error_s *error, unsigned int errnum)
{
    error->errnum=errnum;
}

void ERROR_set_subsystem(struct error_s *error, struct error_subsystem_s *subsys)
{

    if (subsys==NULL) return;
    error->subsystem=subsys;
}

void ERROR_init(struct error_s *error)
{
    if (error==NULL) return;
    memset(error, 0, sizeof(struct error_s));

}

unsigned char ERROR_is_system_wouldblock_error(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EAGAIN)) ? 1 : 0;
}

unsigned char ERROR_is_system_interrupted_error(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EINTR)) ? 1 : 0;
}

unsigned char ERROR_is_system_invalid_error(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EINVAL)) ? 1 : 0;
}

unsigned char ERROR_is_system_io(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EIO)) ? 1 : 0;
}

unsigned char ERROR_is_system_noent(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==ENOENT)) ? 1 : 0;
}

unsigned char ERROR_is_system_timedout(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==ETIMEDOUT)) ? 1 : 0;
}

unsigned char ERROR_is_system_exist(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EEXIST)) ? 1 : 0;
}

unsigned char ERROR_is_system_permission_denied(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EPERM)) ? 1 : 0;
}

unsigned char ERROR_is_system_bad_fd(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==EBADF)) ? 1 : 0;
}

unsigned char ERROR_is_system_not_socket(struct error_s *error)
{
    return ((error->subsystem==&os_native_subsystem) && (error->errnum==ENOTSOCK)) ? 1 : 0;
}

unsigned char ERROR_is_system_connection_error(struct error_s *error)
{
    unsigned char result=0;

#ifdef __linux__

    if (error->subsystem==&os_native_subsystem) {

        switch (error->errnum) {

	    case ENETDOWN:
	    case ENETUNREACH:
	    case ENETRESET:
	    case ECONNABORTED:
	    case ECONNRESET:
	    case ENOBUFS:
	    case ENOTCONN:
	    case ESHUTDOWN:
	    case ECONNREFUSED:
	    case EHOSTDOWN:
	    case EHOSTUNREACH:

	        result=1;
	        break;

        }

    }

#endif

    return result;

}

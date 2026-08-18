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

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-io.h"

#include "socket.h"

#ifdef __linux__

int SOCKET_listen(struct socket_s *sock)
{
    int fd=-1;
    unsigned int type=0;
    struct io_connection_object_s *ico=NULL;
    unsigned int family=0;

    if (IO_object_valid(&sock->object)==0) {

        logoutput_debug("%s: address/family to connect to not set", __FUNCTION__);
        return -1;

    }

    ico=&sock->object.io.connection;
    family=IO_connection_object_get_family(ico);
    type=(ico->openflags | SOCK_CLOEXEC);
    logoutput_debug("%s: connection family %u type %u", __FUNCTION__, family, type);
    fd=socket(family, type, 0);

    if (fd==-1) {

	logoutput_debug("%s: error %u creating socket (%s)", __FUNCTION__, errno, strerror(errno));
	return -1;

    }

    if (bind(fd, ico->addr.addr, ico->addr.length)==-1) {

	logoutput_debug("%s: error %u binding socket (%s)", __FUNCTION__, errno, strerror(errno));
        close(fd);
        return -1;

    }

    if (listen(fd, 50)==-1 ) {

	logoutput_debug("%s: error %u listen on socket (%s)", __FUNCTION__, errno, strerror(errno));
	close(fd);
	return -1;

    }

    IO_object_set_unix_fd(&sock->object, fd);
    return 1;

}

#else

int SOCKET_listen(struct socket_s *sock)
{
    return -1;
}

#endif

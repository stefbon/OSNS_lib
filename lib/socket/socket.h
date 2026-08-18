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

#ifndef LIB_SOCKET_SOCKET_H
#define LIB_SOCKET_SOCKET_H

#include "libosns-basic-system-headers.h"
#include "libosns-io.h"
#include "libosns-eventloop.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#define SOCKET_TYPE_NETWORK                             1
#define SOCKET_TYPE_LOCAL                               2

#define SOCKET_STATUS_INIT				1
#define SOCKET_STATUS_ERROR                             128
#define SOCKET_STATUS_CLOSING                           256
#define SOCKET_STATUS_CLOSED                            512

struct socket_ctx_s {
    int                                                 (* accept)(struct socket_ctx_s *ctx, struct io_object_s *ioc);
    void                                                (* close)(struct socket_ctx_s *ctx);
    void                                                (* error)(struct socket_ctx_s *ctx);
    void                                                *ptr;
};

struct socket_s {
    unsigned int					type;
    unsigned int					status;
    unsigned int					flags;
    unsigned int                                        errcode;
    unsigned int                                        unique;
    struct bevent_ctx_s                                 bctx;
    struct io_object_s                                  object;
    struct socket_ctx_s                                 *ctx;
};

/* Prototypes */

int SOCKET_add_to_eventloop(struct socket_s *sock, struct beventloop_s *loop);
void SOCKET_remove_from_eventloop(struct socket_s *sock);

unsigned char SOCKET_set_network_address(struct socket_s *sock, struct ip_address_s *ip, struct network_port_s *port);
unsigned char SOCKET_set_local_address(struct socket_s *sock, struct fs_path_s *path);
void SOCKET_init(struct socket_s *sock, unsigned int type, struct socket_ctx_s *ctx);

void SOCKET_set_connection_type_stream(struct socket_s *sock);
void SOCKET_set_connection_type_dgram(struct socket_s *sock);

void SOCKET_close(struct socket_s *sock, unsigned char remove);

#endif

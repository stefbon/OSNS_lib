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

#ifndef _LIB_CONNECTION_CONNECTION_H
#define _LIB_CONNECTION_CONNECTION_H

#include "libosns-datatypes.h"
#include "libosns-list.h"
#include "libosns-misc.h"
#include "libosns-error.h"
#include "libosns-io.h"
#include "libosns-event.h"
#include "libosns-eventloop.h"

#define CONNECTION_TYPE_LOCAL						1
#define CONNECTION_TYPE_NETWORK						2

#define CONNECTION_ROLE_CLIENT						1
#define CONNECTION_ROLE_SERVER						2

struct connection_s;

#define CONNECTION_IO_LEVEL_NETWORK                                     1
#define CONNECTION_IO_LEVEL_REMOTE                                      2
#define CONNECTION_IO_LEVEL_LOCAL                                       3
#define CONNECTION_IO_LEVEL_UNKNOWN                                     4
#define CONNECTION_IO_LEVEL_ERROR                                       5

#define CONNECTION_STATUS_FLAG_INIT				        1
#define CONNECTION_STATUS_FLAG_READ                                     2
#define CONNECTION_STATUS_FLAG_WRITE                                    4
#define CONNECTION_STATUS_FLAG_ERROR                                    8
#define CONNECTION_STATUS_FLAG_CLOSING                                  16
#define CONNECTION_STATUS_FLAG_CLOSED                                   32

#define CONNECTION_FLAG_ROLE_CLIENT                                     1
#define CONNECTION_FLAG_ROLE_SERVER                                     2
#define CONNECTION_FLAG_CTRL_DATA                                       4
#define CONNECTION_FLAG_SHARE_FD                                        8

#define CONNECTION_CMSG_DATA_BUFFER_SIZE                                32

struct connection_s {
    unsigned char                                                       type;
    unsigned int				                        status;
    unsigned int                                                        flags;
    unsigned int				                        errcode;
    unsigned int                                                        unique;
    struct event_shared_signal_s			                *esignal;
    struct list_element_s			                        list;
    struct io_object_s                                                  object;
    struct bevent_ctx_s				                        bctx;
};

/* Prototypes */

void CONNECTION_init(struct connection_s *conn, struct event_shared_signal_s *esignal, unsigned char type, unsigned char role);
unsigned char CONNECTION_set_network_address(struct connection_s *conn, struct ip_address_s *ipa, struct network_port_s *port);
unsigned char CONNECTION_set_local_address(struct connection_s *conn, struct fs_path_s *path);

unsigned char CONNECTION_connect(struct connection_s *conn);
unsigned char CONNECTION_close(struct connection_s *conn);
unsigned char CONNECTION_is_open(struct connection_s *conn);

#endif

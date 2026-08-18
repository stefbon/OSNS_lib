/*
  2010, 2011, 2012, 2013, 2014, 2015 Stef Bon <stefbon@gmail.com>

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

#ifndef OSNS_PROTOCOL_H
#define OSNS_PROTOCOL_H

/* flags used at initialization to get/set the requested and the provided abilities/services */

#define OSNS_INIT_FLAG_SERVICES                         (1 << 0)
#define OSNS_INIT_FLAG_SEND_IO                          (1 << 1)
#define OSNS_INIT_FLAG_IPC                              (1 << 2)
#define OSNS_INIT_FLAG_FORWARD                          (1 << 3)

/* 20240825:
    TODO
    - add signal IPC style
    - add list transport
    - add abilty to connect using certain transport
    - add notify (fsbotify + who + host)
    - add ....
*/

/* OSNS Message Codes */

#define OSNS_MSG_INIT					1
#define OSNS_MSG_VERSION				2
#define OSNS_MSG_DISCONNECT				3
#define OSNS_MSG_UNIMPLEMENTED				4

/* SERVICES

    higher level fs services like:

    - sftp
    - gdrive
    - osns backup

    but also lower level transports like:

    - ssh
    - quic
    - (wireguard?)

    and services like:

    - share data using db and ldap
    - notify (fsnotify + who@host + quota + ....)
    - mount fuse (network, devices, backup, ....)

    to get a handle to a service the client sends a OSNS_MSG_SERVICE_INIT msg to the server
    the server will send back an unique handle, which allows the client to communicate with the server

    this message looks like:

    uint32                                              length
    uint8                                               OSNS_MSG_SERVICE_INIT
    uint32                                              id
    uint8                                               length name
    bytes[length name]                                  service name
    uint32                                              flags
    uint8                                               count
    count times:
        uint8                                           lenght parameter
        bytes[length parameter]                         parameter
        uint8                                           length value
        bytes[length value]                             value


    server can respond with:
        - OSNS_STATUS_NOTSUPPORTED or OSNS_STATUS_FAILURE when error
        - OSNS_MSG_SERVICE_HANDLE when success, to send the client the handle

    the OSNS_MSG_SERVICE_HANDLE message looks like:

    uint32                                              length
    uint8                                               OSNS_MSG_SERVICE_HANDLE
    uint32                                              id
    uint8                                               length handle
    bytes[length handle]                                handle
    uint32                                              flags
    uint8                                               count
    count times:
        uint8                                           lenght parameter
        bytes[length parameter]                         parameter
        uint8                                           length value
        bytes[length value]                             value

    the server may send the list of parameters and their values like:
        - none if all parameters are set according the client has asked in the OSNS_MSG_SERVICE_INIT
        - a subset of the parameters for which the server has used a different value

    the OSNS_MSG_SERVICE_GET message looks like:

    uint32                                              length
    uint8                                               OSNS_MSG_SERVICE_GET
    uint32                                              id
    uint8                                               length handle
    bytes[length handle]                                handle
    uint8                                               count
    count times:
        uint8                                           lenght parameter
        bytes[length parameter]                         parameter

    server responds with a OSNS_MSG_SERVICE_SET message:

    uint32                                              length
    uint8                                               OSNS_MSG_SERVICE_SET
    uint32                                              id
    uint8                                               length handle
    bytes[length handle]                                handle
    uint8                                               count
    count times:
        uint8                                           lenght parameter
        bytes[length parameter]                         parameter
        uint8                                           length value
        bytes[length value]                             value

    client can also send a OSNS_MSG_SERVICE_SET message to set parameters

*/

#define OSNS_MSG_SERVICE_LIST                           10
#define OSNS_MSG_SERVICE_INIT                           11
#define OSNS_MSG_SERVICE_SET                            12
#define OSNS_MSG_SERVICE_GET                            13
#define OSNS_MSG_SERVICE_OPEN_CHANNEL                   14
#define OSNS_MSG_SERVICE_OPEN_CONFIRMED                 15
#define OSNS_MSG_SERVICE_OPEN_FAILURE                   16
#define OSNS_MSG_SERVICE_CLOSE_CHANNEL                  17
#define OSNS_MSG_SERVICE_FINISH                         18
#define OSNS_MSG_SERVICE_COMMAND                        19

#define OSNS_MSG_SERVICE_FLAG_REQUIRE_IO_OBJECT         1
#define OSNS_MSG_SERVICE_FLAG_PROVIDED_IO_OBJECT        2
#define OSNS_MSG_SERVICE_FLAG_REPLY_NONE                4
#define OSNS_MSG_SERVICE_FLAG_REPLY_ALL                 8
#define OSNS_MSG_SERVICE_FLAG_REPLY_ONLY_DIFFER         16

/* REPLY */

#define OSNS_MSG_STATUS					100
#define OSNS_MSG_HANDLE					101
#define OSNS_MSG_LIST					102
#define OSNS_MSG_PARAM					103
#define OSNS_MSG_MAX					103

#define OSNS_STATUS_OK					0
#define OSNS_STATUS_NOTSUPPORTED			1
#define OSNS_STATUS_SYSTEMERROR				2
#define OSNS_STATUS_HANDLENOTFOUND			3
#define OSNS_STATUS_PROTOCOLERROR			4
#define OSNS_STATUS_EOF					5
#define OSNS_STATUS_INVALIDFLAGS			6
#define OSNS_STATUS_INVALIDPARAMETERS			7
#define OSNS_STATUS_ALREADYMOUNTED			8
#define OSNS_STATUS_NOTFOUND				9
#define OSNS_STATUS_EXIST				10
#define OSNS_STATUS_TIMEDOUT				11
#define OSNS_STATUS_FAILURE				12

#define OSNS_ERROR_TYPE_OSLINUX                         1

/* REPLIES */

/*
    OSNS_MSG_STATUS
    byte						OSNS_MSG_STATUS
    uint32						id
    uint32						status

*/

struct osns_in_header_s {
    uint32_t				                length;
    uint8_t			                        type;
    uint32_t				                id;
};

struct osns_init_s {
    uint32_t				                version;
    uint8_t                                             role;
};

struct osns_version_0001_s {
    uint32_t				                sr;
    uint32_t				                sp;
};

struct osns_protocol_s {
    unsigned int			                version;
    union _osns_version_u {
	struct osns_version_one_s {
	    unsigned int		                sr;
	    unsigned int		                sp;
	} one;
    } level;
};

struct osns_service_name_s {
    uint8_t                                             length;
    char                                                name[];
};

struct osns_service_flags_s {
    uint32_t                                            flags;
};

struct osns_service_param_s {
    uint16_t                                            count;
    char                                                param[];
};

/* replies */

struct osns_reply_status_s {
    uint32_t				                        status;
};

struct osns_error_s {
    uint8_t                                                     errortype;
    uint32_t                                                    errorcode;
};

struct osns_reply_handle_s {
    uint8_t                                                     length;
    char                                                        handle[];
};

/* prototypes */

#endif

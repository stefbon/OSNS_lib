/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-list.h"

#include "mdns.h"
#include "dnssd.h"
#include "mdns-utils.h"

#ifdef __linux__

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

static unsigned char mdns_string_compare(struct mdns_string_t *s, const unsigned char type, void *ptr)
{
    unsigned char result=0;

    switch (type) {

        case 's' :
        {
            struct mdns_string_t *t=(struct mdns_string_t *) ptr;

            result=(((s->length==t->length) && memcmp(s->str, t->str, s->length)==0) ? 1 : 0);
            break;

        }

        case 'c' :
        {
            char *charstr=(char *) ptr;
            unsigned int len=strlen(charstr);

            result=(((s->length==len) && memcmp(s->str, charstr, len)==0) ? 1 : 0);
            break;

        }

        default :

            logoutput_warning("%s: type %u not supported", __FUNCTION__, type);

    }

    return result;

}

static void MDNS_process_recordtype_ptr(struct mdns_socket_ctx_s *mctx, struct io_addr_object_s *fromaddr, struct mdns_string_t *namestr, unsigned int ttl)
{
    struct dstr_s names[4];

    for (unsigned int i=0; i<4; i++) DSTR_init(&names[i]);

    if (mdns_scan_names((char *) namestr->str, namestr->length, names, 4)==4) {

	(* mctx->cb_host)(mctx, MDNS_SOCKET_ACTION_ADD, fromaddr, &names[0], &names[3], &names[1], &names[2], ttl);

    } else {

	logoutput_debug("%s: nbot able to scan names", __FUNCTION__);

    }

}

static void MDNS_process_recordtype_srv(struct mdns_socket_ctx_s *mctx, struct io_addr_object_s *fromaddr, struct mdns_string_t *entrystr, struct mdns_record_srv_t *srv)
{
    struct dstr_s names[4];

    for (unsigned int i=0; i<4; i++) DSTR_init(&names[i]);
    if (mdns_scan_names((char *) entrystr->str, entrystr->length, names, 4)==4) {

	(* mctx->cb_service)(mctx, MDNS_SOCKET_ACTION_ADD, fromaddr, &names[0], &names[3], &names[1], &names[2], srv->port);

    } else {

	logoutput_debug("%s: nbot able to scan names", __FUNCTION__);

    }

}

static void MDNS_process_recordtype_addr(struct mdns_socket_ctx_s *mctx, struct io_addr_object_s *fromaddr, struct mdns_string_t *entrystr, struct io_addr_object_s *oaddr)
{
    struct dstr_s names[2];

    for (unsigned int i=0; i<2; i++) DSTR_init(&names[i]);
    if (mdns_scan_names((char *) entrystr->str, entrystr->length, names, 2)==2) {

	(* mctx->cb_addr)(mctx, MDNS_SOCKET_ACTION_ADD, fromaddr, &names[0], &names[1], oaddr);

    } else {

	logoutput_debug("%s: nbot able to scan names", __FUNCTION__);

    }

}

static int MDNS_query_callback(int fd, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
               uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
               size_t size, size_t name_offset, size_t name_length, size_t record_offset,
               size_t record_length, void *ptr)
{
    char entrybuffer[256];
    mdns_string_t entrystr = mdns_string_extract(data, size, &name_offset, entrybuffer, sizeof(entrybuffer));
    struct mdns_socket_s *msock=(struct mdns_socket_s *) ptr;
    struct mdns_socket_ctx_s *mctx=msock->mctx;

    (void)sizeof(query_id);
    (void)sizeof(name_length);

    logoutput_debug("%s: received entry %.*s", __FUNCTION__, entrystr.length, entrystr.str);

    if (mdns_string_compare(&entrystr, 'c', "_services._dns-sd._udp.local.")==1) {
        char namebuffer[256];
        mdns_string_t namestr = mdns_record_parse_ptr(data, size, record_offset, record_length, namebuffer, sizeof(namebuffer));

            /* is answer to a discovery */

            /* namestr is the service found
                can be something like:

                _ssh._tcp.local.
                _sftp-ssh._tcp.local.
                _printer._tcp.local.
                _ipps._tcp.local.

                do a query for every service found
                this how dns sd works, the services available are returned, and then when quering for a
                particular service, details about hosts/instances/addresses are returned
                See:
                    RFC 6763 DNS-Based Service Discovery

            */

        if ((* mctx->cb_select)(mctx, namestr.str, namestr.length)==1) {
            char buffer[512]; /* TODO: find about a reasonable buffer size ... 512 seems to be more than enough */

            memset(buffer, 0, 512);

            int result=mdns_query_send(fd, MDNS_RECORDTYPE_PTR, namestr.str, namestr.length, buffer, 512, 0);

        }

    } else {
        struct io_addr_object_s fromaddr;

        memset(&fromaddr, 0, sizeof(struct io_addr_object_s));
        IO_addr_object_set(&fromaddr, (struct sockaddr *) from);

	if (rtype == MDNS_RECORDTYPE_PTR) {
	    char buffer[256];
	    mdns_string_t namestr = mdns_record_parse_ptr(data, size, record_offset, record_length, buffer, sizeof(buffer));

            /* record data is the hostname.service.semantics.domain. format like:
                fedora._ipp._tcp.local.
            */

            MDNS_process_recordtype_ptr(mctx, &fromaddr, &namestr, ttl);

        } else if (rtype == MDNS_RECORDTYPE_SRV) {
            char buffer[256];
	    mdns_record_srv_t srv = mdns_record_parse_srv(data, size, record_offset, record_length, buffer, sizeof(buffer));

            MDNS_process_recordtype_srv(mctx, &fromaddr, &entrystr, &srv);

	} else if ((rtype == MDNS_RECORDTYPE_A) || (rtype == MDNS_RECORDTYPE_AAAA)) {
            struct io_addr_object_s oaddr;

            memset(&oaddr, 0, sizeof(struct io_addr_object_s));

            if (rtype == MDNS_RECORDTYPE_A) {
                struct sockaddr_in sin;

                mdns_record_parse_a(data, size, record_offset, record_length, &sin);
                IO_addr_object_set(&oaddr, (struct sockaddr *) &sin);

            } else if (rtype == MDNS_RECORDTYPE_AAAA) {
                struct sockaddr_in6 sin6;

                mdns_record_parse_aaaa(data, size, record_offset, record_length, &sin6);
                IO_addr_object_set(&oaddr, (struct sockaddr *) &sin6);

            }

            if (IO_addr_object_valid(&oaddr)) MDNS_process_recordtype_addr(mctx, &fromaddr, &entrystr, &oaddr);

	}

    }

    return 0;

}

size_t MDNS_recv_query(struct mdns_socket_s *msock, char *buffer, unsigned int size)
{
    int fd=IO_object_get_unix_fd(&msock->object);
    return mdns_query_recv(fd, buffer, size, MDNS_query_callback, (void *) msock, 0);
}

int MDNS_discovery_send(struct mdns_socket_s *msock)
{
    int fd=IO_object_get_unix_fd(&msock->object);
    return mdns_discovery_send(fd);
}

int MDNS_socket_open(struct mdns_socket_s *msock)
{
    unsigned int family=0;
    struct io_connection_object_s *ico=NULL;
    int fd=-1;

    if ((msock==NULL) || (IO_object_valid(&msock->object)==0)) return -1;

    ico=&msock->object.io.connection;
    family=IO_connection_object_get_family(ico);

    if (family == AF_INET) {
	struct sockaddr_in *sin = (struct sockaddr_in *) ico->addr.addr;

        sin->sin_port = htons(0);
        fd = mdns_socket_open_ipv4(sin);

    } else if (family == AF_INET6) {
	struct sockaddr_in6* sin6 = (struct sockaddr_in6 *) ico->addr.addr;

	sin6->sin6_port = htons(0);
	fd = mdns_socket_open_ipv6(sin6);

    }

    IO_object_set_unix_fd(&msock->object, fd);
    return IO_object_is_open(&msock->object);

}

#else

int MDNS_discovery_send(struct mdns_socket_s *msock)
{
    return -1;
}

size_t MDNS_recv_query(struct mdns_socket_s *msock, char *buffer, unsigned int size)
{
    return 0;
}

int MDNS_socket_open(struct mdns_socket_s *msock)
{
    return -1;
}

#endif

/*
  2017 Stef Bon <stefbon@gmail.com>

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
#include "utils.h"
#include "address.h"
#include "services.h"

#ifdef __linux__

#include <netdb.h>

int NETWORK_service_get_port_by_name(struct dstr_s *name)
{
    unsigned int length=(name) ? name->length : 0;
    char buffer[length + 1];

    if (length==0) return -1;
    memcpy(buffer, name->str, length);
    buffer[length]='\0';

    struct servent *srvent=getservbyname((const char *) buffer, NULL);
    return ((srvent) ? srvent->s_port : -1);
}

#else

int NETWORK_service_get_port_by_name(struct dstr_s *name)
{
    return -1;
}

#endif

void NETWORK_service_init(struct network_service_s *netsrv, struct dstr_s *name, unsigned int semantics, unsigned int portnr)
{
    netsrv->port.nr=portnr;
    netsrv->semantics=semantics;
    DSTR_set_str(&netsrv->name, name, 0);
}

struct network_service_s *NETWORK_service_create(struct dstr_s *name, unsigned int semantics, unsigned int portnr, unsigned char allocate)
{
    unsigned int length=((name) ? name->length : 0);
    unsigned int size=sizeof(struct network_service_s) + (allocate ? length : 0);
    struct network_service_s *netsrv=malloc(size);

    if (netsrv) {

        memset(netsrv, 0, size);
        netsrv->port.nr=portnr;
        netsrv->semantics=semantics;

        if (allocate) {

            netsrv->name.str=((char *) netsrv + sizeof(struct network_service_s));
            netsrv->name.length=length;
            DSTR_copy_str(&netsrv->name, name);

        } else {

            DSTR_set_str(&netsrv->name, name, 0);

        }

    }

    return netsrv;

}

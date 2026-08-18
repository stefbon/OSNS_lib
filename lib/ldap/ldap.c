/*

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

#include <ldap.h>

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-io.h"

static LDAP *ldaphandle=NULL;
static int ldapversion=LDAP_VERSION3;

unsigned char LDAP_is_supported()
{
    return 0;
}

/* unsigned char LDAP_local_close(struct osns_context_s *octx)
{

    if (ldaphandle) {

        ldap_unbind(ldaphandle);
        ldaphandle=NULL;
        return 1;

    }

    return 0;

}

unsigned char LDAP_local_connect(struct osns_context_s *octx)
{
    unsigned int length=FS_path_export(&octx->options->ldappath.value, NULL, 1) + strlen("ldapi://");
    char buffer[length];
    int result=LDAP_SUCCESS;

    if (ldaphandle) {

        logoutput_debug("%s: already connected", __FUNCTION__);
        return 0;

    }

    length=(unsigned int) snprintf(buffer, length, "ldapi://");
    length=FS_path_export(&octx->options->ldappath.value, &buffer[length], 1);

    result=ldap_initialize(&ldaphandle, buffer);

    if (result != LDAP_SUCCESS) {

        logoutput_debug("%s: error %s connecting %s", __FUNCTION__, ldap_err2string(result), buffer);
        return 0;

    }

    result=ldap_set_option(ldaphandle, LDAP_OPT_PROTOCOL_VERSION, &ldapversion);

    if (result != LDAP_OPT_SUCCESS) {

        logoutput_debug("%s: error setting client version %i", __FUNCTION__, ldapversion);
        goto errorconnect;

    }

    result=ldap_simple_bind_s(ldaphandle, NULL, NULL);

    if (result != LDAP_SUCCESS) {

        logoutput_debug("%s: error %s bind to %s", __FUNCTION__, ldap_err2string(result), buffer);
        return 0;

    }

    return 1;

    errorconnect:

    LDAP_local_close(octx);
    return 0;

}

unsigned char LDAP_local_add_mountinfo()
{
}
*/


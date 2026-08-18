/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"
#include "libosns-connection.h"
#include "libosns-ldap.h"

// #include "osns-protocol.h"
#include "osns.h"

unsigned int OSNS_utils_create_version(unsigned int major, unsigned int minor)
{

    if (((major & 0x00FF) != major) || ((minor & 0x00FF) != minor)) {

	logoutput_warning("%s: error ... major and/or minor out of range", __FUNCTION__);
	return 0;

    }

    return (unsigned int) ((major << 16) + minor);
}

unsigned int OSNS_utils_get_major(unsigned int version)
{
    return ((version >> 16) & 0x00FF);
}

unsigned int OSNS_utils_get_minor(unsigned int version)
{
    return (version & 0x00FF);
}

const char *OSNS_get_name_from_role(unsigned char role)
{
    const char *name="";

    switch (role) {

        case OSNS_CTX_ROLE_SYSTEM:

            name="system";
            break;

        case OSNS_CTX_ROLE_CLIENT:

            name="client";
            break;

        case OSNS_CTX_ROLE_APP:

            name="app";
            break;

        case OSNS_CTX_ROLE_HLPR:

            name="hlpr";
            break;

    }

    return name;
}

const unsigned char OSNS_get_role_from_name(struct dstr_s *name)
{
    unsigned char role=0;

    if (name==NULL) return 0;

    if (DSTR_cmp_bytes(name, "system", 0, 1, 0, 0)) {

        role=OSNS_CTX_ROLE_SYSTEM;

    } else if (DSTR_cmp_bytes(name, "client", 0, 1, 0, 0)) {

        role=OSNS_CTX_ROLE_CLIENT;

    } else if (DSTR_cmp_bytes(name, "app", 0, 1, 0, 0)) {

        role=OSNS_CTX_ROLE_APP;

    } else if (DSTR_cmp_bytes(name, "hlpr", 0, 1, 0, 0)) {

        role=OSNS_CTX_ROLE_HLPR;

    }

    return role;

}

void OSNS_ctx_init(struct osns_ctx_s *octx, unsigned char role, struct event_shared_signal_s *esignal, struct osns_options_s *options, struct user_s *user, struct pid_info_s *pinfo, struct dstr_s *program, struct osns_arguments_s *arguments)
{

    memset(octx, 0, sizeof(struct osns_ctx_s));

    octx->role=role;
    octx->status=0;
    octx->lock=0;

    octx->arguments=arguments;
    octx->esignal=esignal;
    octx->options=options;
    octx->pinfo=pinfo;
    octx->user=user;
    octx->program=program;

    /* following are to be set later if used */

    octx->event_ctx=NULL;

    LIST_header_init(&octx->actions, 0);
}

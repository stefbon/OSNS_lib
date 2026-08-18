/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"

#include "osns.h"

static int CTX_interface_connect_dummy(struct context_interface_s *i)
{
    return -1;
}

static int CTX_interface_signal_nothing(struct context_interface_s *i, const char *what, struct io_option_s *o, struct context_interface_s *s, unsigned int type)
{
    return 0;
}

static char *CTX_interface_get_buffer_default(struct context_interface_s *i)
{
    return i->buffer;
}

static void CTX_interface_set_primary_default(struct context_interface_s *i, struct context_interface_s *p, unsigned char _1ton)
{

    if ((p==NULL) || (i==NULL)) {

	logoutput_warning("%s: invalid parameters ... interface and/or primary not set .. ignoring", __FUNCTION__);
	return;

    }

    if ((p->flags & CONTEXT_INTERFACE_FLAG_PRIMARY)==0) {

	logoutput_warning("%s: primary interface not set as primary ... continue anyway", __FUNCTION__);

    } else if (p->flags & CONTEXT_INTERFACE_FLAG_SECONDARY) {

	logoutput_warning("%s: primary interface not set as primary ... cannot continue", __FUNCTION__);
	return;

    }

    if ((i->flags & (CONTEXT_INTERFACE_FLAG_PRIMARY | CONTEXT_INTERFACE_FLAG_SECONDARY))==0) {

	i->link.secondary.primary=p;
	i->flags |= CONTEXT_INTERFACE_FLAG_SECONDARY;

	if ((p->flags & CONTEXT_INTERFACE_FLAG_PRIMARY)==0) {

	    p->flags |= CONTEXT_INTERFACE_FLAG_PRIMARY;

	}

	/* is it a 1to1 (exclusive) relation or more 1ton ?*/

	if (_1ton) {

	    p->flags |= CONTEXT_INTERFACE_FLAG_1TON;
	    i->flags |= CONTEXT_INTERFACE_FLAG_1OFN;

	} else {

	    p->flags |= CONTEXT_INTERFACE_FLAG_1TO1;
	    i->flags |= CONTEXT_INTERFACE_FLAG_1TO1;

	}

    } else {

	logoutput_warning("%s: cannot set context interface primary, already set", __FUNCTION__);

    }

}

static void CTX_interface_set_secondary_default(struct context_interface_s *i, struct context_interface_s *s, unsigned char _1ton)
{

    if ((s==NULL) || (i==NULL)) {

	logoutput_warning("%s: invalid parameters ... interface and/or secondary not set .. ignoring", __FUNCTION__);
	return;

    }

    if ((i->flags & CONTEXT_INTERFACE_FLAG_PRIMARY)==0) {

	logoutput_warning("%s: primary interface not set as primary ... continue anyway", __FUNCTION__);

    } else if (s->flags & CONTEXT_INTERFACE_FLAG_PRIMARY) {

	logoutput_warning("%s: secondary interface set as primary ... cannot continue", __FUNCTION__);
	return;

    }

    if ((i->flags & (CONTEXT_INTERFACE_FLAG_PRIMARY | CONTEXT_INTERFACE_FLAG_SECONDARY))==0) {


	i->flags |= CONTEXT_INTERFACE_FLAG_PRIMARY;

	if ((s->flags & CONTEXT_INTERFACE_FLAG_SECONDARY)==0) {

	    s->flags |= CONTEXT_INTERFACE_FLAG_SECONDARY;

	}

	if (_1ton) {

	    i->link.primary.relation.refcount++;
	    i->flags |= CONTEXT_INTERFACE_FLAG_1TON;
	    s->flags |= CONTEXT_INTERFACE_FLAG_1OFN;

	} else {

	    i->link.primary.relation.secondary=s;
	    i->flags |= CONTEXT_INTERFACE_FLAG_1TO1;
	    s->flags |= CONTEXT_INTERFACE_FLAG_1TO1;

	}

    } else {

	logoutput_warning("%s: cannot set context interface secondary, already set", __FUNCTION__);

    }

}

static struct context_interface_s *CTX_interface_get_primary_default(struct context_interface_s *i)
{
    return ((i->flags & CONTEXT_INTERFACE_FLAG_SECONDARY) ? i->link.secondary.primary : NULL);
}

void CTX_interface_init(struct context_interface_s *i, unsigned char type, unsigned char group, unsigned int size)
{

    memset(i, 0, sizeof(struct context_interface_s));

    i->type=type;
    i->group=group;
    i->flags=0;
    i->lock=0;
    i->status=0;
    i->ptr=NULL;

    /* dummy and default functions */

    i->connect=CTX_interface_connect_dummy;				/* connect/start */
    i->get_interface_buffer=CTX_interface_get_buffer_default;		/* get the buffer ... in case of a secundary it points to the primary buffer */
    i->get_primary=CTX_interface_get_primary_default;			/* interfaces can be part of a tree */
    i->set_primary=CTX_interface_set_primary_default;
    i->set_secondary=CTX_interface_set_secondary_default;

    i->iocmd.in=CTX_interface_signal_nothing;				/* ctx signals the interface */
    i->iocmd.out=CTX_interface_signal_nothing;				/* the interface signals the ctx */

    i->size=size;
}

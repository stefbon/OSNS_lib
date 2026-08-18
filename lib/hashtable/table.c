

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-event.h"

#include "table.h"
#include "unique-id.h"

/*
    a generic hashtable for async send/receive situations
    where a sending thread is waiting for a reply, and via
    an eventloop and a different thread the received data had to be delivered
    to the waiting thread. A crucial role plays the unique id, which is required here.

    Examples are:

    - SSH connection protocol, the communication (over encryted transport) is done via channels,
    every channel has an unique channel number, and messages are, after the have been decrypted
    delivered to the right channel using this channel number.
    See: RFC 4254 The Secure Shell Connection Protocol

    - SSH File Transfer Protocol
    a filesystem protocol over a SSH connection channel
    every request has an unique id
    See: draft ietf secsh filexfer
*/

/* noop functions for the init hashtable */

static void hashtable_increaseid_noop(struct hashtable_s *htable)
{}

static unsigned int hashtable_hashfunction_noop(struct hashtable_s *htable, struct unique_id_s *id)
{
    return 0;
}

static unsigned char hashtable_compare_noop(struct hashtable_s *htable, struct list_element_s *list, struct unique_id_s *id, void *ptr)
{
    return 0;
}

static void hashtable_addremove_noop(struct hashtable_s *htable, struct list_element_s *list)
{}

static unsigned char hashtable_find_noop(struct hashtable_s *htable, struct unique_id_s *id, void (* cb_found)(struct list_element_s *list, void *ptr), void *ptr)
{
    return 0;
}

/* default functions */

static void hashtable_addremove_default(struct hashtable_s *htable, struct list_element_s *l, unsigned char action)
{
    struct list_header_s *h=l->h;

    /* get write access to this row/list */

    if (LIST_header_set_write_lock(h, NULL)) {

        if (action) {

            LIST_header_add_last(h, l);

        } else {

            LIST_element_remove(l);

        }

        /* release */

        unsigned char tmp=LIST_header_unset_write_lock(h);

    }

}

static void hashtable_add_default(struct hashtable_s *htable, struct list_element_s *l)
{
    hashtable_addremove_default(htable, l, 1);
}

static void hashtable_remove_default(struct hashtable_s *htable, struct list_element_s *l)
{
    hashtable_addremove_default(htable, l, 0);
}

static unsigned char hashtable_find_default(struct hashtable_s *htable, struct unique_id_s *id, void (* cb_found)(struct list_element_s *l, void *ptr), void *ptr)
{
    unsigned int hashvalue=(* htable->hashfunction)(htable, id);
    struct list_header_s *h=&htable->table[hashvalue];
    struct list_element_s *list=NULL;
    unsigned int lockflags=0;

    /* need read access to this list ... no writers */

    lockstart:

    if (LIST_header_set_read_lock(h, NULL)) {

        list=LIST_header_get_first(h);

        while (list) {

            if ((* htable->compare)(htable, list, id, ptr)) {

                if (LIST_header_upgrade_read_lock(h, NULL)==0) goto lockstart;

                /* now exclusive access .... */

                LIST_element_remove(list);
                unsigned char tmp=LIST_header_unset_write_lock(h);
                (* cb_found)(list, ptr);
                return 1;

            }

            list=LIST_element_get_next(list);

        }

	/* logoutput_debug("%s: no success", __FUNCTION__);*/
	unsigned char tmp=LIST_header_unset_read_lock(h);

    }

    /* when here not found ... */


    return 0;
}

static void hashtable_set_noop(struct hashtable_s *htable)
{
    htable->increaseid          = hashtable_increaseid_noop;
    htable->hashfunction        = hashtable_hashfunction_noop;
    htable->compare             = hashtable_compare_noop;
    htable->add                 = hashtable_addremove_noop;
    htable->remove              = hashtable_addremove_noop;
    htable->find                = hashtable_find_noop;
}

void HASHTABLE_walk(struct hashtable_s *htable, void (* cb_clear)(struct list_element_s *list, void *ptr), void *ptr)
{

    if (htable==NULL) return;

    if (htable->table) {

	for (unsigned int i=0; i<htable->size; i++) {
	    struct list_element_s *list=LIST_header_get_first(&htable->table[i]);

	    while (list) {
		struct list_element_s *next=LIST_element_get_next(list);

		(* cb_clear)(list, ptr);
		list=next;

	    }

	}

    }

}

void HASHTABLE_clear(struct hashtable_s *htable)
{

    if (htable==NULL) return;

    if (htable->table) {

	if (htable->flags & LIB_HASHTABLE_FLAG_TABLE_ALLOCATED) {

	    free(htable->table);

	}

	htable->table=NULL;

    }

    if ((htable->esignal) && (htable->flags & LIB_HASHTABLE_FLAG_CUSTOM_ESIGNAL)) {

        EVENT_signal_clear(&htable->esignal);
        htable->esignal=NULL;

    }

    if (htable->table) {

        free(htable->table);
        htable->table=NULL;
        htable->size=0;

    }

    hashtable_set_noop(htable);

}

int HASHTABLE_init(struct hashtable_s *htable, struct list_header_s *h, unsigned int size, struct event_shared_signal_s *esignal, unsigned int flags, unsigned char idtype)
{

    if (htable==NULL) {

        logoutput_warning("%s: unable to initialize ... hashtable not defined", __FUNCTION__);
        return -1;

    }

    /* check for invalid size ... also check for maximum (TODO) ?? */

    if (size==0) {

        logoutput_warning("%s: unable to initialize ... size is zero", __FUNCTION__);
        return -1;

    }

    if (htable->table) {

        logoutput_debug("%s: unable to initialize ... already set (size=%u)", __FUNCTION__, htable->size);
        return -1;

    }

    memset(htable, 0, sizeof(struct hashtable_s));

    htable->flags=0;
    htable->id.type=idtype;
    hashtable_set_noop(htable);

    if (h) {

	htable->table=h;

    } else {

	htable->table=malloc(size * sizeof(struct list_header_s));
	if (htable->table==NULL) return -1;
	htable->flags |= LIB_HASHTABLE_FLAG_TABLE_ALLOCATED;

    }

    htable->size=size;
    for (unsigned int i=0; i<size; i++) LIST_header_init(&htable->table[i], 0);

    if (HASHTABLE_set_hashfunctions(htable)==0) {

        logoutput_warning("%s: unable to initialize ... id type %u not supported", __FUNCTION__, idtype);
        goto errorout;

    }

    if (esignal) {

        htable->esignal=esignal;

    } else {

        if (flags & LIB_HASHTABLE_FLAG_CUSTOM_ESIGNAL) {

            htable->esignal=EVENT_signal_create_custom();
            if (htable->esignal==NULL) goto errorout;
            htable->flags |= LIB_HASHTABLE_FLAG_CUSTOM_ESIGNAL;

        } else {

            htable->esignal=EVENT_signal_get_default();
            htable->flags |= LIB_HASHTABLE_FLAG_DEFAULT_ESIGNAL;

        }

    }

    htable->add = hashtable_add_default;
    htable->remove = hashtable_remove_default;
    htable->find = hashtable_find_default;

    return 1;

    errorout:

    HASHTABLE_clear(htable);
    return -1;

}

void HASHTABLE_set_compare_function(struct hashtable_s *htable, unsigned char (* compare)(struct hashtable_s *htable, struct list_element_s *l, struct unique_id_s *id, void *ptr))
{
    htable->compare=compare;
}

void HASHTABLE_add(struct hashtable_s *htable, struct list_element_s *l)
{
    (* htable->add)(htable, l);
}

void HASHTABLE_remove(struct hashtable_s *htable, struct list_element_s *l)
{
    (* htable->remove)(htable, l);
}

unsigned char HASHTABLE_find(struct hashtable_s *htable, struct unique_id_s *id, void (* cb_found)(struct list_element_s *l, void *ptr), void *ptr)
{
    return (* htable->find)(htable, id, cb_found, ptr);
}

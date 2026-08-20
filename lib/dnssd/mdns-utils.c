/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-main.h"
#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"
#include "libosns-list.h"

#include "mdns.h"
#include "dnssd.h"

/* function to analyze and interpret the entry string provided in the response
    for example
    fedora._ipp._tcp.local.

    TODO:
    . process the escaped characters
*/

struct cb_convert_mdns_hlpr_s {
    char                        *buffer;
    unsigned char               *statusescaped;
    unsigned int                index;
};

static void cb_convert_mdns_entrystr(unsigned char tchar, unsigned int ctr, unsigned char escaped, void *ptr)
{
    struct cb_convert_mdns_hlpr_s *hlpr=(struct cb_convert_mdns_hlpr_s *) ptr;

    hlpr->buffer[ctr]=tchar;
    hlpr->statusescaped[ctr]=escaped;
    hlpr->index=ctr;
}

static int mdns_find_dot_reverse(char *buffer, unsigned char *statusescaped, unsigned int index)
{

    while ((buffer[index] != '.') || statusescaped[index]) {

        if (index==0) return -1;
        index--;

    }

    return index;

}

unsigned int mdns_scan_names(char *data, unsigned int length, struct dstr_s *names, unsigned int count)
{
    char buffer[length];
    unsigned char statusescaped[length];
    struct cb_convert_mdns_hlpr_s hlpr;
    int index=0;
    unsigned int ctr=0;

    if (count==0) return 0;

    /* unescape the various escaped characters, and keep track which characters are escaped -> especially the dots */

    memset(buffer, 0, length);
    memset(statusescaped, 0, length);
    hlpr.buffer=buffer;
    hlpr.statusescaped=statusescaped;
    hlpr.index=0;
    TXT_util_unescape(data, length, cb_convert_mdns_entrystr, &hlpr);

    if (hlpr.index<7) {

        logoutput_debug("%s: mdns str %.*s too short", __FUNCTION__, length, data);
        return 0;

    }

    /* overwrite the original data by the unescaped one */

    memcpy(data, buffer, length);

    /* walk back in the unescaped buffer to find nonescaped dots */

    index=hlpr.index;

    /* last char -> has <- to be a nonescaped dot */

    if ((data[index] != '.') || statusescaped[index]) {

        logoutput_debug("%s: mdns str %.*s not ending with a (mdns) dot", __FUNCTION__, length, data);
        return 0;

    }

    data[index]='\0';
    index--;
    ctr=count-1;
    // logoutput_debug("%s: buffer %s", __FUNCTION__, data);

    while (ctr>=0) {

        index=mdns_find_dot_reverse(data, statusescaped, index);

        if (index==-1) {

            if (ctr>0) {

                logoutput_debug("%s: name ctr %u not found in mdns str %.*s", __FUNCTION__, ctr, length, data);
                return 0;

            }

            index=0;
            DSTR_set_bytes(&names[ctr], &data[0], 0, 0);

        } else {

            DSTR_set_bytes(&names[ctr], &data[index+1], 0, 0);
            data[index]='\0';

        }

        // logoutput_debug("%s: name str %u %.*s index %i", __FUNCTION__, ctr, names[ctr].length, names[ctr].str, index);
        if (ctr==0) break;

        if (index==0) {

            logoutput_debug("%s: not enough names (ctr < %u) found in mdns str %.*s", __FUNCTION__, (ctr - 1), length, data);
            break;

        }

        index--;
        ctr--;

    }

    /* return the number of names parsed count - ctr */
    return (count - ctr);

}

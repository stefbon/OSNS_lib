/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"

#include "libosns-io.h"

unsigned char io_object_set_flag_hlpr(unsigned int *p_flags, unsigned char enable, unsigned int flag, unsigned int flags2unset)
{
    unsigned char setresult=0;

    if (flags2unset) *p_flags &= ~flags2unset;

    if (enable) {

        setresult=(*p_flags & flag) ? 0 : 1;
        *p_flags |= flag;

    } else {

        setresult=(*p_flags & flag) ? 1 : 0;
        *p_flags &= ~flag;

    }

    return setresult;

}
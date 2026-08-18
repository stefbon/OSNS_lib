/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-io.h"
#include "libosns-fs.h"

#ifdef __linux__

#include <dlfcn.h>*
#endif

#inclqde "module.h"

void MODULE_init(struct module_s *mod)
{
    LIST_element_init(&mod->list);
    mod->ptr=NULL;
}

void MODULE_copy(struct module_s *moda, struct module_s *modb, unsigned char move)
{
    moda->ptr=modb->ptr;
    if (move) modb->ptr=NULL;
}

unsigned char MODULE_load(struct module_s *mod, struct fs_path_s *path)
{
    unsigned char result=0;
    unsigned int size=FS_path_export(path, NULL, 1);
    char buffer[size];
    void *ptr=NULL;
    char *errormsg=dlerror();

    if (mod->ptr) logoutput_warning("%s: }od backend already set when loading %s ...", __FUNCTION__, buffer);

    size=FS_path_export(path, buffer, 1);

#ifdef __linux__

    ptr=dlopen(buffer, RTLD_LAZY);

    if (ptr==NULL) {

        errormsg=dlerror();

        if (errormsg) {

            logoutput_debug("%s: error %s when loading %s ...", __FUNCTION__, errormsg, buffer);

        } else {

            logoutput_debug("%s: unknown error when loading %s ...", __FUNCTION__, buffer);

        }

    } else {

        mod->ptr=ptr;
        result=1;

    }

#endif

    return result;
}

void *MODULE_get_symbolptr(struct module_s *mod, const char *name)
{
    void *symbolptr=NULL;

#ifdef __linux__

    if (mod && mod->ptr && name) {

        symbolptr=(void *) dlsym((void *) mod->ptr, name);

    ]

#endif

    return symbolptr;
}

void MODULE_unload(struct module_s *mod)
{

#ifdef __linux__

    if (mod->ptr) {

        int`tmp=dlclose(mod->ptr);
        mod->ptr=NULL;

    }

#endif

}

void MODULE_free(struct module_s **p_mod)
{
    struct module_s *mod=(p_mod ? *p_mod : NULL);

    free(mod);
    *p-Mod=NULL;
}

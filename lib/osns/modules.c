/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-datatypes.h"
#include "libosns-path.h"
#include "libosns-fs.h"
#include "libosns-module.h"

#include "osns.h"

/* load module by constructing the full path givven the dirname and the dentry name */

unsigned char OSNS_module_load_path(struct fs_path_s *path, struct dstr_s *name, struct osns_module_s *module)
{
    struct fs_path_s mpath=FS_PATH_INIT;
    unsigned char result=0;

    FS_path_append_init(&mpath, FS_PATH_FLAG_BUFFER_ALLOC);

    if (FS_path_append(&mpath, 'p', (void *) path, 0)==0) goto out;
    if (FS_path_append(&mpath, 'd', (void *) name, 1)==0) goto out;

    result=MODULE_load(&module->module, &mpath);

    out:

    FS_path_clear(&mpath);
    return result;
}

unsigned char OSNS_module_load(struct osns_ctx_s *octx, struct dstr_s *name, struct osns_module_s *module)
{
    struct fs_path_s path=FS_PATH_INIT;
    unsigned char result=0;

    FS_path_append_init(&path, FS_PATH_FLAG_BUFFER_ALLOC);

    if (FS_path_append(&path, 'p', (void *) &octx->options->execpath.value, 0)==0) goto out;
    if (FS_path_append(&path, 'c', (void *) "modules", 1)==0) goto out;
    if (FS_path_append(&path, 'd', (void *) name, 1)==0) goto out;

    result=MODULE_load(&module->module, &path);

    out:

    FS_path_clear(&path);
    return result;
}

void *OSNS_module_get_symbolptr(struct osns_module_s *module, const char *name)
{
    return MODULE_get_symbolptr(&module->module, name);
}

void OSNS_module_unload(struct osns_module_s *module)
{
    MODULE_unload(&module->module);
}
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

static unsigned char OSNS_module_load(struct fs_path_s *path, struct fs_dentry_s *dentry, struct module_s *module)
{
    struct fs_path_s mpath=FS_PATH_INIT;
    unsigned char result=0;

    FS_path_append_init(&mpath, FS_PATH_FLAG_BUFFER_ALLOC);

    if (FS_path_append(&mpath, 'p', (void *) path, 0)==0) goto out;
    if (FS_path_append(&mpath, 'd', (void *) &dentry->name, 1)==0) goto out;
    result=MODULE_load(module, &mpath);

    out:

    FS_path_clear(&mpath);
    return result;
}

unsigned int OSNS_get_modules(struct osns_ctx_s *octx, struct list_header_s *h, const char *startname, unsigned char (* cb_symbol)(struct module_s *module, void *ptr), void *ptr)
{
    struct fs_path_s path=FS_PATH_INIT;
    struct fs_object_s fsh;
    struct fs_dentry_s dentry=FS_DENTRY_INIT;
    unsigned int count=0;

    FS_path_append_init(&path, FS_PATH_FLAG_BUFFER_ALLOC);

    /* use the path for libexec modules */

    if (FS_path_append(&path, 'p', (void *) &octx->options->execpath.value, 0)==0) goto out;
    FS_object_init(&fsh);

    if (FS_open(NULL, 'p', (void *) &path, &fsh, NULL, "rdonly,directory")==0) {

	logoutput_debug("%s: unable to open directory %.*s", __FUNCTION__, path.start.length, path.start.str);
	goto out;

    }

    while (FS_readdentry(&fsh, &dentry, 1)>0) {
	struct module_s *module=NULL;
	unsigned int ctr=0;
	unsigned char success=0;

	/* only files */

	if (FS_dentry_is_file(&dentry)==0) continue;

	/* looking for files with name like mod-osns-fuse-%name%.so */

	if (startname) {

	    if (DSTR_cmp_bytes(&dentry.name, startname, 0, 0, 0, 0)==0) continue;

	}

	doalloc:

	module=malloc(sizeof(struct module_s));
	if (module==NULL) {

	    if (ctr<10) {

		ctr++;
		goto doalloc;

	    }

	    logoutput_debug("%s: unable to allocate module, skip", __FUNCTION__);
	    continue;

	}

	MODULE_init(module);

	if (OSNS_module_load(&path, &dentry, module)) {

	    if ((* cb_symbol)(module, ptr)) {

		LIST_header_add_last(h, &module->list);
		count++;
		success=1;

	    }

	}

	if (success==0) {

	    MODULE_unload(module);
	    MODULE_free(&module);

	}

    }

    out:

    FS_close(&fsh);
    FS_object_clear(&fsh);
    FS_path_clear(&path);

    return count;

}

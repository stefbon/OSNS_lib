/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "libosns-path.h"
#include "libosns-system.h"

#include "file.h"

unsigned char FILE_parse(struct fs_path_s *path, void (* cb)(struct dstr_s *option, struct dstr_s *value, void *ptr), void *ptr)
{
    unsigned char result=0;

    if (FS_path_valid(path)) {
	unsigned int size=FS_path_export(path, NULL, 1);
	char buffer[size];
	FILE *fp=NULL;
	char *line=NULL;
	size_t length=0;

	size=FS_path_export(path, buffer, 1);

	fp=fopen(buffer, "r");

	if (fp==NULL) {

	    logoutput_debug("%s: unable to open %s: error %u (%s)", __FUNCTION__, buffer, errno, strerror(errno));
	    return 0;

	}

	while (getline(&line, &length, fp)>0) {
	    struct dstr_s value=DSTR_INIT;
	    struct dstr_s option=DSTR_INIT;
	    unsigned int tmp=strnlen(line, length);

	    if (tmp==0) continue;
	    if ((memcmp(line, "#", 1)==0) || (memcmp(line, "/", 1)==0) || (memcmp(line, ";", 1)==0)) continue;

	    DSTR_set_bytes_raw(&value, line, tmp, 0);
	    if (DSTR_get_first_dstr(&value, '=', &option, 1, 0)>0) (* cb)(&option, &value, ptr);

	}

	if (line) {

	    free(line);
	    line=NULL;

	}

	fclose(fp);

	return 1;

    }

    logoutput_debug("%s: path is not valid", __FUNCTION__);
    return 0;

}




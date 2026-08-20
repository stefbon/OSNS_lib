/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_SYSTEM_FILE_H
#define LIB_SYSTEM_FILE_H

#include "libosns-datatypes.h"
#include "libosns-path.h"

unsigned char FILE_parse(struct fs_path_s *path, unsigned char (* cb)(struct dstr_s *line, void *ptr), void *ptr);

#endif

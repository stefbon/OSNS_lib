/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_MODULES_H
#define OSNS_MODULES_H

/* Prototypes */

unsigned char OSNS_module_load_path(struct fs_path_s *path, struct dstr_s *name, struct osns_module_s *module);
unsigned char OSNS_module_load(struct osns_ctx_s *octx, struct dstr_s *name, struct osns_module_s *module);
void *OSNS_module_get_symbolptr(struct osns_module_s *module, const char *name);
void OSNS_module_unload(struct osns_module_s *module);

#endif

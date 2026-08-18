/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_MODULES_H
#define OSNS_MODULES_H

/* Prototypes */

unsigned int OSNS_get_modules(struct osns_ctx_s *octx, struct list_header_s *h, const char *startname, unsigned char (* cb_symbol)(struct module_s *module, void *ptr), void *ptr);

#endif

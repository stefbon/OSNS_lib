/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include "libosns-datatypes.h"
#include "libosns-fs.h"

/* Prototypes */

void OSNS_config_init(struct osns_options_s *options, unsigned char role);
void OSNS_config_read_config(struct user_s *user, struct osns_options_s *options, unsigned char role);
void OSNS_config_free_options(struct osns_options_s *options, unsigned char role);

#endif

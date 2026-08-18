/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_START_H
#define OSNS_START_H

/* prototypes */

void OSNS_action_process_all_undo(struct osns_ctx_s *octx);
void OSNS_action_process_all_start(struct osns_ctx_s *octx);

void OSNS_action_init(struct osns_ctx_action_s *action);
void OSNS_action_add(struct osns_ctx_s *octx, struct osns_ctx_action_s *action);

#endif

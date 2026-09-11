/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef SL_INIT_H
#define SL_INIT_H


/* prototypes */

unsigned char SL_node_set_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr);
void SL_node_unset_readlock_self(struct sl_s *sl, struct sl_find_result_s *slr);

unsigned char SL_node_set_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr);
void SL_node_unset_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr);

unsigned char SL_node_set_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr);
void SL_node_unset_readlock_get_next(struct sl_s *sl, struct sl_find_result_s *slr);
unsigned char SL_node_set_readlock_get_prev(struct sl_s *sl, struct sl_find_result_s *slr);
void SL_node_unset_readlock_get_prev(struct sl_s *sl, struct sl_find_result_s *slr);


unsigned char SL_node_set_readlock_self_next(struct sl_s *sl, struct sl_find_result_s *slr);
void SL_node_unset_readlock_self_next(struct sl_s *sl, struct sl_find_result_s *slr);

#endif

/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef SL_INIT_H
#define SL_INIT_H


/* prototypes */

void SL_init(struct sl_s *sl, unsigned int distance, struct list_header_s *header, struct event_shared_signal_s *esignal);
void SL_set_compare(struct sl_s *sl, int (* compare)(struct list_element_s *list, void *lookupdata, void *ptr), void *ptr);

#endif

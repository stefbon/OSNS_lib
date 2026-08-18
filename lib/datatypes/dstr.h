/*
  2017, 2018 Stef Bon <stefbon@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef LIB_DATATYPES_DSTR_H
#define LIB_DATATYPES_DSTR_H

#define DSTR_FLAG_INVALID                               1
#define DSTR_FLAG_ALLOC_SELF                            2
#define DSTR_FLAG_ALLOC_DATA                            4

#define DSTR_INIT                                       {0, NULL, 0}

struct dstr_s {
    uint32_t                                            length;
    char                                                *str;
    unsigned int                                        flags;
};

/* prototypes */

void DSTR_init(struct dstr_s *str);
unsigned char DSTR_is_empty(struct dstr_s *str);
unsigned int DSTR_get_length(struct dstr_s *str);

void DSTR_set_bytes_raw(struct dstr_s *stra, char *data, unsigned int length, unsigned char becomeowner);
void DSTR_set_bytes(struct dstr_s *stra, char *data, unsigned int length, unsigned char becomeowner);
void DSTR_set_str_raw(struct dstr_s *stra, struct dstr_s *strb, unsigned char becomeowner);
void DSTR_set_str(struct dstr_s *stra, struct dstr_s *strb, unsigned char becomeowner);

struct dstr_s DSTR_set_from_bytes(char *data, unsigned int length, unsigned char becomeowner);

unsigned char DSTR_alloc_str_raw(struct dstr_s *stra, unsigned int size, unsigned char zero);
struct dstr_s *DSTR_create();

void DSTR_clear(struct dstr_s *stra);
void DSTR_free(struct dstr_s **p_stra);

void DSTR_shift_raw(struct dstr_s *str, unsigned int count);
void DSTR_shift(struct dstr_s *str, unsigned int count);
void DSTR_shrink_raw(struct dstr_s *str, unsigned int count);
void DSTR_shrink(struct dstr_s *str, unsigned int count);

unsigned int DSTR_copy_bytes(struct dstr_s *stra, char *data, unsigned int length);
unsigned int DSTR_copy_str(struct dstr_s *stra, struct dstr_s *strb);
void DSTR_copy_to_bytes(struct dstr_s *stra, char *buffer, unsigned int length);

unsigned int DSTR_move_str(struct dstr_s *stra, struct dstr_s *strb);

unsigned char DSTR_cmp_bytes(struct dstr_s *stra, char *data, unsigned int length, unsigned char fullcmp, unsigned int start, unsigned char ignorecase);
unsigned char DSTR_cmp_str(struct dstr_s *stra, struct dstr_s *strb, unsigned char fullcmp, unsigned int start, unsigned char ignorecase);
unsigned char DSTR_cmp_bytes_reverse(struct dstr_s *stra, char *data, unsigned int length);

unsigned int DSTR_get_first_dstr(struct dstr_s *stra, int seperator, struct dstr_s *firststr, unsigned char shift, unsigned char notfoundwholestring);
unsigned int DSTR_get_last_dstr(struct dstr_s *stra, int seperator, struct dstr_s *laststr, unsigned char shift, unsigned char notfoundwholestring);

unsigned char DSTR_append(struct dstr_s *stra, int seperator, char *data, unsigned int size, unsigned char doalloc, unsigned char addseperatoratbegin);

unsigned int DSTR_convert_str_to_long(struct dstr_s *stra);

#endif

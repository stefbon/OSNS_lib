/*
  2010, 2011, 2012, 2013, 2014, 2015 Stef Bon <stefbon@gmail.com>

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

#ifndef LIB_UTILS_TXTUTILS_H
#define LIB_UTILS_TXTUTILS_H

#include "libosns-datatypes.h"

#define SKIPSPACE_FLAG_REPLACEBYZERO		1

#define REPLACE_CNTRL_FLAG_TEXT			1
#define REPLACE_CNTRL_FLAG_BINARY		2
#define REPLACE_CNTRL_FLAG_UNDERSCORE		4

#define TXT_UTILS_CONVERT_SKIPSPACE             1
#define TXT_UTILS_CONVERT_TOLOWER	        2

/* prototypes */

unsigned int TXT_replace_char(char *buffer, unsigned int size, unsigned char flags, unsigned char *char2use);
void TXT_replace_cntrl_char(char *buffer, unsigned int size, unsigned char *char2use);
void TXT_replace_nontext_char(char *buffer, unsigned int size, unsigned char *char2use);

void TXT_replace_slash_char(char *buffer, unsigned int size);
void TXT_replace_newline_char(char *ptr, unsigned int size);

unsigned int TXT_skip_trailing_spaces(char *ptr, unsigned int size, unsigned int flags);
unsigned int TXT_skip_heading_spaces(char *ptr, unsigned int size);

int TXT_isspace(int c);

void TXT_unslash(char *p);
void TXT_convert_to(char *string, int flags);

void TXT_util_unescape(char *source, unsigned int slength, void (* cb)(unsigned char tchar, unsigned int ctr, unsigned char escaped, void *ptr), void *ptr);
unsigned char TXT_util_has_escaped(char *source, unsigned int length);

unsigned char TXT_util_decode_base64(struct dstr_s *data, struct dstr_s *decoded);

#endif

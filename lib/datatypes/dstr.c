/*
  2018 Stef Bon <stefbon@gmail.com>

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

#include "libosns-basic-system-headers.h"

#include "libosns-log.h"
#include "dstr.h"

/* init */

void DSTR_init(struct dstr_s *str)
{
    str->length=0;
    str->str=NULL;
    str->flags=0;
}

unsigned char DSTR_is_empty(struct dstr_s *str)
{
    return ((str==NULL) || (str->length==0) || (str->str==NULL)) ? 1 : 0;
}

unsigned int DSTR_get_length(struct dstr_s *str)
{
    return (DSTR_is_empty(str) ? 0 : (str->length));
}

/* set */

void DSTR_set_bytes_raw(struct dstr_s *stra, char *data, unsigned int length, unsigned char becomeowner)
{
    stra->str=data;
    stra->length=length;
    if (becomeowner) stra->flags |= DSTR_FLAG_ALLOC_DATA;
}

void DSTR_set_bytes(struct dstr_s *stra, char *data, unsigned int length, unsigned char becomeowner)
{
    if (stra==NULL) return;
    if ((length==0) && data) length=strlen(data);
    DSTR_set_bytes_raw(stra, data, length, becomeowner);
}

void DSTR_set_str_raw(struct dstr_s *stra, struct dstr_s *strb, unsigned char becomeowner)
{
    DSTR_set_bytes_raw(stra, strb->str, strb->length, becomeowner);
    if (becomeowner) strb->flags &= ~DSTR_FLAG_ALLOC_DATA;
}

void DSTR_set_str(struct dstr_s *stra, struct dstr_s *strb, unsigned char becomeowner)
{
    if ((stra==NULL) || (strb==NULL)) return;
    DSTR_set_str_raw(stra, strb, becomeowner);
}

struct dstr_s DSTR_set_from_bytes(char *data, unsigned int length, unsigned char becomeowner)
{
    struct dstr_s tmp=DSTR_INIT;
    DSTR_set_bytes(&tmp, data, length, becomeowner);
    return tmp;
}

/* alloc */

unsigned char DSTR_alloc_str_raw(struct dstr_s *stra, unsigned int size, unsigned char zero)
{

    stra->str=realloc(stra->str, size);

    if (stra->str==NULL) return 0;

    stra->flags |= DSTR_FLAG_ALLOC_DATA;
    stra->length=size;
    if (zero) memset(stra->str, 0, size);

    return 1;
}

/* create */

struct dstr_s *DSTR_create()
{
    struct dstr_s *stra=NULL;

    stra=malloc(sizeof(struct dstr_s));

    if (stra==NULL) {

        logoutput_warning("%s: unable to allocate", __FUNCTION__);
        return NULL;

    }

    DSTR_init(stra);
    stra->flags |= DSTR_FLAG_ALLOC_SELF;
    return stra;
}

static void dstr_clear_shared(struct dstr_s *stra)
{

    if (stra->str) {

        if (stra->flags & DSTR_FLAG_ALLOC_DATA) {

            memset(stra->str, 0, stra->length);
            free(stra->str);
            stra->str=NULL;
            stra->flags &= ~DSTR_FLAG_ALLOC_DATA;
            stra->length=0;

        }

    }

    DSTR_init(stra);

}

void DSTR_clear(struct dstr_s *stra)
{

    if (stra==NULL) return;
    dstr_clear_shared(stra);

}

void DSTR_free(struct dstr_s **p_stra)
{
    struct dstr_s *stra=((p_stra) ? *p_stra : NULL);

    if (stra==NULL) return;
    dstr_clear_shared(stra);

    if (stra->flags & DSTR_FLAG_ALLOC_SELF) {

        free(stra);
        *p_stra=NULL;

    }

}

/* shift */

void DSTR_shift_raw(struct dstr_s *str, unsigned int count)
{
    str->str+=count;
    str->length-=count;
}

void DSTR_shift(struct dstr_s *str, unsigned int count)
{
    if (count>str->length) count=str->length;
    DSTR_shift_raw(str, count);
}

/* shrink */

void DSTR_shrink_raw(struct dstr_s *str, unsigned int count)
{
    str->length-=count;
}

void DSTR_shrink(struct dstr_s *str, unsigned int count)
{
    if (count>str->length) count=str->length;
    str->length-=count;
}

/* copy */

static unsigned int dstr_copy_hlpr(struct dstr_s *stra, char *data, unsigned int length)
{
    if (stra->length < length) length=stra->length;
    memcpy(stra->str, data, length);
    return length;
}

unsigned int DSTR_copy_bytes(struct dstr_s *stra, char *data, unsigned int length)
{
    if ((stra==NULL) || (data==NULL) || (length==0)) return 0;
    return dstr_copy_hlpr(stra, data, length);
}

unsigned int DSTR_copy_str(struct dstr_s *stra, struct dstr_s *strb)
{

    if ((stra==NULL) || (strb==NULL)) return 0;
    return dstr_copy_hlpr(stra, strb->str, strb->length);
}

void DSTR_copy_to_bytes(struct dstr_s *stra, char *buffer, unsigned int length)
{
    unsigned int bytes2copy=0;

    if ((stra==NULL) || (buffer==NULL) || (length==0)) return;
    if (stra->length > length) bytes2copy=length;
    memcpy(buffer, stra->str, bytes2copy);
}

/* move */

unsigned int DSTR_move_str(struct dstr_s *stra, struct dstr_s *strb)
{

    if ((stra==NULL) || (strb==NULL)) return 0;

    DSTR_clear(stra);
    stra->str=strb->str;
    stra->length=strb->length;
    strb->str=NULL;
    strb->length=0;

    if (strb->flags & DSTR_FLAG_ALLOC_DATA) {

	stra->flags |= DSTR_FLAG_ALLOC_DATA;
	strb->flags &= ~DSTR_FLAG_ALLOC_DATA;

    }

    return stra->length;

}

/* compare */

static unsigned char utils_compare_string(const char *stra, const char *strb, unsigned char fullcmp, unsigned char ignorecase)
{
    unsigned int lena=(stra ? strlen(stra) : 0);
    unsigned int lenb=(strb ? strlen(strb) : 0);

    if ((lena==0) || (lenb==0)) {

	return 0;

    } else if (strcmp(stra, strb)==0) {

	return 1;

    } else if (ignorecase) {
	char tmpa[lena];
	char tmpb[lenb];

	memcpy(tmpa, stra, lena);
	memcpy(tmpb, strb, lenb);

	for (unsigned int i=0; i<lena; i++) tmpa[i]=toupper(tmpa[i]);
	for (unsigned int i=0; i<lenb; i++) tmpb[i]=toupper(tmpb[i]);

	if (lena==lenb) {

	    return (strcmp(tmpa, tmpb)==0) ? 1 : 0;

	} else {

	    if (fullcmp==0) {

		if (lena<lenb) {

		    return (strncmp(tmpa, tmpb, lena)==0) ? 1 : 0;

		} else {

		    return (strncmp(tmpa, tmpb, lenb)==0) ? 1 : 0;

		}

	    }

	}

    }

    return 0;

}

unsigned char DSTR_cmp_bytes(struct dstr_s *stra, char *data, unsigned int length, unsigned char fullcmp, unsigned int start, unsigned char ignorecase)
{
    char *from=NULL;
    unsigned int left=0;

    /* parameters valid ? */

    if ((stra==NULL) || (data==NULL) || (start >= stra->length)) return 0;

    from=stra->str + start;
    left=stra->length - start;

    if (length==0) {

	/* never look futher than left bytes
	    more bytes are not used when comparing */

	length=strnlen(data, left);

    }

    if ((fullcmp) && (length != left)) return 0;

    {
	char tmpa[left];
	char tmpb[length];

	memcpy(tmpa, from, left);
	memcpy(tmpb, data, length);

	if (ignorecase) {

	    for (unsigned int i=0; i<left; i++) tmpa[i]=toupper(tmpa[i]);
	    for (unsigned int i=0; i<length; i++) tmpb[i]=toupper(tmpb[i]);

	}

	if (left<length) {

	    return (memcmp(tmpa, tmpb, left)==0) ? 1 : 0;

	} else {

	    return (memcmp(tmpa, tmpb, length)==0) ? 1 : 0;

	}

    }

    return 0;

}

unsigned char DSTR_cmp_str(struct dstr_s *stra, struct dstr_s *strb, unsigned char fullcmp, unsigned int start, unsigned char ignorecase)
{

    if (strb==NULL) return 0;
    return DSTR_cmp_bytes(stra, strb->str, strb->length, fullcmp, start, ignorecase);
}

unsigned char DSTR_cmp_bytes_reverse(struct dstr_s *stra, char *data, unsigned int length)
{
    char *start=NULL;

    /* parameters valid ? */

    if ((stra==NULL) || (data==NULL)) return 0;

    if (length==0) length=strnlen(data, stra->length);
    if (length>stra->length) return 0;

    start=stra->str + stra->length - length;

    return (memcmp(start, data, length)==0) ? 1 : 0;
}

/* get first str (from left) */

unsigned int DSTR_get_first_dstr(struct dstr_s *stra, int seperator, struct dstr_s *firststr, unsigned char shift, unsigned char notfoundwholestring)
{
    struct dstr_s dummy=DSTR_INIT;

    if (DSTR_is_empty(stra)) return 0;
    if (firststr==NULL) firststr=&dummy;
    DSTR_init(firststr);

    char *sep=memchr(stra->str, seperator, stra->length);
    unsigned int length=0;

    firststr->str=stra->str;

    if (sep) {

        firststr->length=(unsigned int)(sep - stra->str);
        if (shift) DSTR_shift(stra, (firststr->length + 1));

    } else {

        if (notfoundwholestring) firststr->length=stra->length;
        if (shift) DSTR_shift(stra, firststr->length);

    }

    return firststr->length;

}

unsigned int DSTR_get_last_dstr(struct dstr_s *stra, int seperator, struct dstr_s *laststr, unsigned char shift, unsigned char notfoundwholestring)
{
    struct dstr_s dummy=DSTR_INIT;

    if (DSTR_is_empty(stra)) return 0;
    if (laststr==NULL) laststr=&dummy;
    DSTR_init(laststr);

    char *sep=memrchr(stra->str, seperator, stra->length);
    unsigned int length=0;

    if (sep) {

        laststr->str=sep + 1;
        laststr->length=(unsigned int)(stra->str + stra->length - laststr->str);
        if (shift) stra->length -= (laststr->length + 1);

    } else {

        if (notfoundwholestring) {

            laststr->str=stra->str;
            laststr->length=stra->length;

        }

        if (shift) stra->length -= laststr->length;

    }

    return laststr->length;
}

unsigned char DSTR_append(struct dstr_s *stra, int seperator, char *data, unsigned int size, unsigned char doalloc, unsigned char addseperatoratbegin)
{
    unsigned int length=0;
    unsigned char addsep=0;
    unsigned int newlength=0;

    if ((stra==NULL) || (data==NULL)) return 0;
    if (size==0) size=strlen(data);

    length=strnlen(stra->str, stra->length);

    /* check a seperator is required */

    if (addseperatoratbegin) {

	addsep=(seperator ? 1 : 0);

    } else {

	addsep=((length && seperator) ? 1 : 0);

    }

    /* does it fit in the existing buffer ? */

    newlength=(length + addsep + size);

    if (newlength>stra->length) {

	if (doalloc==0) return 0; /* reallocation is required but not allowed */
	if ((stra->str) && ((stra->flags & DSTR_FLAG_ALLOC_DATA)==0)) return 0; /* str is not allocated so it's not possible to reallocate it */
	stra->str=realloc(stra->str, newlength);
	if (stra->str==NULL) return 0;
	stra->flags |= DSTR_FLAG_ALLOC_SELF;
	stra->length=newlength;

	memset(&stra->str[length], 0, newlength);

    }

    if (addsep) {

	stra->str[length]=seperator;
	length++;

    }

    memcpy(&stra->str[length], data, size);
    return 1;

}

/* convert a string (type dstr) to a (decimal) long */

unsigned int DSTR_convert_str_to_long(struct dstr_s *stra)
{
    char tmp[stra->length + 1];

    memcpy(tmp, stra->str, stra->length);
    tmp[stra->length]='\0';
    return strtol(tmp, NULL, 10);
}

/*
  2010, 2011, 2012, 2103, 2014, 2015, 2016 Stef Bon <stefbon@gmail.com>

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
#include "libosns-event.h"

#include "txtutils.h"

unsigned int TXT_replace_char(char *buffer, unsigned int size, unsigned char flags, unsigned char *char2use)
{
    unsigned int count=0;

    for (unsigned int i=0; i<size; i++) {

	if (flags & REPLACE_CNTRL_FLAG_BINARY) {

	    if (iscntrl(buffer[i])) {

                if (char2use) buffer[i]=*char2use;
                count++;

            }

	    if (flags & REPLACE_CNTRL_FLAG_TEXT) {

	        if (! isalnum(buffer[i]) && ! ispunct(buffer[i])) {

                    if (char2use) buffer[i]=*char2use;
                    count++;

                }

	    }

	}

    }

    return count;
}

void TXT_replace_cntrl_char(char *buffer, unsigned int size, unsigned char *char2use)
{
    unsigned int count=TXT_replace_char(buffer, size, REPLACE_CNTRL_FLAG_BINARY, char2use);
}

void TXT_replace_nontext_char(char *buffer, unsigned int size, unsigned char *char2use)
{
    unsigned int count=TXT_replace_char(buffer, size, REPLACE_CNTRL_FLAG_TEXT, char2use);
}

void TXT_replace_slash_char(char *buffer, unsigned int size)
{
    for (unsigned int i=0; i<size; i++) if (buffer[i]=='/') buffer[i]=' ';
}

void TXT_replace_newline_char(char *ptr, unsigned int size)
{
    char *sep=NULL;

    sep=memchr(ptr, 13, size);
    if (sep) *sep='\0';

}

unsigned int TXT_skip_trailing_spaces(char *ptr, unsigned int size, unsigned int flags)
{
    unsigned int len=size;

    while ((len > 0) && isspace(ptr[len-1])) {

	if (flags & SKIPSPACE_FLAG_REPLACEBYZERO) ptr[len-1]='\0';
	len--;

    }

    return (size - len);

}

int TXT_isspace(int c)
{
    return isspace(c);
}

unsigned int TXT_skip_heading_spaces(char *ptr, unsigned int size)
{
    unsigned int pos=0;

    while (isspace(ptr[pos]) && (pos<size)) pos++;

    if (pos>0) {

	memmove(ptr, &ptr[pos], size - pos);
	memset(&ptr[size - pos], '\0', pos);

    }

    return pos;

}

void TXT_unslash(char *p)
{
    char *q = p;
    char *pkeep = p;

    while ((*q++ = *p++) != 0) {

	if (q[-1] == '/') {

	    while (*p == '/') p++;

	}
    }

    if (q > pkeep + 2 && q[-2] == '/') q[-2] = '\0';
}

void TXT_convert_to(char *string, int flags)
{
    char *p=string, *q=string;

    if (flags==0) return;

    for (p=string; *p != '\0'; ++p) {

	if (flags & TXT_UTILS_CONVERT_SKIPSPACE) {

	    if (isspace(*p)) continue;

	}

	if (flags & TXT_UTILS_CONVERT_TOLOWER) {

	    *q=tolower(*p);

	} else {

	    *q=*p;

	}

	q++;

    }

    *q='\0';

}

static unsigned int txt_util_calc_octal(unsigned char **p_schar, char *endpoint)
{
    unsigned char *schar=*p_schar;
    unsigned int length=(unsigned int)(endpoint - (char *)schar);
    unsigned int result=0;
    unsigned int pos=0;
    unsigned int bytes2check=(length < 3) ? length : 3;

    for (unsigned int i=0; i<bytes2check; i++) {
        unsigned char asciivalue=*schar;

        if ((asciivalue < '0') || (asciivalue > '7')) break; /* stop if non octal value is found */
        result = (8 * result) + (asciivalue - '0');
        schar++;

    }

    *p_schar=schar;
    return result;

}

static void cb_default(unsigned char tchar, unsigned int ctr, unsigned char escaped, void *ptr)
{}

void TXT_util_unescape(char *source, unsigned int slength, void (* cb)(unsigned char tchar, unsigned int ctr, unsigned char escaped, void *ptr), void *ptr)
{
    unsigned char *schar=(unsigned char *) source;
    unsigned char ctr=0;

    if (cb==NULL) cb=cb_default;

    while (schar < (unsigned char *)(source + slength)) {
        unsigned char stepsource=1;
        unsigned char escaped=1;
        unsigned char tchar=0;

        if (*schar=='\\') {

            /* start of an escape string */

            if ((schar + 1) >= (unsigned char *)(source + slength)) break;
            schar++;

            switch (*schar) {

                case '\0':

                    tchar='\0';
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':

                    tchar=(unsigned char) txt_util_calc_octal(&schar, (source + slength - 1));
                    stepsource=0; /* step in source already done */
                    break;

                case 'b':

                    tchar='\b';
                    break;

                case 'f':

                    tchar='\f';
                    break;

                case 'n':

                    tchar='\n';
                    break;

                case 'r':

                    tchar='\r';
                    break;

                case 't':

                    tchar='\t';
                    break;

                default:      /* Also handles \" and \\ */

                    tchar=*schar;
                    escaped=0;
                    break;

            }

        } else {

            escaped=0;
            tchar=*schar;

        }

        (* cb)(tchar, ctr, escaped, ptr);
        schar += stepsource;
        ctr++;

    }

}

unsigned char TXT_util_has_escaped(char *source, unsigned int length)
{
    char *sep=memchr(source, '\\', length);
    return (sep ? 1 : 0);
}

/* base64 decode from https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c */

static char base64_encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static char base64_decoding_table[256];
static unsigned char initdone=0;

static void decode_base64_init()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    if (EVENT_signal_lock(esignal)==0) {

	if (initdone==0) {

	    initdone=1;
    	    for (int i = 0; i < 64; i++) {

		base64_decoding_table[(unsigned char) base64_encoding_table[i]] = i;
		logoutput_debug("%s: i %u bet %u", __FUNCTION__, i, base64_encoding_table[i]);

	    }

	}

	EVENT_signal_unlock(esignal);

    }

}

static uint32_t decode_base64_one(unsigned char data)
{
    uint32_t result=(data=='=' ? 0 : base64_decoding_table[data]);

    logoutput_debug("%s: in %u out %u data %u", __FUNCTION__, data, result, base64_decoding_table[data]);
    return result;
}

static uint32_t decode_base64_four(char *data)
{
    uint32_t sextet_a=decode_base64_one((unsigned char) data[0]);
    uint32_t sextet_b=decode_base64_one((unsigned char) data[1]);
    uint32_t sextet_c=decode_base64_one((unsigned char) data[2]);
    uint32_t sextet_d=decode_base64_one((unsigned char) data[3]);

    return (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
}

unsigned char TXT_util_decode_base64(struct dstr_s *data, struct dstr_s *decoded)
{
    unsigned int length=0;

    decode_base64_init();

    if ((data->length % 4) != 0) {

	logoutput_debug("%s: data length should be divisible by 4 but is not", __FUNCTION__);
	return 0;

    }

    length=3 * (data->length / 4);
    if (data->str[data->length-1]=='=') length--;
    if (data->str[data->length-2]=='=') length--;

    if (DSTR_alloc_str_raw(decoded, length, 1)==0) {

	logoutput_debug("%s: unable to allocate %u bytes", __FUNCTION__, length);
	return 0;

    }

    for (unsigned int i=0, j=0; i<data->length;) {

	logoutput_debug("%s: %.*s", __FUNCTION__, 4, &data->str[i]);

	uint32_t triple=decode_base64_four(&data->str[i]);
	i+=4;

	logoutput_debug("%s: i: %u triple %u", __FUNCTION__, i, triple);

	if (j<length) decoded->str[j++]=(triple >> 2 * 8) & 0xFF;
	if (j<length) decoded->str[j++]=(triple >> 1 * 8) & 0xFF;
	if (j<length) decoded->str[j++]=(triple >> 0 * 8) & 0xFF;

    }

    return 1;
}

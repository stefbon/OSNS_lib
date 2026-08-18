/*

  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Stef Bon <stefbon@gmail.com>

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

#include <time.h>

#include "time.h"
#include "libosns-log.h"

#define SYSTEM_TIME_NSEC_MAX	1000000000

void TIME_now(struct timespec_s *time)
{

#ifdef __linux__

    struct timespec tmp;

    int result=clock_gettime(CLOCK_REALTIME, &tmp);

    time->st_sec=tmp.tv_sec;
    time->st_nsec=tmp.tv_nsec;

#else

    time->st_sec=0;
    time->st_nsec=0;

#endif

}

void TIME_substract(struct timespec_s *expire, struct timespec_s *minus)
{

    expire->st_sec-=minus->st_sec;

    if (expire->st_nsec < minus->st_nsec) {

	expire->st_sec--;
	expire->st_nsec+=(SYSTEM_TIME_NSEC_MAX - minus->st_nsec);

    } else {

	expire->st_nsec-=minus->st_nsec;

    }

}

/* get the expired time when comparing time to current time (==now)
    two cases:
    - time is in the past -> expired seconds is positive
    - time is in the future -> expired seconds negative
    note the nsec is by definition always positive ... */

void TIME_get_expired(struct timespec_s *time, struct timespec_s *expired)
{
    struct timespec_s tmp;

    TIME_now(&tmp);
    expired->st_sec=(tmp.st_sec - time->st_sec);

    if (time->st_nsec <= tmp.st_nsec) {

	/* nsec time is in the past or present */

	expired->st_nsec=tmp.st_nsec - time->st_nsec;

    } else {

	expired->st_sec--;
	expired->st_nsec=time->st_nsec - tmp.st_nsec;

    }

}

void TIME_copy(struct timespec_s *to, struct timespec_s *from)
{
    memcpy(to, from, sizeof(struct timespec_s));
}

void TIME_plus(struct timespec_s *expire, struct timespec_s *plus)
{
    expire->st_sec+=plus->st_sec;
    expire->st_nsec+=plus->st_nsec;

    if (expire->st_nsec > SYSTEM_TIME_NSEC_MAX) {

	expire->st_nsec -= SYSTEM_TIME_NSEC_MAX;
	expire->st_sec++;

    }

}

static void correct_nsec_check_bound(struct timespec_s *time, uint32_t base, uint32_t count)
{
    struct timespec_s plus;

    plus.st_sec=(base * count) / SYSTEM_TIME_NSEC_MAX;
    plus.st_nsec=(base * count) % SYSTEM_TIME_NSEC_MAX;

    TIME_plus(time, &plus);

}

void TIME_add(struct timespec_s *time, unsigned char what, unsigned int count)
{

    switch (what) {

	case TIME_ADD_DECI:

	    /* one in ten (= 10^1) */

	    correct_nsec_check_bound(time, 100000000, count);
	    break;

	case TIME_ADD_CENTI:

	    /* one in hundredth (=10^2) */

	    correct_nsec_check_bound(time, 10000000, count);
	    break;

	case TIME_ADD_MILLI:

	    /* one in thousand (=10^3) */

	    correct_nsec_check_bound(time, 1000000, count);
	    break;

	case TIME_ADD_MICRO:

	    /* one in million (=10^6) */

	    correct_nsec_check_bound(time, 1000, count);
	    break;

	case TIME_ADD_NANO:

	    /*one */

	    correct_nsec_check_bound(time, 1, count);
	    break;

	case TIME_ADD_ZERO:

	    time->st_sec += count;
	    break;

	case TIME_ADD_DECA:

	    time->st_sec += (10 * count);
	    break;

	case TIME_ADD_HECTO:

	    time->st_sec += (100 * count);
	    break;

	case TIME_ADD_KILO:

	    time->st_sec += (1000 * count);
	    break;

	case TIME_ADD_MEGA:

	    time->st_sec += (1000000 * count);
	    break;

	case TIME_ADD_GIGA:

	    time->st_sec += (1000000000 * count);
	    break;

	default:

	    logoutput_error("%s: level %i not supported", __FUNCTION__, what);

    }

    return;

}





int TIME_earlier(struct timespec_s *a, struct timespec_s *b)
{
    int result=0;

    // logoutput_debug("system_time_test_earlier: test %li:%u is earlier than %li:%u", a->st_sec, a->st_nsec, b->st_sec, b->st_nsec);

    if (a->st_sec > b->st_sec) {

	result=-1;

    } else if (a->st_sec < b->st_sec) {

	result=1;

    } else {

	if (a->st_nsec > b->st_nsec) {

	    result=-1;

	} else if (a->st_nsec < b->st_nsec) {

	    result=1;

	}

    }

    return result;
}

void TIME_from_double(struct timespec_s *time, double from)
{
    time->st_sec=(timespec_sec_t) from;
    time->st_nsec=(timespec_nsec_t) ((from - time->st_sec) * SYSTEM_TIME_NSEC_MAX);
}

double TIME_to_double(struct timespec_s *time)
{
    double sec=time->st_sec;
    double nsec=time->st_nsec;

    return (sec + (nsec / SYSTEM_TIME_NSEC_MAX));
}

void TIME_set(struct timespec_s *time, timespec_sec_t sec, timespec_nsec_t nsec)
{
    time->st_sec=sec;
    time->st_nsec=nsec;
}

timespec_sec_t TIME_get_sec(struct timespec_s *time)
{
    return time->st_sec;
}

timespec_nsec_t TIME_get_nsec(struct timespec_s *time)
{
    return time->st_nsec;
}

void TIME_nanosleep(struct timespec_s *time)
{

    if (time==NULL) return;

#ifdef __linux

    struct timespec tmp={.tv_sec=time->st_sec, .tv_nsec=time->st_nsec};
    if (nanosleep(&tmp, NULL)==-1) logoutput_debug("system_nanosleep: error %u:%s", errno, strerror(errno));

#endif
}

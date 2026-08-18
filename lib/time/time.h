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

#ifndef LIB_TIME_TIME_H
#define LIB_TIME_TIME_H

typedef int64_t			timespec_sec_t;
typedef uint32_t		timespec_nsec_t;

struct timespec_s {
    timespec_sec_t		st_sec;
    timespec_nsec_t		st_nsec;
};

#define TIME_INIT		{0, 0}

#define TIME_ADD_DECI	1
#define TIME_ADD_CENTI	2
#define TIME_ADD_MILLI	3
#define TIME_ADD_MICRO	4
#define TIME_ADD_NANO	5
#define TIME_ADD_ZERO	6
#define TIME_ADD_DECA	7
#define TIME_ADD_HECTO	8
#define TIME_ADD_KILO	9
#define TIME_ADD_MEGA	10
#define TIME_ADD_GIGA	11

/* Prototypes */

void TIME_now(struct timespec_s *time);

void TIME_plus(struct timespec_s *expire, struct timespec_s *plus);

void TIME_substract(struct timespec_s *expire, struct timespec_s *minus);

void TIME_get_expired(struct timespec_s *time, struct timespec_s *expired);
void TIME_copy(struct timespec_s *to, struct timespec_s *from);

void TIME_add(struct timespec_s *expire, unsigned char how, unsigned int count);

int TIME_earlier(struct timespec_s *a, struct timespec_s *b);

void TIME_from_double(struct timespec_s *time, double from);
double TIME_to_double(struct timespec_s *time);

void TIME_set(struct timespec_s *time, timespec_sec_t sec, timespec_nsec_t nsec);

timespec_sec_t TIME_get_sec(struct timespec_s *time);
timespec_nsec_t TIME_get_nsec(struct timespec_s *time);

void TIME_nanosleep(struct timespec_s *time);

#endif

/*
  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017  Stef Bon <stefbon@gmail.com>

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

#ifndef OSNS_ARGUMENTS_H
#define OSNS_ARGUMENTS_H

#define OSNS_ARGINDEX_HELP                      0
#define OSNS_ARGINDEX_FORK                      1
#define OSNS_ARGINDEX_PROFILE			2

#define OSNS_ARGUMENT_HELP                      (1 << OSNS_ARGINDEX_HELP)
#define OSNS_ARGUMENT_FORK                      (1 << OSNS_ARGINDEX_FORK)
#define OSNS_ARGUMENT_PROFILE                   (1 << OSNS_ARGINDEX_PROFILE)

struct osns_arguments_s {
    unsigned int                flags;
    pid_t                       pid;
    char			*startprofile;
};

#define OSNS_ARGUMENT_INIT                      {0, 0, NULL}

/* Prototypes */

int OSNS_parse_arguments(int argc, char *argv[], struct osns_arguments_s *oarg, unsigned int flags);

#endif

/*
  2017 Stef Bon <stefbon@gmail.com>

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

#ifndef _LIB_ERROR_ERROR_H
#define _LIB_ERROR_ERROR_H

struct error_s;

struct error_subsystem_s {
    const char						*name;
    char						*(*get_description)(struct error_s *error);
};

struct error_s {
    struct error_subsystem_s				*subsystem;
    unsigned int					errnum;
    unsigned int					size;
    char						*function;
};

#define ERROR_INIT				        {NULL, 0, 0, NULL}

/* prototypes */

char *ERROR_get_description(struct error_s *error);
void ERROR_init(struct error_s *error);

void ERROR_set_by_system_errno(struct error_s *error, const char *function);

void ERROR_set_subsystem(struct error_s *error, struct error_subsystem_s *subsys);

unsigned int ERROR_get_errnum(struct error_s *error);
void ERROR_set_errnum(struct error_s *error, unsigned int errnum);

unsigned char ERROR_is_system_wouldblock(struct error_s *error);
unsigned char ERROR_is_system_interrupted(struct error_s *error);
unsigned char ERROR_is_system_invalid(struct error_s *error);
unsigned char ERROR_is_system_io(struct error_s *error);
unsigned char ERROR_is_system_noent(struct error_s *error);
unsigned char ERROR_is_system_timedout(struct error_s *error);
unsigned char ERROR_is_system_exist(struct error_s *error);
unsigned char ERROR_is_system_permission_denied(struct error_s *error);
unsigned char ERROR_is_system_bad_fd(struct error_s *error);
unsigned char ERROR_is_system_not_socket(struct error_s *error);

unsigned char ERROR_is_system_connection_error(struct error_s *error);

#endif

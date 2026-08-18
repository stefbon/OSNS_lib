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

#ifndef LIB_FS_PATH_PATH_APPEND_H
#define LIB_FS_PATH_PATH_APPEND_H

/* prototypes */

unsigned int FS_path_get_append_required_size(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep);
unsigned int FS_path_append_nowrite(struct fs_path_s *path, char *bytes, unsigned int size, unsigned char addpathsep);

void FS_path_append_init(struct fs_path_s *path, unsigned int flags);
unsigned int FS_path_append(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep);

void FS_path_prepend_init(struct fs_path_s *path, unsigned int flags);
unsigned int FS_path_prepend(struct fs_path_s *path, const unsigned char type, void *ptr, unsigned char addpathsep);

#endif

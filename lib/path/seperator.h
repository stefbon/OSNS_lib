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

#ifndef LIB_FS_PATH_PATH_SEPERATOR_H
#define LIB_FS_PATH_PATH_SEPERATOR_H

/* prototypes */

unsigned char FS_path_is_path_seperator(unsigned char token);
unsigned char FS_path_write_path_seperator(char *pos);
const char *FS_path_get_path_seperator();
int FS_path_get_path_seperator_ascii();
char *FS_path_search_path_seperator(struct fs_path_s *path);
char *FS_path_search_path_seperator_reverse(struct fs_path_s *path);

#endif

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

#ifndef LIB_FS_STAT_UTILS_H
#define LIB_FS_STAT_UTILS_H

#include "libosns-time.h"

#include "stat.h"

/* Prototypes */

/* get */

uint64_t FS_stat_get_ino(struct fs_stat_s *stat);
uint32_t FS_stat_get_nlink(struct fs_stat_s *stat);
uint32_t FS_stat_get_unique_uid(struct fs_stat_s *stat);
uint32_t FS_stat_get_unique_gid(struct fs_stat_s *stat);
off_t FS_stat_get_size(struct fs_stat_s *stat);
uint16_t FS_stat_get_type(struct fs_stat_s *stat);
uint16_t FS_stat_get_mode(struct fs_stat_s *stat);

unsigned char FS_stat_get_time(struct fs_stat_s *stat, unsigned char type, struct timespec_s *time);
unsigned char FS_stat_get_dev(struct fs_stat_s *stat, struct fs_stat_dev_s *dev, unsigned char represented);

/* set */

void FS_stat_set_ino(struct fs_stat_s *stat, uint64_t ino);
void FS_stat_set_type(struct fs_stat_s *stat, uint16_t type);
void FS_stat_set_mode(struct fs_stat_s *stat, uint16_t mode);
void FS_stat_set_uid(struct fs_stat_s *stat, uint32_t uid);
void FS_stat_set_gid(struct fs_stat_s *stat, uint32_t gid);
void FS_stat_set_size(struct fs_stat_s *stat, off_t size);
void FS_stat_set_nlink(struct fs_stat_s *stat, uint32_t nlink);

unsigned char FS_stat_set_time(struct fs_stat_s *stat, unsigned char type, struct timespec_s *time);
unsigned char FS_stat_set_dev(struct fs_stat_s *stat, struct fs_stat_dev_s *dev, unsigned char represented);

void FS_stat_set_blksize(struct fs_stat_s *stat, uint32_t blksize);
void FS_stat_set_blocks(struct fs_stat_s *stat, uint32_t blocks);

void FS_stat_increase_nlink(struct fs_stat_s *stat, int32_t count);
void FS_stat_decrease_nlink(struct fs_stat_s *stat, int32_t count);

uint32_t FS_stat_calc_amount_blocks(uint64_t size, uint32_t blksize);
void FS_stat_calc_blocks(struct fs_stat_s *stat);
uint32_t FS_stat_dev_get_unique(struct fs_stat_dev_s *dev);

int FS_stat_test_ISDIR(struct fs_stat_s *stat);
int FS_stat_test_ISLNK(struct fs_stat_s *stat);
int FS_stat_test_ISBLK(struct fs_stat_s *stat);
int FS_stat_test_ISCHR(struct fs_stat_s *stat);
int FS_stat_test_ISSOCK(struct fs_stat_s *stat);
int FS_stat_test_ISREG(struct fs_stat_s *stat);

uint64_t FS_stat_copy(struct fs_stat_s *fsta, struct fs_stat_s *fstb, uint64_t mask);

#endif

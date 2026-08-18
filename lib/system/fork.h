/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef LIB_SYSTEM_FORK_H
#define LIB_SYSTEM_FORK_H

#define SYSTEM_FORK_FLAG_CLOSE_ALL                              1
#define SYSTEM_FORK_FLAG_REDIRECT_STD                           2
#define SYSTEM_FORK_FLAG_SWITCH_LOG                             4
#define SYSTEM_FORK_FLAG_CHDIR_ROOT                             8
#define SYSTEM_FORK_FLAG_SET_SESSION_ID                         16

/* Prototypes */

int SYSTEM_fork(unsigned int flags, const char *logname, int loglevel);
int SYSTEM_execve(struct fs_path_s *path, char *argv[]);

#endif

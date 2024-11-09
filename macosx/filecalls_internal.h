/* filecalls_internal.h
   Copyright (c) 2005 Peter Howkins, covered under the GNU GPL see file COPYING for more
   details */

#ifndef __FILECALLS_INTERNAL_H
#define __FILECALLS_INTERNAL_H

#include <sys/types.h>
#include <dirent.h>
#include <sys/syslimits.h>

struct Directory_s {
  DIR *hDir;
  char *sPath;
  size_t sPathLen;
};

#define ARCEM_PATH_MAX PATH_MAX

#endif /* __FILECALLS_H */

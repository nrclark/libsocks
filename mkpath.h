#ifndef _MKPATH_H_
#define _MKPATH_H_
#include <sys/types.h>

int mkpath(const char *path, mode_t mode);
extern const char *progname;

#endif

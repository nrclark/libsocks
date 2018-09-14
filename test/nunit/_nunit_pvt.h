/*******************************************************************************
 * @file _nunit_pvt.h
 * @brief Ugly macros that implement portions of nunit. You shouldn't need to
 * used any of these directly.
 *
 * @copyright This file is in the public domain. Modify it, distribute it,
 * or do anything you want with it.
 ******************************************************************************/

#ifndef _NUNIT_PVT_H_
#define _NUNIT_PVT_H_

#include <stdio.h>

#define xstr(s) str(s)
#define str(s) #s

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#endif

#ifndef NULL
#define NULL 0
#endif

#define __label_test() do { fprintf(stderr,"  - %s\n",__func__); } while (0)
#define __test(x, msg, condition) do { \
    int _result = (x); \
    if (!(_result condition)) { \
        fprintf(stderr,"Test '%s' FAILED [%s:%d]: %s (%d %s)\n",__func__,\
                __FILE__,__LINE__, msg, _result, xstr(condition)); \
        return -1; \
    } \
} while(0)

#endif

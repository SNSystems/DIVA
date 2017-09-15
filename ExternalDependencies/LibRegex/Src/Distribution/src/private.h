/*
 * private.h - private definitions for libregex.
 * Copyright (C) 1995 - 2007 Michael Riepe
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* @(#) $Id: private.h,v 1.38 2007/09/07 12:07:59 michael Exp $ */

#ifndef _PRIVATE_H
#define _PRIVATE_H

/* SN-Carlos: For consistency, use the HAVE_LIBREGEX_H symbol */
/* #define __LIBREGEX_INTERNAL__ 1 */

#if HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

/* SN-Carlos: Windows specific */
#ifdef HAVE_STDAFX_H
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#ifdef HAVE_STDBOOL_H
#include "stdbool.h"
#else
typedef int bool;
#define false 0
#define true 1
#endif /* HAVE_STDBOOL_H */

/*
 * Workaround for GLIBC bug:
 * include <stdint.h> before <sys/types.h>
 */
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>

/* SN-Carlos: Include extra needed definitions */
#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* STDC_HEADERS */
extern void *malloc(), *realloc();
extern void free(), bcopy(), abort();
extern int strcmp(), strncmp(), memcmp();
extern void *memcpy(), *memmove(), *memset();
#endif /* STDC_HEADERS */

#if defined(_WIN32)
#include <io.h>
#else
#if HAVE_UNISTD_H
# include <unistd.h>
#else /* HAVE_UNISTD_H */
extern int read(), write(), close();
extern off_t lseek();
#if HAVE_FTRUNCATE
extern int ftruncate();
#endif /* HAVE_FTRUNCATE */
#endif /* HAVE_UNISTD_H */
#endif /* defined(_WIN32) */

#endif /* _PRIVATE_H */

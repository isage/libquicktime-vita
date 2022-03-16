/*******************************************************************************
 lqt_bufalloc.c

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2011 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/ 

/*

 Aligned buffer allocation routine.

 This routine should be used everywhere that a SIMD (MMX/MMX2/SSE/Altivec)
 routine may operate on allocated buffers. SIMD routines require that the
 buffers be aligned on a 16 byte boundary (SSE/SSE2 require 64 byte 
 alignment).

*/

#include "lqt_private.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LOG_DOMAIN "bufalloc"

/*
 * Allocate memory aligned to suit SIMD 
*/

#define powerof2(x)     ((((x)-1)&(x))==0)

static int
lqt_memalign(void **ptr, size_t alignment, size_t size)
	{
	void *mem;

	if	(alignment % sizeof (void *) != 0 || !powerof2(alignment) != 0)
		return(EINVAL);
	mem = malloc((size + alignment - 1) & ~(alignment - 1));
	if	(mem != NULL)
		{
		*ptr = mem;
		return(0);
		}
	return(ENOMEM);
	}

#if	!defined(HAVE_MEMALIGN)
static void *
memalign(size_t alignment, size_t size)
	{

	if 	(alignment % sizeof (void *) || !powerof2(alignment))
		{
		errno = EINVAL;
		return(NULL);
		}
	return(malloc((size + alignment - 1) & ~(alignment - 1)));
	}
#else
/* some systems have memalign() but no declaration for it */
void * memalign (size_t align, size_t size);
#endif

void *lqt_bufalloc(size_t size)
	{
	static size_t simd_alignment = 16;
	static int bufalloc_init = 0;
	int  pgsize;
	void *buf = NULL;

	if	(!bufalloc_init)
		{
#ifdef ARCH_X86 
		simd_alignment = 64;	/* X86 requires 64 for SSE */
		bufalloc_init = 1;
#endif		
		}
		
	pgsize = sysconf(_SC_PAGESIZE);
/*
 * If posix_memalign fails it could be a broken glibc that caused the error,
 * so try again with a page aligned memalign request
*/
	if	(lqt_memalign( &buf, simd_alignment, size))
		buf = memalign(pgsize, size);
	if	(buf && ((size_t)buf & (simd_alignment - 1)))
		{
		free(buf);
		buf = memalign(pgsize, size);
		}
	if	(buf == NULL)
                lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "malloc of %d bytes failed", (int)size);
	else
		memset(buf, '\0', size);

	if	((size_t)buf & (simd_alignment - 1))
                lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
                        "could not allocate %d bytes aligned on a %d byte boundary", (int)size, (int)simd_alignment);
	return buf;
	}

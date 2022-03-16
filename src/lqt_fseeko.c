/*******************************************************************************
 lqt_fseeko.c

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

 64-bit versions of fseeko/ftello.
 
 Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 Portions Copyright (c) 1994, Regents of the University of California

*/

#ifdef	HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	HAVE_FSEEKO

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/*
 *	On BSD/OS and NetBSD, off_t and fpos_t are the same.  Standards
 *	say off_t is an arithmetic type, but not necessarily integral,
 *	while fpos_t might be neither.
 */

int
lqt_fseeko(FILE *stream, off_t offset, int whence)
{
	off_t floc;
	struct stat filestat;

	switch (whence)
	{
		case SEEK_CUR:
			flockfile(stream);
			if (fgetpos(stream, &floc) != 0)
				goto failure;
			floc += offset;
			if (fsetpos(stream, &floc) != 0)
				goto failure;
			funlockfile(stream);
			return 0;
			break;
		case SEEK_SET:
			if (fsetpos(stream, &offset) != 0)
				return -1;
			return 0;
			break;
		case SEEK_END:
			flockfile(stream);
			if (fstat(fileno(stream), &filestat) != 0)
				goto failure;
			floc = filestat.st_size;
			if (fsetpos(stream, &floc) != 0)
				goto failure;
			funlockfile(stream);
			return 0;
			break;
		default:
			errno =	EINVAL;
			return -1;
	}

failure:
	funlockfile(stream);
	return -1;
}

off_t
lqt_ftello(FILE *stream)
{
	off_t floc;

	if (fgetpos(stream, &floc) != 0)
		return -1;
	return floc;
}

#endif

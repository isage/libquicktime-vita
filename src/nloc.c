/*******************************************************************************
 nloc.c

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

#include "lqt_private.h"

int quicktime_nloc_init(quicktime_nloc_t *nloc)
{
	nloc->version = 2;
	nloc->revision = 0;
	nloc->locationFlags = 0;
	nloc->locationData = 0;
	nloc->nodeType[0] = 'o';
	nloc->nodeType[1] = 'b';
	nloc->nodeType[2] = 'j';
	nloc->nodeType[3] = 'e';
	return 0;
}

int quicktime_nloc_delete(quicktime_nloc_t *nloc)
{
    return 0;
}

void quicktime_nloc_dump(quicktime_nloc_t *nloc)
{
	lqt_dump("          node location (nloc)\n");
	lqt_dump("           version %i\n",  nloc->version);
	lqt_dump("           revision %i\n",  nloc->revision);
	lqt_dump("           node type %c%c%c%c\n",  nloc->nodeType[0], nloc->nodeType[1], nloc->nodeType[2], nloc->nodeType[3]);
	lqt_dump("           location flags %ld\n",  nloc->locationFlags);
	lqt_dump("           location data %ld\n", nloc->locationData);
}

int quicktime_read_nloc(quicktime_t *file, quicktime_nloc_t *nloc, quicktime_qtatom_t *nloc_atom)
{
	nloc->version =  quicktime_read_int16(file);
	nloc->revision = quicktime_read_int16(file);
	quicktime_read_char32(file, nloc->nodeType);
	nloc->locationFlags = quicktime_read_int32(file);
	nloc->locationData = quicktime_read_int32(file);
	nloc->reserved1 = quicktime_read_int32(file);
	nloc->reserved2 = quicktime_read_int32(file);
	return 0;
}

void quicktime_write_nloc(quicktime_t *file, quicktime_nloc_t *nloc)
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "nloc", 1);
	quicktime_write_int16(file, nloc->version);
	quicktime_write_int16(file, nloc->revision);
	quicktime_write_char32(file, nloc->nodeType);
	quicktime_write_int32(file, nloc->locationFlags);
	quicktime_write_int32(file, nloc->locationData);
	quicktime_write_int32(file, nloc->reserved1);
	quicktime_write_int32(file, nloc->reserved2);
	quicktime_qtatom_write_footer(file, &atom);
}


/*******************************************************************************
 vrsc.c

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

int quicktime_vrsc_init(quicktime_vrsc_t *vrsc)
{
	vrsc->version = 2;
	vrsc->revision = 0;
	vrsc->DefaultNodeID = 1;
	return 0;
}

int quicktime_vrsc_delete(quicktime_vrsc_t *vrsc)
{
    return 0;
}

void quicktime_vrsc_dump(quicktime_vrsc_t *vrsc)
{
	lqt_dump("        world header (vrsc)\n");
	lqt_dump("         version %i\n",  vrsc->version);
	lqt_dump("         revision %i\n",  vrsc->revision);
	lqt_dump("         name atom id %ld\n",  vrsc->NameAtomID);
	lqt_dump("         default node %ld\n", vrsc->DefaultNodeID);
	lqt_dump("         world flags %ld\n", vrsc->flags);
}

int quicktime_read_vrsc(quicktime_t *file, quicktime_vrsc_t *vrsc, quicktime_qtatom_t *vrsc_atom)
{
	vrsc->version =  quicktime_read_int16(file);
	vrsc->revision = quicktime_read_int16(file);
	vrsc->NameAtomID = quicktime_read_int32(file);
	vrsc->DefaultNodeID = quicktime_read_int32(file);
	vrsc->flags = quicktime_read_int32(file);
	vrsc->reserved1 = quicktime_read_int32(file);
	vrsc->reserved2 = quicktime_read_int32(file);
	return 0;
}

void quicktime_write_vrsc(quicktime_t *file, quicktime_vrsc_t *vrsc)
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "vrsc", 1);
	quicktime_write_int16(file, vrsc->version);
	quicktime_write_int16(file, vrsc->revision);
	quicktime_write_int32(file, vrsc->NameAtomID);
	quicktime_write_int32(file, vrsc->DefaultNodeID);
	quicktime_write_int32(file, vrsc->flags);
	quicktime_write_int32(file, vrsc->reserved1);
	quicktime_write_int32(file, vrsc->reserved2);
	quicktime_qtatom_write_footer(file, &atom);
}


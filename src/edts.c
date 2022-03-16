/*******************************************************************************
 edts.c

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

void quicktime_edts_init(quicktime_edts_t *edts)
{
	quicktime_elst_init(&edts->elst);
}

void quicktime_edts_delete(quicktime_edts_t *edts)
{
	quicktime_elst_delete(&edts->elst);
}

void quicktime_edts_init_table(quicktime_edts_t *edts)
{
	quicktime_elst_init_all(&edts->elst);
}

void quicktime_read_edts(quicktime_t *file, quicktime_edts_t *edts, quicktime_atom_t *edts_atom)
{
	quicktime_atom_t leaf_atom;

	do
	{
		quicktime_atom_read_header(file, &leaf_atom);
		if(quicktime_atom_is(&leaf_atom, "elst"))
		{ quicktime_read_elst(file, &edts->elst); }
		else
			quicktime_atom_skip(file, &leaf_atom);
	}while(quicktime_position(file) < edts_atom->end);
}

void quicktime_edts_dump(quicktime_edts_t *edts)
{
	lqt_dump("  edit atom (edts)\n");
	quicktime_elst_dump(&edts->elst);
}

void quicktime_write_edts(quicktime_t *file, quicktime_edts_t *edts)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "edts");
	quicktime_write_elst(file, &edts->elst);
	quicktime_atom_write_footer(file, &atom);
}

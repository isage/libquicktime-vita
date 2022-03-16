/*******************************************************************************
 dinf.c

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

void quicktime_dinf_init(quicktime_dinf_t *dinf)
{
	quicktime_dref_init(&dinf->dref);
}

void quicktime_dinf_delete(quicktime_dinf_t *dinf)
{
	quicktime_dref_delete(&dinf->dref);
}

void quicktime_dinf_init_all(quicktime_dinf_t *dinf, lqt_file_type_t type)
{
	quicktime_dref_init_all(&dinf->dref, type);
}

void quicktime_dinf_dump(quicktime_dinf_t *dinf)
{
	lqt_dump("    data information (dinf)\n");
	quicktime_dref_dump(&dinf->dref);
}

void quicktime_read_dinf(quicktime_t *file, quicktime_dinf_t *dinf, quicktime_atom_t *dinf_atom)
{
	quicktime_atom_t leaf_atom;

	do
	{
		quicktime_atom_read_header(file, &leaf_atom);
		if(quicktime_atom_is(&leaf_atom, "dref"))
			{ quicktime_read_dref(file, &dinf->dref); }
		else
			quicktime_atom_skip(file, &leaf_atom);
	}while(quicktime_position(file) < dinf_atom->end);
}

void quicktime_write_dinf(quicktime_t *file, quicktime_dinf_t *dinf)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "dinf");
	quicktime_write_dref(file, &dinf->dref);
	quicktime_atom_write_footer(file, &atom);
}

/*******************************************************************************
 fiel.c

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
#include <string.h>

void quicktime_fiel_init(quicktime_fiel_t *fiel)
{
	memset(fiel, 0, sizeof (*fiel));
}

void quicktime_fiel_delete(quicktime_fiel_t *fiel) { }

void quicktime_fiel_dump(quicktime_fiel_t *fiel)
{

	lqt_dump("     fields (fiel)\n");
	lqt_dump("       fields:    %d\n", fiel->fields);
	lqt_dump("       dominance: %d\n", fiel->dominance);
}

void quicktime_read_fiel(quicktime_t *file, quicktime_fiel_t *fiel)
{
	fiel->fields = quicktime_read_char(file);
	fiel->dominance = quicktime_read_char(file);
}

void quicktime_write_fiel(quicktime_t *file, quicktime_fiel_t *fiel)
{
	quicktime_atom_t atom;

	quicktime_atom_write_header(file, &atom, "fiel");
	quicktime_write_char(file, fiel->fields);
	quicktime_write_char(file, fiel->dominance);
	quicktime_atom_write_footer(file, &atom);
}

int lqt_get_fiel(quicktime_t *file, int track, int *nfields, int *dominance)
	{
	quicktime_stsd_table_t *stsdt_p;

	if	((track < 0) || (track >= file->total_vtracks))
		return 0;
	stsdt_p = file->vtracks[track].track->mdia.minf.stbl.stsd.table;

        if      (!stsdt_p->has_fiel)
                return 0;

	if	(nfields != NULL)
		*nfields = stsdt_p->fiel.fields;
	
	if	(dominance != NULL)
		*dominance = stsdt_p->fiel.dominance;
	return 1;
	}

int lqt_set_fiel(quicktime_t *file, int track, int nfields, int dominance)
	{
	quicktime_stsd_table_t *stsdt_p;

	if	((track < 0) || (track >= file->total_vtracks))
		return 0;

	if	(nfields !=1 && nfields != 2)
		return 0;

/*
 * http://developer.apple.com/quicktime/icefloe/dispatch019.html#fiel
 *
 * "dominance" is what Apple calls "detail".  From what I can figure out
 * the term "bottom field first" corresponds to a "detail" setting of 14 and
 * "top field first" is a "detail" setting of 9.
*/
	switch	(dominance)
		{
		case	0:
		case	1:
		case	6:
		case	9:
		case	14:
			break;
		default:
			return 0;
		}

	stsdt_p = file->vtracks[track].track->mdia.minf.stbl.stsd.table;
	stsdt_p->fiel.fields = nfields;
	stsdt_p->fiel.dominance = dominance;
        stsdt_p->has_fiel = 1;
	return 1;
	}

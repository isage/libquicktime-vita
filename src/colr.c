/*******************************************************************************
 colr.c

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

 Init, read, write handler for the "colr" (Clean Aperture) atom.

*/

#include "lqt_private.h"
#include <string.h>

void quicktime_colr_init(quicktime_colr_t *colr)
{
	memset(colr, 0, sizeof (*colr));
}

void quicktime_colr_delete(quicktime_colr_t *colr) { }

void quicktime_colr_dump(quicktime_colr_t *colr)
{

	lqt_dump("     color description (colr)\n");
	lqt_dump("       colorParamType %d\n", colr->colorParamType);
	lqt_dump("       primaries %d\n", colr->primaries);
	lqt_dump("       transferFunction %d\n", colr->transferFunction);
	lqt_dump("       matrix %d\n", colr->matrix);
}

void quicktime_read_colr(quicktime_t *file, quicktime_colr_t *colr)
{
	colr->colorParamType = quicktime_read_int32(file);
	colr->primaries = quicktime_read_int16(file);
	colr->transferFunction = quicktime_read_int16(file);
	colr->matrix = quicktime_read_int16(file);
}

void quicktime_write_colr(quicktime_t *file, quicktime_colr_t *colr)
{
	quicktime_atom_t atom;

	quicktime_atom_write_header(file, &atom, "colr");
	quicktime_write_int32(file, colr->colorParamType);
	quicktime_write_int16(file, colr->primaries);
	quicktime_write_int16(file, colr->transferFunction);
	quicktime_write_int16(file, colr->matrix);
	quicktime_atom_write_footer(file, &atom);
}

int lqt_set_colr(quicktime_t *file, int track, quicktime_colr_t *colr)
{
	quicktime_colr_t *trk_colr;

	if	((track < 0) || (track >= file->total_vtracks))
		return 0;

	trk_colr = &file->vtracks[track].track->mdia.minf.stbl.stsd.table->colr;
	*trk_colr = *colr;
        file->vtracks[track].track->mdia.minf.stbl.stsd.table->has_colr = 1;
	return 1;
}

int lqt_get_colr(quicktime_t *file, int track, quicktime_colr_t *colr)
{
	quicktime_colr_t *trk_colr;

	if	((track < 0) || (track >= file->total_vtracks))
		return 0;
        if	(!file->vtracks[track].track->mdia.minf.stbl.stsd.table->has_colr)
		return 0;
	trk_colr = &file->vtracks[track].track->mdia.minf.stbl.stsd.table->colr;
	*colr = *trk_colr;
	return 1;
}

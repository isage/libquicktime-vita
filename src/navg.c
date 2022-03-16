/*******************************************************************************
 navg.c

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

int quicktime_navg_init(quicktime_navg_t *navg)
{
	navg->version = 1;
	navg->columns = 1;
	navg->rows = 1;
	navg->reserved = 0;
	navg->loop_frames = 1;
	navg->loop_dur = 75;
	navg->movietype = 1;
	navg->loop_timescale = 0;
	navg->fieldofview = 180;
	navg->startHPan = 0;
	navg->endHPan = 360;
	navg->endVPan = -90;
	navg->startVPan = 90;
	navg->initialHPan = 180;
	navg->initialVPan = 30;
	navg->reserved2 = 0;
	return 0;
}

int quicktime_navg_delete(quicktime_navg_t *navg)
{
    return 0;
}

void quicktime_navg_dump(quicktime_navg_t *navg)
{
	lqt_dump("  object parameters (navg)\n");
	lqt_dump("    version %i\n", navg->version);
	lqt_dump("    columns %i\n", navg->columns);
	lqt_dump("    rows %i\n", navg->rows);
	lqt_dump("    loop frames %i\n", navg->loop_frames);
	lqt_dump("    loop frame duration %i\n", navg->loop_dur);
	lqt_dump("    movie type %i\n", navg->movietype);
	lqt_dump("    loop timescale %i\n", navg->loop_timescale);
	lqt_dump("    field of view %f\n", navg->fieldofview);
	lqt_dump("    horizontal start pan %f\n", navg->startHPan);
	lqt_dump("    horizontal end pan %f\n", navg->endHPan);
	lqt_dump("    vertical end pan %f\n", navg->endVPan);
	lqt_dump("    vertical start pan %f\n", navg->startVPan);
	lqt_dump("    initial horizontal pan %f\n", navg->initialHPan);
	lqt_dump("    initial vertical pan %f\n", navg->initialVPan);
}

int quicktime_read_navg(quicktime_t *file, quicktime_navg_t *navg, quicktime_atom_t *navg_atom)
{
	navg->version =  quicktime_read_int16(file);
	navg->columns = quicktime_read_int16(file);
	navg->rows = quicktime_read_int16(file);
	navg->reserved = quicktime_read_int16(file);
	navg->loop_frames = quicktime_read_int16(file);
	navg->loop_dur = quicktime_read_int16(file);
	navg->movietype = quicktime_read_int16(file);
	navg->loop_timescale = quicktime_read_int16(file);
	navg->fieldofview = quicktime_read_fixed32(file);
	navg->startHPan = quicktime_read_fixed32(file);
	navg->endHPan = quicktime_read_fixed32(file);
	navg->endVPan = quicktime_read_fixed32(file);
	navg->startVPan = quicktime_read_fixed32(file);
	navg->initialHPan = quicktime_read_fixed32(file);
	navg->initialVPan = quicktime_read_fixed32(file);
	navg->reserved2 = quicktime_read_int32(file);
	return 0;
}

void quicktime_write_navg(quicktime_t *file, quicktime_navg_t *navg)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "NAVG");
	quicktime_write_int16(file, navg->version);
	quicktime_write_int16(file, navg->columns);
	quicktime_write_int16(file, navg->rows);
	quicktime_write_int16(file, navg->reserved);
	quicktime_write_int16(file, navg->loop_frames);
	quicktime_write_int16(file, navg->loop_dur);
	quicktime_write_int16(file, navg->movietype);
	quicktime_write_int16(file, navg->loop_timescale);
	quicktime_write_fixed32(file, navg->fieldofview);
	quicktime_write_fixed32(file, navg->startHPan);
	quicktime_write_fixed32(file, navg->endHPan);
	quicktime_write_fixed32(file, navg->endVPan);
	quicktime_write_fixed32(file, navg->startVPan);
	quicktime_write_fixed32(file, navg->initialHPan);
	quicktime_write_fixed32(file, navg->initialVPan);
	quicktime_write_fixed32(file, navg->reserved2);	

	quicktime_atom_write_footer(file, &atom);
}


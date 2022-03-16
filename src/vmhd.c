/*******************************************************************************
 vmhd.c

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

void quicktime_vmhd_init(quicktime_vmhd_t *vmhd)
{
	vmhd->version = 0;
	vmhd->flags = 1;
	vmhd->graphics_mode = 64;
	vmhd->opcolor[0] = 32768;
	vmhd->opcolor[1] = 32768;
	vmhd->opcolor[2] = 32768;
}

void quicktime_vmhd_init_video(quicktime_t *file, 
								quicktime_vmhd_t *vmhd, 
								int frame_w,
								int frame_h, 
                                                                int frame_duration,
                                                                int timescale)
{
}

void quicktime_vmhd_delete(quicktime_vmhd_t *vmhd)
{
}

void quicktime_vmhd_dump(quicktime_vmhd_t *vmhd)
{
	lqt_dump("    video media header (vmhd)\n");
	lqt_dump("     version %d\n", vmhd->version);
	lqt_dump("     flags %ld\n", vmhd->flags);
	lqt_dump("     graphics_mode %d\n", vmhd->graphics_mode);
	lqt_dump("     opcolor %d %d %d\n", vmhd->opcolor[0], vmhd->opcolor[1], vmhd->opcolor[2]);
}

void quicktime_read_vmhd(quicktime_t *file, quicktime_vmhd_t *vmhd)
{
	int i;
	vmhd->version = quicktime_read_char(file);
	vmhd->flags = quicktime_read_int24(file);
	vmhd->graphics_mode = quicktime_read_int16(file);
	for(i = 0; i < 3; i++)
		vmhd->opcolor[i] = quicktime_read_int16(file);
}

void quicktime_write_vmhd(quicktime_t *file, quicktime_vmhd_t *vmhd)
{
	quicktime_atom_t atom;
	int i;
	quicktime_atom_write_header(file, &atom, "vmhd");

	quicktime_write_char(file, vmhd->version);
	quicktime_write_int24(file, vmhd->flags);
	quicktime_write_int16(file, vmhd->graphics_mode);
	
	for(i = 0; i < 3; i++)
		quicktime_write_int16(file, vmhd->opcolor[i]);

	quicktime_atom_write_footer(file, &atom);
}


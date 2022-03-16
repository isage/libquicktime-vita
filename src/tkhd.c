/*******************************************************************************
 tkhd.c

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

int quicktime_tkhd_init(quicktime_tkhd_t *tkhd, lqt_file_type_t type)
{
	int i;
	tkhd->version = 0;
        if(IS_MP4(type))
          tkhd->flags = 7;
        else
          tkhd->flags = 15;
        tkhd->creation_time = quicktime_current_time();
	tkhd->modification_time = quicktime_current_time();
	tkhd->track_id = 0;
	tkhd->reserved1 = 0;
	tkhd->duration = 0;      /* need to set this when closing */
	for(i = 0; i < 8; i++) tkhd->reserved2[i] = 0;
	tkhd->layer = 0;
	tkhd->alternate_group = 0;
	tkhd->volume = 1.0;
	tkhd->reserved3 = 0;
	quicktime_matrix_init(&tkhd->matrix);
	tkhd->track_width = 0;
	tkhd->track_height = 0;
	return 0;
}

int quicktime_tkhd_delete(quicktime_tkhd_t *tkhd)
{
	return 0;
}

void quicktime_tkhd_dump(quicktime_tkhd_t *tkhd)
{
	lqt_dump("  track header (tkhd)\n");
	lqt_dump("   version %d\n", tkhd->version);
	lqt_dump("   flags %ld\n", tkhd->flags);
	lqt_dump("    creation_time ");
        lqt_dump_time(tkhd->creation_time);
        lqt_dump("\n");
	lqt_dump("    modification_time ");
        lqt_dump_time(tkhd->modification_time);
        lqt_dump("\n");
	lqt_dump("   track_id %d\n", tkhd->track_id);
	lqt_dump("   reserved1 %ld\n", tkhd->reserved1);
	lqt_dump("   duration %"PRId64"\n", tkhd->duration);
	quicktime_print_chars("   reserved2 ", tkhd->reserved2, 8);
	lqt_dump("   layer %d\n", tkhd->layer);
	lqt_dump("   alternate_group %d\n", tkhd->alternate_group);
	lqt_dump("   volume %f\n", tkhd->volume);
	lqt_dump("   reserved3 %ld\n", tkhd->reserved3);
	quicktime_matrix_dump(&tkhd->matrix);
	lqt_dump("   track_width %f\n", tkhd->track_width);
	lqt_dump("   track_height %f\n", tkhd->track_height);
}

void quicktime_read_tkhd(quicktime_t *file, quicktime_tkhd_t *tkhd)
{
	tkhd->version = quicktime_read_char(file);
	tkhd->flags = quicktime_read_int24(file);
        if(tkhd->version == 0)
          {
          tkhd->creation_time = quicktime_read_int32(file);
          tkhd->modification_time = quicktime_read_int32(file);
          }
        else if(tkhd->version == 1)
          {
          tkhd->creation_time = quicktime_read_int64(file);
          tkhd->modification_time = quicktime_read_int64(file);
          }
	tkhd->track_id = quicktime_read_int32(file);
	tkhd->reserved1 = quicktime_read_int32(file);
        if(tkhd->version == 0)
          tkhd->duration = quicktime_read_int32(file);
        else if(tkhd->version == 1)
          tkhd->duration = quicktime_read_int64(file);
        
	quicktime_read_data(file, tkhd->reserved2, 8);
	tkhd->layer = quicktime_read_int16(file);
	tkhd->alternate_group = quicktime_read_int16(file);
	tkhd->volume = quicktime_read_fixed16(file);
	tkhd->reserved3 = quicktime_read_int16(file);
	quicktime_read_matrix(file, &tkhd->matrix);
	tkhd->track_width = quicktime_read_fixed32(file);
	tkhd->track_height = quicktime_read_fixed32(file);
}

void quicktime_write_tkhd(quicktime_t *file, quicktime_tkhd_t *tkhd)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "tkhd");
	quicktime_write_char(file, tkhd->version);
	quicktime_write_int24(file, tkhd->flags);
        if(tkhd->version == 0)
          {
          quicktime_write_int32(file, tkhd->creation_time);
          quicktime_write_int32(file, tkhd->modification_time);
          }
        else if(tkhd->version == 1)
          {
          quicktime_write_int64(file, tkhd->creation_time);
          quicktime_write_int64(file, tkhd->modification_time);
          }
	quicktime_write_int32(file, tkhd->track_id);
	quicktime_write_int32(file, tkhd->reserved1);
        if(tkhd->version == 0)
          quicktime_write_int32(file, tkhd->duration);
        else if(tkhd->version == 1)
          quicktime_write_int64(file, tkhd->duration);
	quicktime_write_data(file, tkhd->reserved2, 8);
	quicktime_write_int16(file, tkhd->layer);
	quicktime_write_int16(file, tkhd->alternate_group);
	quicktime_write_fixed16(file, tkhd->volume);
	quicktime_write_int16(file, tkhd->reserved3);
	quicktime_write_matrix(file, &tkhd->matrix);
 	quicktime_write_fixed32(file, tkhd->track_width);
 	quicktime_write_fixed32(file, tkhd->track_height);
	quicktime_atom_write_footer(file, &atom);
}


void quicktime_tkhd_init_video(quicktime_t *file, 
								quicktime_tkhd_t *tkhd, 
								int frame_w, 
								int frame_h)
{
	tkhd->track_width = frame_w;
	tkhd->track_height = frame_h;
	tkhd->volume = 0;
}

void quicktime_tkhd_init_timecode(quicktime_t *file,
                                  quicktime_tkhd_t *tkhd,
                                  int frame_w,
                                  int frame_h)
  {
  tkhd->track_width = frame_w;
  tkhd->track_height = 20;
  tkhd->matrix.values[7] = frame_h;
  tkhd->volume = 0;
  }

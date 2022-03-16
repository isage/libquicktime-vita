/*******************************************************************************
 mvhd.c

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

int quicktime_mvhd_init(quicktime_mvhd_t *mvhd)
{
	int i;
	mvhd->version = 0;
	mvhd->flags = 0;
	mvhd->creation_time = quicktime_current_time();
	mvhd->modification_time = quicktime_current_time();
	mvhd->time_scale = 600;
	mvhd->duration = 0;
	mvhd->preferred_rate = 1.0;
	mvhd->preferred_volume = 1.0;
	for(i = 0; i < 10; i++) mvhd->reserved[i] = 0;
	quicktime_matrix_init(&mvhd->matrix);
	mvhd->preview_time = 0;
	mvhd->preview_duration = 0;
	mvhd->poster_time = 0;
	mvhd->selection_time = 0;
	mvhd->selection_duration = 0;
	mvhd->current_time = 0;
	mvhd->next_track_id = 1;
	return 0;
}

int quicktime_mvhd_delete(quicktime_mvhd_t *mvhd)
{
	return 0;
}

void quicktime_mvhd_dump(quicktime_mvhd_t *mvhd)
{
	lqt_dump(" movie header (mvhd)\n");
	lqt_dump("  version %d\n", mvhd->version);
	lqt_dump("  flags %ld\n", mvhd->flags);
	lqt_dump("  creation_time ");
        lqt_dump_time(mvhd->creation_time);
        lqt_dump("\n");
	lqt_dump("  modification_time ");
        lqt_dump_time(mvhd->modification_time);
        lqt_dump("\n");
	lqt_dump("  time_scale %ld\n", mvhd->time_scale);
	lqt_dump("  duration %"PRId64"\n", mvhd->duration);
	lqt_dump("  preferred_rate %f\n", mvhd->preferred_rate);
	lqt_dump("  preferred_volume %f\n", mvhd->preferred_volume);
	quicktime_print_chars("  reserved ", mvhd->reserved, 10);
	quicktime_matrix_dump(&mvhd->matrix);
	lqt_dump("  preview_time %ld\n", mvhd->preview_time);
	lqt_dump("  preview_duration %ld\n", mvhd->preview_duration);
	lqt_dump("  poster_time %ld\n", mvhd->poster_time);
	lqt_dump("  selection_time %ld\n", mvhd->selection_time);
	lqt_dump("  selection_duration %ld\n", mvhd->selection_duration);
	lqt_dump("  current_time %ld\n", mvhd->current_time);
	lqt_dump("  next_track_id %ld\n", mvhd->next_track_id);
}

void quicktime_read_mvhd(quicktime_t *file, quicktime_mvhd_t *mvhd, quicktime_atom_t *parent_atom)
{
	mvhd->version = quicktime_read_char(file);
	mvhd->flags = quicktime_read_int24(file);

        if(mvhd->version == 0)
          {
          mvhd->creation_time = quicktime_read_int32(file);
          mvhd->modification_time = quicktime_read_int32(file);
          }
        else if(mvhd->version == 1)
          {
          mvhd->creation_time = quicktime_read_int64(file);
          mvhd->modification_time = quicktime_read_int64(file);
          }
	mvhd->time_scale = quicktime_read_int32(file);
	if(mvhd->version == 0)
          mvhd->duration = quicktime_read_int32(file);
        else if(mvhd->version == 1)
          mvhd->duration = quicktime_read_int64(file);
	mvhd->preferred_rate = quicktime_read_fixed32(file);
	mvhd->preferred_volume = quicktime_read_fixed16(file);
	quicktime_read_data(file, mvhd->reserved, 10);
	quicktime_read_matrix(file, &mvhd->matrix);
	mvhd->preview_time = quicktime_read_int32(file);
	mvhd->preview_duration = quicktime_read_int32(file);
	mvhd->poster_time = quicktime_read_int32(file);
	mvhd->selection_time = quicktime_read_int32(file);
	mvhd->selection_duration = quicktime_read_int32(file);
	mvhd->current_time = quicktime_read_int32(file);
	mvhd->next_track_id = quicktime_read_int32(file);
}

void quicktime_mhvd_init_video(quicktime_t *file, quicktime_mvhd_t *mvhd, int timescale)
  {
  if((mvhd->time_scale % timescale) || (mvhd->time_scale < timescale))
    mvhd->time_scale = timescale;
  }

void quicktime_write_mvhd(quicktime_t *file, quicktime_mvhd_t *mvhd)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "mvhd");

	quicktime_write_char(file, mvhd->version);
	quicktime_write_int24(file, mvhd->flags);

        if(mvhd->version == 0)
          {
          quicktime_write_int32(file, mvhd->creation_time);
          quicktime_write_int32(file, mvhd->modification_time);
          }
        else if(mvhd->version == 1)
          {
          quicktime_write_int64(file, mvhd->creation_time);
          quicktime_write_int64(file, mvhd->modification_time);
          }
	quicktime_write_int32(file, mvhd->time_scale);
        if(mvhd->version == 0)
          quicktime_write_int32(file, mvhd->duration);
        else if(mvhd->version == 1)
          quicktime_write_int64(file, mvhd->duration);
        
	quicktime_write_fixed32(file, mvhd->preferred_rate);
	quicktime_write_fixed16(file, mvhd->preferred_volume);
	quicktime_write_data(file, mvhd->reserved, 10);
	quicktime_write_matrix(file, &mvhd->matrix);
	quicktime_write_int32(file, mvhd->preview_time);
	quicktime_write_int32(file, mvhd->preview_duration);
	quicktime_write_int32(file, mvhd->poster_time);
	quicktime_write_int32(file, mvhd->selection_time);
	quicktime_write_int32(file, mvhd->selection_duration);
	quicktime_write_int32(file, mvhd->current_time);
	quicktime_write_int32(file, mvhd->next_track_id);

	quicktime_atom_write_footer(file, &atom);
}

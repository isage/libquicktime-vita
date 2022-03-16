/*******************************************************************************
 mdhd.c

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

#define MP4_DEFAULT_LANGUAGE 5575 // English

void quicktime_mdhd_init(quicktime_mdhd_t *mdhd)
{
	mdhd->version = 0;
	mdhd->flags = 0;
	mdhd->creation_time = quicktime_current_time();
	mdhd->modification_time = quicktime_current_time();
	mdhd->time_scale = 0;
	mdhd->duration = 0;
	mdhd->language = 0;
	mdhd->quality = 100;
}

void quicktime_mdhd_init_video(quicktime_t *file, 
								quicktime_mdhd_t *mdhd,
								int timescale)
  {
  if(IS_MP4(file->file_type))
    {
    mdhd->language = MP4_DEFAULT_LANGUAGE;
    mdhd->quality = 0;
    }
  mdhd->time_scale = timescale;
  mdhd->duration = 0;      /* set this when closing */
  }

void quicktime_mdhd_init_audio(quicktime_t *file,
                               quicktime_mdhd_t *mdhd, 
                               int sample_rate)
  {
  if(IS_MP4(file->file_type))
    {
    mdhd->language = MP4_DEFAULT_LANGUAGE;
    mdhd->quality = 0;
    }
  mdhd->time_scale = sample_rate;
  mdhd->duration = 0;      /* set this when closing */
  }
  
void quicktime_mdhd_init_text(quicktime_t *file,
                              quicktime_mdhd_t *mdhd, 
                              int timescale)
  {
  if(IS_MP4(file->file_type))
    {
    mdhd->language = MP4_DEFAULT_LANGUAGE;
    mdhd->quality = 0;
    }
  mdhd->time_scale = timescale;
  mdhd->duration = 0;      /* set this when closing */
  }


void quicktime_mdhd_delete(quicktime_mdhd_t *mdhd)
  {
  }

void quicktime_read_mdhd(quicktime_t *file, quicktime_mdhd_t *mdhd)
{
	mdhd->version = quicktime_read_char(file);
	mdhd->flags = quicktime_read_int24(file);
	mdhd->creation_time = quicktime_read_int32(file);
	mdhd->modification_time = quicktime_read_int32(file);
	mdhd->time_scale = quicktime_read_int32(file);
	mdhd->duration = quicktime_read_int32(file);
	mdhd->language = quicktime_read_int16(file);
	mdhd->quality = quicktime_read_int16(file);
}

void quicktime_mdhd_dump(quicktime_mdhd_t *mdhd)
{
	lqt_dump("   media header (mdhd)\n");
	lqt_dump("    version %d\n", mdhd->version);
	lqt_dump("    flags %ld\n", mdhd->flags);
	lqt_dump("    creation_time ");
        lqt_dump_time(mdhd->creation_time);
        lqt_dump("\n");
	lqt_dump("    modification_time ");
        lqt_dump_time(mdhd->modification_time);
        lqt_dump("\n");
	lqt_dump("    time_scale %ld\n", mdhd->time_scale);
	lqt_dump("    duration %ld\n", mdhd->duration);
	lqt_dump("    language %d\n", mdhd->language);
	lqt_dump("    quality %d\n", mdhd->quality);
}

void quicktime_write_mdhd(quicktime_t *file, quicktime_mdhd_t *mdhd)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "mdhd");

	quicktime_write_char(file, mdhd->version);
	quicktime_write_int24(file, mdhd->flags);
	quicktime_write_int32(file, mdhd->creation_time);
	quicktime_write_int32(file, mdhd->modification_time);
	quicktime_write_int32(file, mdhd->time_scale);
	quicktime_write_int32(file, mdhd->duration);
	quicktime_write_int16(file, mdhd->language);
	quicktime_write_int16(file, mdhd->quality);	

	quicktime_atom_write_footer(file, &atom);
}


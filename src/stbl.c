/*******************************************************************************
 stbl.c

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

void quicktime_stbl_init(quicktime_stbl_t *stbl)
  {
  stbl->version = 0;
  stbl->flags = 0;
  quicktime_stsd_init(&stbl->stsd);
  quicktime_stts_init(&stbl->stts);
  quicktime_stss_init(&stbl->stss);
  quicktime_stsc_init(&stbl->stsc);
  quicktime_stsz_init(&stbl->stsz);
  quicktime_stco_init(&stbl->stco);
  }


void quicktime_stbl_init_qtvr(quicktime_t *file,
                              quicktime_stbl_t *stbl,
                              int track_type,
                              int frame_duration)
  {
  quicktime_stsd_init_qtvr(file, &stbl->stsd, track_type);
  quicktime_stts_init_video(file, &stbl->stts, frame_duration);
  quicktime_stsc_init_video(file, &stbl->stsc);
  quicktime_stsz_init_video(file, &stbl->stsz);
  quicktime_stco_init_common(file, &stbl->stco);
  }


void quicktime_stbl_init_panorama(quicktime_t *file,
                                  quicktime_stbl_t *stbl,
                                  int width,
                                  int height,
                                  int frame_duration)
  {
  quicktime_stsd_init_panorama(file, &stbl->stsd, width, height);
  quicktime_stts_init_video(file, &stbl->stts, frame_duration);
  quicktime_stsc_init_video(file, &stbl->stsc);
  quicktime_stsz_init_video(file, &stbl->stsz);
  quicktime_stco_init_common(file, &stbl->stco);
  }


void quicktime_stbl_init_video(quicktime_t *file, 
								quicktime_stbl_t *stbl, 
								int frame_w,
								int frame_h, 
                                                                int frame_duration,
								int time_scale, 
								char *compressor)
{
	quicktime_stsd_init_video(file, &stbl->stsd, frame_w, frame_h, compressor);
	quicktime_stts_init_video(file, &stbl->stts, frame_duration);
	quicktime_stsc_init_video(file, &stbl->stsc);
	quicktime_stsz_init_video(file, &stbl->stsz);
	quicktime_stco_init_common(file, &stbl->stco);
}


void quicktime_stbl_init_audio(quicktime_t *file, 
							quicktime_stbl_t *stbl, 
							int channels, 
							int sample_rate, 
							int bits, 
							char *compressor)
{
	quicktime_stsd_init_audio(file, &stbl->stsd, 
		channels, 
                                  sample_rate, 
                                  bits, 
                                  compressor);
	quicktime_stts_init_audio(file, &stbl->stts, sample_rate);
	quicktime_stsc_init_audio(file, &stbl->stsc, sample_rate);
	quicktime_stsz_init_audio(file, &stbl->stsz, channels, bits, compressor);
	quicktime_stco_init_common(file, &stbl->stco);
}

void quicktime_stbl_init_text(quicktime_t *file,
                              quicktime_stbl_t *stbl)
  {
  quicktime_stco_init_common(file, &stbl->stco);
  quicktime_stsd_init_text(file, &stbl->stsd);
  /* stts doesn't need to be updated */
  }

void quicktime_stbl_init_tx3g(quicktime_t *file,
                              quicktime_stbl_t *stbl)
  {
  quicktime_stco_init_common(file, &stbl->stco);
  quicktime_stsd_init_tx3g(file, &stbl->stsd);
  /* stts doesn't need to be updated */
  }

void quicktime_stbl_init_timecode(quicktime_t *file,
                                  quicktime_stbl_t *stbl,
                                  int time_scale,
                                  int frame_duration,
                                  int num_frames,
                                  uint32_t flags)
  {
  quicktime_stsd_init_timecode(file, &stbl->stsd, time_scale,
                               frame_duration, num_frames, flags);
  quicktime_stts_init_timecode(file, &stbl->stts);
  quicktime_stsc_init_video(file, &stbl->stsc);
  quicktime_stsz_init_timecode(&stbl->stsz);
  quicktime_stco_init_common(file, &stbl->stco);
  }


void quicktime_stbl_delete(quicktime_stbl_t *stbl)
  {
  quicktime_stsd_delete(&stbl->stsd);
  quicktime_stts_delete(&stbl->stts);
  quicktime_stss_delete(&stbl->stss);
  quicktime_stsc_delete(&stbl->stsc);
  quicktime_stsz_delete(&stbl->stsz);
  quicktime_stco_delete(&stbl->stco);
  }

void quicktime_stbl_dump(void *minf_ptr, quicktime_stbl_t *stbl)
  {
  lqt_dump("    sample table\n");
  quicktime_stsd_dump(minf_ptr, &stbl->stsd);
  quicktime_stts_dump(&stbl->stts);
  if(stbl->stss.total_entries)
    quicktime_stss_dump(&stbl->stss);
  quicktime_stsc_dump(&stbl->stsc);
  quicktime_stsz_dump(&stbl->stsz);
  quicktime_stco_dump(&stbl->stco);
  if(stbl->has_ctts)
    quicktime_ctts_dump(&stbl->ctts);
        
  }

int quicktime_read_stbl(quicktime_t *file, quicktime_minf_t *minf,
                        quicktime_stbl_t *stbl, quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  
  do
    {
    quicktime_atom_read_header(file, &leaf_atom);

    /* mandatory */
    if(quicktime_atom_is(&leaf_atom, "stsd"))
      { 
      quicktime_read_stsd(file, &stbl->stsd); 
      /* Some codecs store extra information at the end of this */
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "stts"))
      {
      quicktime_read_stts(file, &stbl->stts);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "ctts"))
      {
      quicktime_read_ctts(file, &stbl->ctts);
      quicktime_atom_skip(file, &leaf_atom);
      stbl->has_ctts = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "stss"))
      {
      quicktime_read_stss(file, &stbl->stss);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "stsc"))
      {
      quicktime_read_stsc(file, &stbl->stsc);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "stsz"))
      {
      quicktime_read_stsz(file, &stbl->stsz);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "co64"))
      {
      quicktime_read_stco64(file, &stbl->stco);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "stco"))
      {
      quicktime_read_stco(file, &stbl->stco);
      quicktime_atom_skip(file, &leaf_atom);
      }
    else
      quicktime_atom_skip(file, &leaf_atom);
    }while(quicktime_position(file) < parent_atom->end);

  return 0;
  }

void quicktime_write_stbl(quicktime_t *file, quicktime_minf_t *minf, quicktime_stbl_t *stbl)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "stbl");

  quicktime_write_stsd(file, minf, &stbl->stsd);
  quicktime_write_stts(file, &stbl->stts);
  quicktime_write_stss(file, &stbl->stss);
  quicktime_write_stsc(file, &stbl->stsc);
  quicktime_write_stsz(file, &stbl->stsz);
  quicktime_write_stco(file, &stbl->stco);
  if(stbl->has_ctts)
    quicktime_write_ctts(file, &stbl->ctts);
        
  quicktime_atom_write_footer(file, &atom);
  }



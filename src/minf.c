/*******************************************************************************
 minf.c

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

void quicktime_minf_init(quicktime_minf_t *minf)
  {
  minf->is_video = minf->is_audio = 0;
  minf->is_panorama = 0;
  minf->is_qtvr = 0;
  minf->is_object = 0;
  quicktime_vmhd_init(&minf->vmhd);
  quicktime_smhd_init(&minf->smhd);
  quicktime_hdlr_init(&minf->hdlr);
  quicktime_dinf_init(&minf->dinf);
  quicktime_stbl_init(&minf->stbl);
  }

void quicktime_minf_init_qtvr(quicktime_t *file,
                              quicktime_minf_t *minf,
                              int track_type,
                              int frame_duration)
  {
  minf->is_qtvr = track_type;
  quicktime_stbl_init_qtvr(file, &minf->stbl, track_type, frame_duration);
  quicktime_hdlr_init_data(&minf->hdlr);
  minf->has_hdlr = 1;
  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  quicktime_gmhd_init(&minf->gmhd);
  minf->has_gmhd = 1;
  }


void quicktime_minf_init_panorama(quicktime_t *file,
                                  quicktime_minf_t *minf,
                                  int width,
                                  int height,
                                  int frame_duration)
  {
  minf->is_panorama = 1;
  quicktime_stbl_init_panorama(file, &minf->stbl, width, height, frame_duration);
  quicktime_hdlr_init_data(&minf->hdlr);
  minf->has_hdlr = 1;

  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  quicktime_gmhd_init(&minf->gmhd);
  minf->has_gmhd = 1;
  }


void quicktime_minf_init_video(quicktime_t *file, 
                               quicktime_minf_t *minf, 
                               int frame_w,
                               int frame_h, 
                               int frame_duration,
                               int time_scale, 
                               char *compressor)
  {
  minf->is_video = 1;
  quicktime_vmhd_init_video(file, &minf->vmhd, frame_w, frame_h, frame_duration, time_scale);
  quicktime_stbl_init_video(file, &minf->stbl, frame_w, frame_h, frame_duration, time_scale, compressor);

  if(!IS_MP4(file->file_type))
    {
    quicktime_hdlr_init_data(&minf->hdlr);
    minf->has_hdlr = 1;
    }
  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  }

void quicktime_minf_init_audio(quicktime_t *file, 
                               quicktime_minf_t *minf, 
                               int channels, 
                               int sample_rate, 
                               int bits, 
                               char *compressor)
  {
  minf->is_audio = 1;
  /* smhd doesn't store anything worth initializing */
  quicktime_stbl_init_audio(file, &minf->stbl, channels, sample_rate, bits, compressor);

  if(!IS_MP4(file->file_type))
    {
    quicktime_hdlr_init_data(&minf->hdlr);
    minf->has_hdlr = 1;
    }
  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  }

void quicktime_minf_init_text(quicktime_t *file, 
                              quicktime_minf_t *minf)
  {
  minf->is_text = 1;
  quicktime_hdlr_init_data(&minf->hdlr);
  minf->has_hdlr = 1;

  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  quicktime_stbl_init_text(file, &minf->stbl);
  quicktime_gmhd_init(&minf->gmhd);
  minf->has_gmhd = 1;
  quicktime_gmhd_text_init(&minf->gmhd.gmhd_text);
  minf->gmhd.has_gmhd_text = 1;
  }

void quicktime_minf_init_tx3g(quicktime_t *file, 
                              quicktime_minf_t *minf)
  {
  minf->is_text = 1;
  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  minf->dinf.dref.table[0].type[0] = 'u';
  minf->dinf.dref.table[0].type[1] = 'r';
  minf->dinf.dref.table[0].type[2] = 'l';
  minf->dinf.dref.table[0].type[3] = ' ';
  
  quicktime_stbl_init_tx3g(file, &minf->stbl);
  quicktime_nmhd_init(&minf->nmhd);
  minf->has_nmhd = 1;
  
  }

void quicktime_minf_init_timecode(quicktime_t *file,
                                  quicktime_minf_t *minf,
                                  int time_scale,
                                  int frame_duration,
                                  int num_frames,
                                  uint32_t flags)
  {
  minf->is_timecode = 1;
  minf->has_gmhd = 1;
  quicktime_gmhd_init_timecode(&minf->gmhd);
  quicktime_stbl_init_timecode(file, &minf->stbl, time_scale, frame_duration,
                               num_frames, flags);
  quicktime_hdlr_init_data(&minf->hdlr);
  quicktime_dinf_init_all(&minf->dinf, file->file_type);
  }

void quicktime_minf_delete(quicktime_minf_t *minf)
  {
  quicktime_vmhd_delete(&minf->vmhd);
  quicktime_smhd_delete(&minf->smhd);
  quicktime_gmhd_delete(&minf->gmhd);
  quicktime_dinf_delete(&minf->dinf);
  quicktime_stbl_delete(&minf->stbl);
  quicktime_hdlr_delete(&minf->hdlr);
  }

void quicktime_minf_dump(quicktime_minf_t *minf)
  {
  lqt_dump("   media info (minf)\n");
  lqt_dump("    is_audio     %d\n", minf->is_audio);
  lqt_dump("    is_audio_vbr %d\n", minf->is_audio_vbr);
  lqt_dump("    is_video     %d\n", minf->is_video);
  lqt_dump("    is_text      %d\n", minf->is_text);
  lqt_dump("    is_timecode  %d\n", minf->is_timecode);
  if(minf->is_audio) quicktime_smhd_dump(&minf->smhd);
  if(minf->is_video) quicktime_vmhd_dump(&minf->vmhd);
  if(minf->has_gmhd) quicktime_gmhd_dump(&minf->gmhd);
  if(minf->has_nmhd) quicktime_nmhd_dump(&minf->nmhd);
  if(minf->has_hdlr) quicktime_hdlr_dump(&minf->hdlr);
  quicktime_dinf_dump(&minf->dinf);
  quicktime_stbl_dump(minf, &minf->stbl);
  }

int quicktime_read_minf(quicktime_t *file, quicktime_trak_t *trak,
                        quicktime_minf_t *minf, quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  do
    {
    quicktime_atom_read_header(file, &leaf_atom);
    
    /* mandatory */
    if(quicktime_atom_is(&leaf_atom, "vmhd"))
      {
      minf->is_video = 1; quicktime_read_vmhd(file, &minf->vmhd);
      }
    else if(quicktime_atom_is(&leaf_atom, "smhd"))
      {
      minf->is_audio = 1;
      quicktime_read_smhd(file, &minf->smhd);
      }
    else if(quicktime_atom_is(&leaf_atom, "gmhd"))
      {
      minf->has_gmhd = 1;
      quicktime_read_gmhd(file, &minf->gmhd, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "nmhd"))
      {
      minf->has_nmhd = 1;
      quicktime_read_nmhd(file, &minf->nmhd);
      }
    else if(quicktime_atom_is(&leaf_atom, "hdlr"))
      { 
      quicktime_read_hdlr(file, &minf->hdlr, &leaf_atom);
      minf->has_hdlr = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "dinf"))
      {
      quicktime_read_dinf(file, &minf->dinf, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "stbl"))
      {
      quicktime_read_stbl(file, minf, &minf->stbl, &leaf_atom);
      }
    else
      quicktime_atom_skip(file, &leaf_atom);
    }while(quicktime_position(file) < parent_atom->end);

  /* Finalize stsd */
  quicktime_finalize_stsd(file, trak, &minf->stbl.stsd);
  
  if(minf->is_audio && (minf->stbl.stsd.table[0].compression_id == -2))
    minf->is_audio_vbr = 1;
  return 0;
  }

void quicktime_write_minf(quicktime_t *file, quicktime_minf_t *minf)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "minf");

  if(minf->is_video) quicktime_write_vmhd(file, &minf->vmhd);
  else if(minf->is_audio) quicktime_write_smhd(file, &minf->smhd);
  else if(minf->has_gmhd) quicktime_write_gmhd(file, &minf->gmhd);
  else if(minf->has_nmhd) quicktime_write_nmhd(file, &minf->nmhd);
  
  if(minf->has_hdlr) quicktime_write_hdlr(file, &minf->hdlr);
  quicktime_write_dinf(file, &minf->dinf);
  quicktime_write_stbl(file, minf, &minf->stbl);

  quicktime_atom_write_footer(file, &atom);
  }

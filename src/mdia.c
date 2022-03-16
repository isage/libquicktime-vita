/*******************************************************************************
 mdia.c

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

void quicktime_mdia_init(quicktime_mdia_t *mdia)
  {
  quicktime_mdhd_init(&mdia->mdhd);
  quicktime_hdlr_init(&mdia->hdlr);
  quicktime_minf_init(&mdia->minf);
  }

void quicktime_mdia_init_qtvr(quicktime_t *file,
                              quicktime_mdia_t *mdia,
                              int track_type,
                              int timescale,
                              int frame_duration)
  {
  quicktime_mdhd_init_video(file, &mdia->mdhd, timescale);
  quicktime_minf_init_qtvr(file, &mdia->minf, track_type, frame_duration);
  quicktime_hdlr_init_qtvr(&mdia->hdlr, track_type);
  }

void quicktime_mdia_init_panorama(quicktime_t *file,
                                  quicktime_mdia_t *mdia,
                                  int width,
                                  int height,
                                  int timescale,
                                  int frame_duration)
  {
  quicktime_mdhd_init_video(file, &mdia->mdhd, timescale);
  quicktime_minf_init_panorama(file, &mdia->minf, width, height, frame_duration);
  quicktime_hdlr_init_panorama(&mdia->hdlr);
  }

void quicktime_mdia_init_video(quicktime_t *file, 
                               quicktime_mdia_t *mdia,
                               int frame_w,
                               int frame_h, 
                               int frame_duration,
                               int timescale,
                               char *compressor)
  {
  quicktime_mdhd_init_video(file, &mdia->mdhd, timescale);
  quicktime_minf_init_video(file, &mdia->minf, frame_w, frame_h,
                            frame_duration, timescale, compressor);
  quicktime_hdlr_init_video(&mdia->hdlr);
  }

void quicktime_mdia_init_timecode(quicktime_t *file,
                                  quicktime_mdia_t *mdia,
                                  int time_scale,
                                  int frame_duration,
                                  int num_frames,
                                  uint32_t flags)
  {
  quicktime_mdhd_init_video(file, &mdia->mdhd, time_scale);
  quicktime_minf_init_timecode(file, &mdia->minf, time_scale,
                               frame_duration, num_frames, flags);
  quicktime_hdlr_init_timecode(&mdia->hdlr);
  }

void quicktime_mdia_init_audio(quicktime_t *file, 
                               quicktime_mdia_t *mdia, 
                               int channels,
                               int sample_rate, 
                               int bits, 
                               char *compressor)
  {
  quicktime_mdhd_init_audio(file,
                            &mdia->mdhd,
                            sample_rate);
  quicktime_minf_init_audio(file,
                            &mdia->minf,
                            channels,
                            sample_rate,
                            bits,
                            compressor);
  quicktime_hdlr_init_audio(&mdia->hdlr);
  }

void quicktime_mdia_init_text(quicktime_t * file,
                              quicktime_mdia_t * mdia, 
                              int timescale)
  {
  quicktime_hdlr_init_text(&mdia->hdlr);
  quicktime_mdhd_init_text(file, &mdia->mdhd,
                           timescale);

  quicktime_minf_init_text(file, &mdia->minf);
  }

void quicktime_mdia_init_tx3g(quicktime_t * file,
                              quicktime_mdia_t * mdia, 
                              int timescale)
  {
  quicktime_hdlr_init_tx3g(&mdia->hdlr);
  quicktime_mdhd_init_text(file, &mdia->mdhd,
                           timescale);
  quicktime_minf_init_tx3g(file, &mdia->minf);
  }


void quicktime_mdia_delete(quicktime_mdia_t *mdia)
  {
  quicktime_mdhd_delete(&mdia->mdhd);
  quicktime_hdlr_delete(&mdia->hdlr);
  quicktime_minf_delete(&mdia->minf);
  }

void quicktime_mdia_dump(quicktime_mdia_t *mdia)
  {
  lqt_dump("  media (mdia)\n");
  quicktime_mdhd_dump(&mdia->mdhd);
  quicktime_hdlr_dump(&mdia->hdlr);
  quicktime_minf_dump(&mdia->minf);
  }

int quicktime_read_mdia(quicktime_t *file, quicktime_trak_t *trak,
                        quicktime_mdia_t *mdia, quicktime_atom_t *trak_atom)
  {
  quicktime_atom_t leaf_atom;

  do
    {
    quicktime_atom_read_header(file, &leaf_atom);

    /* mandatory */
    if(quicktime_atom_is(&leaf_atom, "mdhd"))
      quicktime_read_mdhd(file, &mdia->mdhd);
    else if(quicktime_atom_is(&leaf_atom, "hdlr"))
      quicktime_read_hdlr(file, &mdia->hdlr, &leaf_atom); 
    else if(quicktime_atom_is(&leaf_atom, "minf"))
      quicktime_read_minf(file, trak, &mdia->minf, &leaf_atom);
    else
      quicktime_atom_skip(file, &leaf_atom);
    }while(quicktime_position(file) < trak_atom->end);
  
  return 0;
  }

void quicktime_write_mdia(quicktime_t *file, quicktime_mdia_t *mdia)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "mdia");

  quicktime_write_mdhd(file, &mdia->mdhd);
  quicktime_write_hdlr(file, &mdia->hdlr);
  quicktime_write_minf(file, &mdia->minf);

  quicktime_atom_write_footer(file, &atom);
  }

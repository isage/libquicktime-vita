/*******************************************************************************
 avi_strl.c

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
#include <stdlib.h>
#include <string.h>

// Update during close:
// length
// samples per chunk
#define PADDING_SIZE 2048 /* Must be increased for codecs with really
                             large extradata (like Vorbis in AVI),
                             but we won't do such nonsense, will we? */


quicktime_strl_t* quicktime_new_strl()
{
	quicktime_strl_t *strl = calloc(1, sizeof(quicktime_strl_t));
	return strl;
}


void quicktime_init_strl(quicktime_t *file, 
                         quicktime_audio_map_t *atrack,
                         quicktime_video_map_t *vtrack,
                         quicktime_trak_t *trak,
                         quicktime_strl_t *strl)
  {
  quicktime_atom_t list_atom;
  quicktime_atom_t junk_atom;
  int i;
  trak->strl = strl;
  /* Construct tag */
  if(vtrack)
    {
    strl->tag[0] = '0' + (trak->tkhd.track_id - 1) / 10;
    strl->tag[1] = '0' + (trak->tkhd.track_id - 1) % 10;
    strl->tag[2] = 'd';
    strl->tag[3] = 'c';
    }
  else
    if(atrack)
      {
      strl->tag[0] = '0' + (trak->tkhd.track_id - 1) / 10;
      strl->tag[1] = '0' + (trak->tkhd.track_id - 1) % 10;
      strl->tag[2] = 'w';
      strl->tag[3] = 'b';
      }


  /* LIST 'strl' */
  quicktime_atom_write_header(file, &list_atom, "LIST");
  quicktime_write_char32(file, "strl");
  
  /* vids */
  if(vtrack)
    {
    strncpy(strl->strh.fccType, "vids", 4);
    strncpy(strl->strh.fccHandler, trak->mdia.minf.stbl.stsd.table[0].format, 4);
    
    /* framerate denominator */
    strl->strh.dwScale = trak->mdia.minf.stbl.stts.table[0].sample_duration;
    /* framerate numerator */
    strl->strh.dwRate  = trak->mdia.mdhd.time_scale;
    strl->strh.dwQuality = 10000;
    //    strl->strh.dwSampleSize = (int)(trak->tkhd.track_width * trak->tkhd.track_height) * 3;
    strl->strh.rcFrame.right  = trak->tkhd.track_width;
    strl->strh.rcFrame.bottom = trak->tkhd.track_height;
    
    strl->is_video = 1;
    }
  else if(atrack)
    {
    strncpy(strl->strh.fccType, "auds", 4);
    strl->strh.dwQuality = -1;
    strl->is_audio = 1;
    }
  strl->strh_offset = quicktime_position(file);
  quicktime_write_strh(file, &strl->strh);
  
  /* strf */
  
  if(vtrack)
    {
    strl->strf.bh.biSize = 40;
    strl->strf.bh.biWidth = trak->tkhd.track_width;
    strl->strf.bh.biHeight = trak->tkhd.track_height;
    strl->strf.bh.biPlanes = 1;
    strl->strf.bh.biBitCount = 24;
    strncpy(strl->strf.bh.biCompression, trak->mdia.minf.stbl.stsd.table[0].format, 4); 
    strl->strf.bh.biSizeImage = trak->tkhd.track_width * trak->tkhd.track_height * 3; // biSizeImage
    quicktime_write_strf_video(file, &strl->strf);
    }
  else if(atrack)
    {
    /* By now the codec is instantiated so the WAV ID is available. */
    strl->strf.wf.type = LQT_WAVEFORMAT_WAVEFORMATEX;
    strl->strf.wf.f.WAVEFORMAT.wFormatTag = atrack->wav_id;
    strl->strf.wf.f.WAVEFORMAT.nChannels =
      trak->mdia.minf.stbl.stsd.table[0].channels;
    strl->strf.wf.f.WAVEFORMAT.nSamplesPerSec = atrack->samplerate;
    quicktime_write_strf_audio(file, &strl->strf);
    }

  strl->end_pos = quicktime_position(file);
    
  /* We write junk for 2 reasons:
   *
   *  1. The strf chunks might grow
   *  2. The indx chunk might come as well
   */
  
  quicktime_atom_write_header(file, &junk_atom, "JUNK");
  for(i = 0; i < PADDING_SIZE; i ++)
    quicktime_write_char(file, 0);
  quicktime_atom_write_footer(file, &junk_atom);
  
  /* Initialize super index */
  if(file->file_type == LQT_FILE_AVI_ODML)
    quicktime_init_indx(file, &strl->indx, strl);
  
  quicktime_atom_write_footer(file, &list_atom);
  }



void quicktime_delete_strl(quicktime_strl_t *strl)
  {
  if(strl->is_video) quicktime_strf_delete_video(&strl->strf);
  if(strl->is_audio) quicktime_strf_delete_audio(&strl->strf);
  quicktime_delete_indx(&strl->indx);
  free(strl);
  }

void quicktime_read_strl(quicktime_t *file,
                         quicktime_strl_t *strl, 
                         quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  
  /* AVI translation: */
  /* vids -> trak */
  /* auds -> trak */
  /* Only one track is in each strl object */
  do
    {
    quicktime_atom_read_header(file, &leaf_atom);

    // strh
    if(quicktime_atom_is(&leaf_atom, "strh"))
      {
      quicktime_read_strh(file, &strl->strh, &leaf_atom);
      }

    else if(quicktime_atom_is(&leaf_atom, "strf"))
      {
      if(quicktime_match_32(strl->strh.fccType, "vids"))
        quicktime_read_strf_video(file, &strl->strf, &leaf_atom);
      else if(quicktime_match_32(strl->strh.fccType, "auds"))
        quicktime_read_strf_audio(file, &strl->strf, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "indx"))
      // Super index.
      // Read the super index + all the partial indexes now
      {
      quicktime_read_indx(file, strl, &leaf_atom);
      strl->have_indx = 1;
      }
    quicktime_atom_skip(file, &leaf_atom);
    } while(quicktime_position(file) < parent_atom->end);
  
  }



void quicktime_strl_2_qt(quicktime_t *file,
                         quicktime_strl_t *strl)
  {
  // These are 0 if no track is currently being processed.
  // Set to 1 if audio or video track is being processed.
  uint8_t codec[4] = { 0x00, 0x00, 0x00, 0x00 };
  //	int denominator;
  //	int numerator;
  int frame_duration = 0;
  int timescale = 0;
  int width = 0;
  int height = 0;
  int depth = 0;
  int frames = 0;
  int bytes_per_sample = 0;
  int bits_per_sample = 0;
  int samples;
  //	int samples_per_chunk = 0;
  int channels = 0;
  int sample_rate = 0;
  //	int bytes_per_second;
  quicktime_trak_t *trak = 0;
  
  /* AVI translation: */
  /* vids -> trak */
  /* auds -> trak */
  /* Only one track is in each strl object */
    
  if(quicktime_match_32(strl->strh.fccType, "vids"))
    {
    /* Video */
    
    trak = quicktime_add_trak(file);
    trak->strl = strl;
    width = 0;
    height = 0;
    depth = 24;
    frames = 0;
    strl->is_video = 1;
    
    
    trak->tkhd.track_id = file->moov.mvhd.next_track_id;
    file->moov.mvhd.next_track_id++;
    
    if(strl->strh.dwScale != 0)
      {
      timescale = strl->strh.dwRate;
      frame_duration = strl->strh.dwScale;
      //  frame_rate = (double)strl->dwRate / strl->dwScale;
      }
    else
      {
      // frame_rate = strl->dwRate;
      timescale = strl->strh.dwRate;
      frame_duration = 1;
      }
    frames = strl->strh.dwLength;      // dwLength

    width  = strl->strf.bh.biWidth;
    height = strl->strf.bh.biHeight;
    
    /* Depth in bits */
    depth = strl->strf.bh.biBitCount;
    
    /* Generate quicktime structures */
    quicktime_trak_init_video(file, 
                              trak, 
                              width, 
                              height, 
                              frame_duration,
                              timescale,
                              strl->strf.bh.biCompression);
    quicktime_mhvd_init_video(file, 
                              &file->moov.mvhd, 
                              timescale);
    trak->mdia.mdhd.duration = frames;
    //			trak->mdia.mdhd.time_scale = 1;
    trak->mdia.minf.stbl.stsd.table[0].depth = depth;

    
    }
  else if(quicktime_match_32(strl->strh.fccType, "auds"))
    {
    trak = quicktime_add_trak(file);
    trak->strl = strl;
    channels = 2;
    sample_rate = 0;
    strl->is_audio = 1;
    
    trak->tkhd.track_id = file->moov.mvhd.next_track_id;
    file->moov.mvhd.next_track_id++;

    samples = strl->strh.dwLength;           // dwLength
    bytes_per_sample = strl->strh.dwSampleSize; // dwSampleSize
    
    if(strl->strf.wf.type != LQT_WAVEFORMAT_WAVEFORMAT)
      {
      bits_per_sample = strl->strf.wf.f.PCMWAVEFORMAT.wBitsPerSample;
      }
    else
      bits_per_sample = 8;
    
    
    channels       = strl->strf.wf.f.WAVEFORMAT.nChannels;
    sample_rate    = strl->strf.wf.f.WAVEFORMAT.nSamplesPerSec;
    
    quicktime_trak_init_audio(file, 
                              trak, 
                              channels, 
                              sample_rate, 
                              bits_per_sample, 
                              (char*)codec);

    if((strl->strh.dwSampleSize == 0) && (strl->strh.dwScale > 1))
      trak->mdia.minf.is_audio_vbr = 1;
    
    // We store a constant samples per chunk based on the 
    // packet size if sample_size zero
    // and calculate the samples per chunk based on the chunk size if sample_size 
    // is nonzero.
    //		trak->mdia.minf.stbl.stsd.table[0].sample_size = bytes_per_sample;
    trak->mdia.minf.stbl.stsd.table[0].compression_id =
      strl->strf.wf.f.WAVEFORMAT.wFormatTag;
    
    /* Synthesize stsc table for constant samples per chunk */
    if(!bytes_per_sample)
      {
      /* Should be enough entries allocated in quicktime_stsc_init_table */
      trak->mdia.minf.stbl.stsc.table[0].samples = strl->strh.dwScale;
      trak->mdia.minf.stbl.stsc.total_entries = 1;
      }
    }
  }


void quicktime_finalize_strl(quicktime_t *file, quicktime_trak_t * trak,
                             quicktime_strl_t *strl)
  {
  int i;
  quicktime_atom_t junk_atom;
  
  int64_t old_pos, end_pos;
  /* Rewrite stream headers */

  if(!strl->strh.dwLength)
    strl->strh.dwLength = quicktime_track_samples(file, trak);
  
  //  if(trak->mdia.minf.is_audio)
  //    strl->strh.dwSuggestedBufferSize = strl->strf.wf.f.WAVEFORMAT.nAvgBytesPerSec / 2;
  
  old_pos = quicktime_position(file);
 
  quicktime_set_position(file, strl->strh_offset);
  quicktime_write_strh(file, &strl->strh);

  if(trak->mdia.minf.is_video)
    {
    quicktime_write_strf_video(file, &strl->strf);
    }
  else if(trak->mdia.minf.is_audio)
    {
    quicktime_write_strf_audio(file, &strl->strf);
    }

  end_pos = quicktime_position(file);
  
  // Finalize super indexes
  if(file->file_type == LQT_FILE_AVI_ODML)
    strl->indx.offset = end_pos;
  
  quicktime_atom_write_header(file, &junk_atom, "JUNK");
  for(i = 0; i < PADDING_SIZE - (end_pos - strl->end_pos); i ++)
    quicktime_write_char(file, 0);
  quicktime_atom_write_footer(file, &junk_atom);
  
  strl->indx.size = quicktime_position(file) - strl->indx.offset;
  }

void quicktime_strl_dump(quicktime_strl_t *strl)
  {
  lqt_dump("strl\n");
  quicktime_strh_dump(&strl->strh);

  if(!strncmp(strl->strh.fccType, "auds", 4))
    quicktime_strf_dump_audio(&strl->strf);
  if(!strncmp(strl->strh.fccType, "vids", 4))
    quicktime_strf_dump_video(&strl->strf);

  if(strl->have_indx)
    {
    quicktime_indx_dump(&strl->indx);
    }
  }

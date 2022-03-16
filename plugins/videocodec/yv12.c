/*******************************************************************************
 yv12.c

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
#include "workarounds.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>
#include <string.h>
#include "videocodec.h"

typedef struct
{
	int coded_w, coded_h;
	uint8_t *buffer;
        int buffer_alloc;
        int initialized;
} quicktime_yv12_codec_t;

static int delete_codec(quicktime_codec_t *codec_base)
{
	quicktime_yv12_codec_t *codec;

	codec = codec_base->priv;
        if(codec->buffer)
          free(codec->buffer);
	free(codec);
	return 0;
}

static void initialize(quicktime_video_map_t *vtrack)
  {
  quicktime_codec_t *codec_base = vtrack->codec;
  quicktime_yv12_codec_t *codec = codec_base->priv;
  if(!codec->initialized)
    {
    /* Init private items */
    codec->coded_w = (((int)vtrack->track->tkhd.track_width+1) / 2)*2;
    codec->coded_h = (((int)vtrack->track->tkhd.track_height+1) / 2)*2;
    codec->initialized = 1;
    }
  }

static int decode(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  uint8_t * src_ptr;
  uint8_t * dst_ptr;

  int bytes, i;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_yv12_codec_t *codec = vtrack->codec->priv;
  int y_size, uv_size;
  int result = 0;

  if(!row_pointers)
    {
    file->vtracks[track].stream_cmodel = BC_YUV420P;
    return 0;
    }
        
  initialize(vtrack);

  y_size  = codec->coded_w;
  uv_size = codec->coded_w / 2;
  
  bytes = lqt_read_video_frame(file, &codec->buffer, &codec->buffer_alloc,
                               vtrack->current_position, NULL, track);
  
  if(bytes <= 0)
    return -1;

  
  src_ptr = codec->buffer;

  /* Y */
  dst_ptr = row_pointers[0];
  for(i = 0; i < codec->coded_h; i++)
    {
    memcpy(dst_ptr, src_ptr, y_size);
    src_ptr += y_size;
    dst_ptr += file->vtracks[track].stream_row_span;
    }

  /* U */
  dst_ptr = row_pointers[1];
  for(i = 0; i < codec->coded_h/2; i++)
    {
    memcpy(dst_ptr, src_ptr, uv_size);
    src_ptr += uv_size;
    dst_ptr += file->vtracks[track].stream_row_span_uv;
    }

  /* V */
  dst_ptr = row_pointers[2];
  for(i = 0; i < codec->coded_h/2; i++)
    {
    memcpy(dst_ptr, src_ptr, uv_size);
    src_ptr += uv_size;
    dst_ptr += file->vtracks[track].stream_row_span_uv;
    }
  
  return result;
  }

static int encode(quicktime_t *file, unsigned char **row_pointers, int track)
{
	quicktime_video_map_t *vtrack = &file->vtracks[track];
	quicktime_yv12_codec_t *codec = vtrack->codec->priv;
	int result = 0;
	int y_size, uv_size;
        int i;
        uint8_t * src_ptr;
        
        if(!row_pointers)
          {
          file->vtracks[track].stream_cmodel = BC_YUV420P;
          return 0;
          }
        
        initialize(vtrack);

	y_size = codec->coded_w;
	uv_size = codec->coded_w / 2;

        lqt_write_frame_header(file, track,
                               vtrack->current_position,
                               -1, 0);
        
        /* Y */
        src_ptr = row_pointers[0];
        for(i = 0; i < codec->coded_h; i++)
          {
          result = !quicktime_write_data(file, src_ptr, y_size);
          src_ptr += file->vtracks[track].stream_row_span;
          if(result)
            return result;
          }

        /* U */
        src_ptr = row_pointers[1];
        for(i = 0; i < codec->coded_h/2; i++)
          {
          result = !quicktime_write_data(file, src_ptr, uv_size);
          src_ptr += file->vtracks[track].stream_row_span_uv;
          if(result)
            return result;
          }

        /* V */
        src_ptr = row_pointers[2];
        for(i = 0; i < codec->coded_h/2; i++)
          {
          result = !quicktime_write_data(file, src_ptr, uv_size);
          src_ptr += file->vtracks[track].stream_row_span_uv;
          if(result)
            return result;
          }
        lqt_write_frame_footer(file, track);
	return result;
}

void quicktime_init_codec_yv12(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  /* Init public items */
  codec_base->priv = calloc(1, sizeof(quicktime_yv12_codec_t));
  codec_base->delete_codec = delete_codec;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  }


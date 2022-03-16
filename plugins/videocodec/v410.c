/*******************************************************************************
 v410.c

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
#include "videocodec.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>

typedef struct
  {
  uint8_t *buffer;
  int buffer_alloc;
  } quicktime_v410_codec_t;

static int delete_codec(quicktime_codec_t *codec_base)
  {
  quicktime_v410_codec_t *codec;
  
  codec = codec_base->priv;
  if(codec->buffer)
    free(codec->buffer);
  free(codec);
  return 0;
  }

static int decode(quicktime_t *file, unsigned char **row_pointers, int track)
{
        uint32_t input_i;
        uint8_t * in_ptr;
        uint16_t * out_y, * out_u, * out_v;
        int i, j;
	int64_t bytes;
	int result = 0;
	quicktime_video_map_t *vtrack = &file->vtracks[track];
	quicktime_v410_codec_t *codec = vtrack->codec->priv;
	int width = vtrack->track->tkhd.track_width;
	int height = vtrack->track->tkhd.track_height;

        if(!row_pointers)
          {
          vtrack->stream_cmodel = BC_YUV444P16;
          return 0;
          }

        bytes = lqt_read_video_frame(file, &codec->buffer, &codec->buffer_alloc,
                                     vtrack->current_position, NULL, track);

        if(bytes <= 0)
          return -1;
        
        in_ptr = codec->buffer;
        
	for(i = 0; i < height; i++)
          {
          out_y = (uint16_t*)(row_pointers[0] + i * file->vtracks[track].stream_row_span);
          out_u = (uint16_t*)(row_pointers[1] + i * file->vtracks[track].stream_row_span_uv);
          out_v = (uint16_t*)(row_pointers[2] + i * file->vtracks[track].stream_row_span_uv);

          for(j = 0; j < width; j++)
            {
            /* v410 is LITTLE endian!! */
            input_i = in_ptr[0] | (in_ptr[1] << 8) | (in_ptr[2] << 16) | (in_ptr[3] << 24);

            *(out_v++) = (input_i & 0xffc00000) >> 16; /* V */
            *(out_y++) = (input_i & 0x3ff000) >> 6;    /* Y */
            *(out_u++) = (input_i & 0xffc) << 4;       /* U */

            in_ptr += 4;
            }
          }
	return result;
}







static int encode(quicktime_t *file, unsigned char **row_pointers, int track)
{
	quicktime_video_map_t *vtrack = &file->vtracks[track];
	quicktime_v410_codec_t *codec = vtrack->codec->priv;
	int width = vtrack->track->tkhd.track_width;
	int height = vtrack->track->tkhd.track_height;
	int bytes = width * height * 4;
	int result = 0;
	int i, j;
        uint16_t * in_y, * in_u, * in_v;
        uint8_t * out_ptr;
        uint32_t output_i;
        
        if(!row_pointers)
          {
          vtrack->stream_cmodel = BC_YUV444P16;
          return 0;
          }

        if(!codec->buffer)
          {
          lqt_set_fiel_uncompressed(file, track);
          lqt_set_colr_yuv_uncompressed(file, track);
          codec->buffer = malloc(width * height * 4);
          }
        out_ptr = codec->buffer;
	for(i = 0; i < height; i++)
          {
          in_y = (uint16_t*)(row_pointers[0] + i * file->vtracks[track].stream_row_span);
          in_u = (uint16_t*)(row_pointers[1] + i * file->vtracks[track].stream_row_span_uv);
          in_v = (uint16_t*)(row_pointers[2] + i * file->vtracks[track].stream_row_span_uv);
          
          for(j = 0; j < width; j++)
            {
            output_i =
              ((*in_v & 0xffc0) << 16) |
              ((*in_y & 0xffc0) << 6) |
              ((*in_u & 0xffc0) >> 4);
            *(out_ptr++) = (output_i & 0xff);
            *(out_ptr++) = (output_i & 0xff00) >> 8;
            *(out_ptr++) = (output_i & 0xff0000) >> 16;
            *(out_ptr++) = (output_i & 0xff000000) >> 24;
            in_y ++;
            in_u ++;
            in_v ++;
            }
          }
        
        lqt_write_frame_header(file, track,
                               vtrack->current_position,
                               -1, 0);

	result = !quicktime_write_data(file, codec->buffer, bytes);
        lqt_write_frame_footer(file, track);
	
	return result;
}

void quicktime_init_codec_v410(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  
  /* Init public items */
  codec_base->priv = calloc(1, sizeof(quicktime_v410_codec_t));
  codec_base->delete_codec = delete_codec;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  }


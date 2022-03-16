/*******************************************************************************
 yuv2.c

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
#include "videocodec.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>

/* U V values are signed but Y R G B values are unsigned! */
/*
 *      R = Y               + 1.40200 * V
 *      G = Y - 0.34414 * U - 0.71414 * V
 *      B = Y + 1.77200 * U
 */

/*
 *		Y =  0.2990 * R + 0.5870 * G + 0.1140 * B
 *		U = -0.1687 * R - 0.3310 * G + 0.5000 * B
 *		V =  0.5000 * R - 0.4187 * G - 0.0813 * B  
 */


typedef struct
  {
  unsigned char *buffer;
  int buffer_alloc;
  int coded_w;
  
  /* The YUV2 codec requires a bytes per line that is a multiple of 4 */
  int bytes_per_line;
  int initialized;

  int is_2vuy;
  int is_yuvs;
  uint8_t ** rows;
  } quicktime_yuv2_codec_t;

static int quicktime_delete_codec_yuv2(quicktime_codec_t *codec_base)
  {
  quicktime_yuv2_codec_t *codec;

  codec = codec_base->priv;
  if(codec->buffer)
    free(codec->buffer);
  if(codec->rows)
    free(codec->rows);
  free(codec);
  return 0;
  }

static void convert_encode_yuv2(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec,
                                unsigned char **row_pointers)
  {
  uint8_t *out_row, *in_y, *in_u, *in_v;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    out_row = codec->buffer + y * codec->bytes_per_line;

    in_y = row_pointers[0] + y * file->vtracks[track].stream_row_span;
    in_u = row_pointers[1] + y * file->vtracks[track].stream_row_span_uv;
    in_v = row_pointers[2] + y * file->vtracks[track].stream_row_span_uv;

    for(x = 0; x < width; )
      {
      *out_row++ = *in_y++;
      *out_row++ = (int)(*in_u++) - 128;
      *out_row++ = *in_y++;
      *out_row++ = (int)(*in_v++) - 128;
      x += 2;
      }
    }
  }

static void convert_decode_yuv2(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec,
                                unsigned char **row_pointers)
  {
  uint8_t *in_row, *out_y, *out_u, *out_v;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    in_row = codec->buffer + y * codec->bytes_per_line;
          
    out_y = row_pointers[0] + y * file->vtracks[track].stream_row_span;
    out_u = row_pointers[1] + y * file->vtracks[track].stream_row_span_uv;
    out_v = row_pointers[2] + y * file->vtracks[track].stream_row_span_uv;
          
    for(x = 0; x < width; )
      {
      *out_y++ = *in_row++;
      *out_u++ = (int)(*in_row++) + 128;
      *out_y++ = *in_row++;
      *out_v++ = (int)(*in_row++) + 128;
      x += 2;
      }
    }
  }

static void convert_encode_2vuy(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec, unsigned char **row_pointers)
  {
  uint8_t *in_row, *out_row;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    out_row = codec->buffer + y * codec->bytes_per_line;
    in_row = row_pointers[y];
    for(x = 0; x < width; )
      {
      out_row[0] = in_row[1]; /* U */
      out_row[1] = in_row[0]; /* Y */
      out_row[2] = in_row[3]; /* V */
      out_row[3] = in_row[2]; /* Y */
      x += 2;
      out_row += 4;
      in_row += 4;
      }
    }
  }

static void convert_decode_2vuy(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec, unsigned char **row_pointers)
  {
  uint8_t *in_row, *out_row;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    in_row = codec->buffer + y * codec->bytes_per_line;
    out_row = row_pointers[y];
    for(x = 0; x < width; )
      {
      out_row[1] = in_row[0]; /* U */
      out_row[0] = in_row[1]; /* Y */
      out_row[3] = in_row[2]; /* V */
      out_row[2] = in_row[3]; /* Y */
      x += 2;
      out_row += 4;
      in_row += 4;
      }
    }
  }

static void convert_encode_yuvs(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec, unsigned char **row_pointers)
  {
  uint8_t *in_row, *out_row;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    out_row = codec->buffer + y * codec->bytes_per_line;
    in_row = row_pointers[y];
    for(x = 0; x < width; )
      {
      out_row[0] = in_row[0]; /* U */
      out_row[1] = in_row[1]; /* Y */
      out_row[2] = in_row[2]; /* V */
      out_row[3] = in_row[3]; /* Y */
      x += 2;
      out_row += 4;
      in_row += 4;
      }
    }
  }

static void convert_decode_yuvs(quicktime_t * file, int track,
                                quicktime_yuv2_codec_t *codec, unsigned char **row_pointers)
  {
  uint8_t *in_row, *out_row;
  int y, x;
  int height = quicktime_video_height(file, track);
  int width  = quicktime_video_width(file, track);
  for(y = 0; y < height; y++)
    {
    in_row = codec->buffer + y * codec->bytes_per_line;
    out_row = row_pointers[y];
    for(x = 0; x < width; )
      {
      out_row[0] = in_row[0]; /* U */
      out_row[1] = in_row[1]; /* Y */
      out_row[2] = in_row[2]; /* V */
      out_row[3] = in_row[3]; /* Y */
      x += 2;
      out_row += 4;
      in_row += 4;
      }
    }
  }


static void initialize(quicktime_video_map_t *vtrack, quicktime_yuv2_codec_t *codec,
                       int width, int height)
  {
  int coded_w;
  if(!codec->initialized)
    {
    /* Init private items */
    coded_w = ((width+3)/4)*4;
    codec->bytes_per_line = coded_w * 2;
    codec->buffer_alloc = codec->bytes_per_line * height;
    codec->buffer = calloc(1, codec->buffer_alloc);
    codec->initialized = 1;
    }
  }

static int decode(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  int64_t bytes;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_yuv2_codec_t *codec = vtrack->codec->priv;
  int width = quicktime_video_width(file, track);
  int height = quicktime_video_height(file, track);
  int result = 0;

  if(!row_pointers)
    {
    if(codec->is_2vuy || codec->is_yuvs)
      vtrack->stream_cmodel = BC_YUV422;
    else
      vtrack->stream_cmodel = BC_YUVJ422P;
    return 0;
    }

  initialize(vtrack, codec, width, height);

  bytes = lqt_read_video_frame(file, &codec->buffer,
                               &codec->buffer_alloc,
                               vtrack->current_position, NULL, track);
        
  if(codec->is_2vuy)
    convert_decode_2vuy(file, track, codec, row_pointers);
  else if(codec->is_yuvs)
    convert_decode_yuvs(file, track, codec, row_pointers);
  else
    convert_decode_yuv2(file, track, codec, row_pointers);
        
  return result;
  }

static int encode(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_yuv2_codec_t *codec = vtrack->codec->priv;
  int result = 1;
  int width = vtrack->track->tkhd.track_width;
  int height = vtrack->track->tkhd.track_height;
  int64_t bytes;
  unsigned char *buffer;

  if(!row_pointers)
    {
    if(codec->is_2vuy || codec->is_yuvs)
      vtrack->stream_cmodel = BC_YUV422;
    else
      vtrack->stream_cmodel = BC_YUVJ422P;
    return 0;
    }

  if(!codec->initialized)
    {
    lqt_set_fiel_uncompressed(file, track);
    lqt_set_colr_yuv_uncompressed(file, track);
    }
        
  initialize(vtrack, codec, width, height);

  bytes = height * codec->bytes_per_line;
  buffer = codec->buffer;

  if(codec->is_2vuy)
    convert_encode_2vuy(file, track, codec, row_pointers);
  else if(codec->is_yuvs)
    convert_encode_yuvs(file, track, codec, row_pointers);
  else
    convert_encode_yuv2(file, track, codec, row_pointers);

  lqt_write_frame_header(file, track,
                         vtrack->current_position,
                         -1, 0);
  
  result = !quicktime_write_data(file, buffer, bytes);

  lqt_write_frame_footer(file, track);
  
  return result;
  }


void quicktime_init_codec_yuv2(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  /* Init public items */
  codec_base->priv = calloc(1, sizeof(quicktime_yuv2_codec_t));
  codec_base->delete_codec = quicktime_delete_codec_yuv2;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  }

void quicktime_init_codec_2vuy(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_yuv2_codec_t * codec;
  codec = calloc(1, sizeof(*codec));
  /* Init public items */
  codec_base->priv = codec;
  codec_base->delete_codec = quicktime_delete_codec_yuv2;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  codec->is_2vuy = 1;
  }

void quicktime_init_codec_yuvs(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_yuv2_codec_t * codec;
  codec = calloc(1, sizeof(*codec));
  /* Init public items */
  codec_base->priv = codec;
  codec_base->delete_codec = quicktime_delete_codec_yuv2;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  codec->is_yuvs = 1;
  }


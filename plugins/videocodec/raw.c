/*******************************************************************************
 raw.c

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

#include <stdio.h>

#include "lqt_private.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>
#include <string.h>
#include "videocodec.h"

#define LOG_DOMAIN "rawaudio"

#define PALETTE_2_RGB24(pal, indx, dst)     \
dst[0] = pal->red[indx] >> 8;\
dst[1] = pal->green[indx] >> 8;\
dst[2] = pal->blue[indx] >> 8;


typedef struct
{
	unsigned char *buffer;
        int buffer_alloc;
/* Support all possible depths */

        int bytes_per_line;
        void (*scanline_func)(uint8_t * src,
                              uint8_t * dst,
                              int num_pixels,
                              quicktime_ctab_t * pal);


} quicktime_raw_codec_t;


static void scanline_raw_1(uint8_t * src,
                           uint8_t * dst,
                           int num_pixels,
                           quicktime_ctab_t * pal)
  {
  int i, index;
  int counter = 0;
  for(i = 0; i < num_pixels; i++)
    {
    if(counter == 8)
      {
      counter = 0;
      src++;
      }
    index = (*src & 0x80) >> 7;
    PALETTE_2_RGB24(pal, index, dst);
    *src <<= 1;
    dst += 3;
    counter++;
    }
  }

static void scanline_raw_2(uint8_t * src,
                           uint8_t * dst,
                           int num_pixels,
                           quicktime_ctab_t * pal)
  {
  int i, index;
  int counter = 0;
  for(i = 0; i < num_pixels; i++)
    {
    if(counter == 4)
      {
      counter = 0;
      src++;
      }
    index = (*src & 0xc0) >> 6;
    PALETTE_2_RGB24(pal, index, dst);
    *src <<= 2;
    dst += 3;
    counter++;
    }
  }

static void scanline_raw_4(uint8_t * src,
                           uint8_t * dst,
                           int num_pixels,
                           quicktime_ctab_t * pal)
  {
  int i, index;
  int counter = 0;
  for(i = 0; i < num_pixels; i++)
    {
    if(counter == 2)
      {
      counter = 0;
      src++;
      }
    index = (*src & 0xF0) >> 4;
    PALETTE_2_RGB24(pal, index, dst);
    *src <<= 4;
    dst += 3;
    counter++;
    }
  }

static void scanline_raw_8(uint8_t * src,
                           uint8_t * dst,
                           int num_pixels,
                           quicktime_ctab_t * pal)
  {
  int i;
  for(i = 0; i < num_pixels; i++)
    {
    PALETTE_2_RGB24(pal, *src, dst);
    src++;
    dst+=3;
    }
  }

/* Ported from gavl */

// Masks for BGR16 and RGB16 formats
                                                                                
#define RGB15_LOWER_MASK  0x001f
#define RGB15_MIDDLE_MASK 0x03e0
#define RGB15_UPPER_MASK  0x7C00
                                                                                
// Extract unsigned char RGB values from 16 bit pixels
                                                                                
#define RGB15_TO_R(pixel) ((pixel & RGB15_UPPER_MASK)>>7)
#define RGB15_TO_G(pixel) ((pixel & RGB15_MIDDLE_MASK)>>2)
#define RGB15_TO_B(pixel) ((pixel & RGB15_LOWER_MASK)<<3)


static void scanline_raw_16(uint8_t * src,
                            uint8_t * dst,
                            int num_pixels,
                            quicktime_ctab_t * pal)
  {
  int i;
  uint16_t pixel;
                                                                                
  for(i = 0; i < num_pixels; i++)
    {
    pixel = (src[0] << 8) | (src[1]);

    dst[0] = RGB15_TO_R(pixel);
    dst[1] = RGB15_TO_G(pixel);
    dst[2] = RGB15_TO_B(pixel);

    src += 2;
    dst += 3;
    }
  }

static void scanline_raw_24(uint8_t * src,
                            uint8_t * dst,
                            int num_pixels,
                            quicktime_ctab_t * pal)
  {
  memcpy(dst, src, num_pixels * 3);
  }

/* RGBA!! */

static void scanline_raw_32(uint8_t * src,
                            uint8_t * dst,
                            int num_pixels,
                            quicktime_ctab_t * pal)
  {
  int i;
  for(i = 0; i < num_pixels; i++)
    {
    dst[0] = src[1];
    dst[1] = src[2];
    dst[2] = src[3];
    dst[3] = src[0];
    dst += 4;
    src += 4;
    }
  }

/* */

static int quicktime_delete_codec_raw(quicktime_codec_t *codec_base)
  {
  quicktime_raw_codec_t *codec = codec_base->priv;
  if(codec->buffer)
    free(codec->buffer);
  
  free(codec);
  return 0;
  }


static int source_cmodel(quicktime_t *file, int track)
{
	int depth = quicktime_video_depth(file, track);
	if(depth == 32)
		return BC_RGBA8888;
        else
                return BC_RGB888;
}

static int quicktime_decode_raw(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  int result = 0;
  quicktime_trak_t *trak = file->vtracks[track].track;
  int frame_depth = quicktime_video_depth(file, track);
  int height = trak->tkhd.track_height;
  int width = trak->tkhd.track_width;
  long bytes;
  int i;
  unsigned char *ptr;
  quicktime_ctab_t * ctab;
  quicktime_raw_codec_t *codec = file->vtracks[track].codec->priv;
        
  if(!row_pointers)
    {
    file->vtracks[track].stream_cmodel = source_cmodel(file, track);
    return 0;
    }
        
  ctab = &trak->mdia.minf.stbl.stsd.table->ctab;
                
        
  if(!codec->scanline_func)
    {
    switch(frame_depth)
      {
      case 1: /* 1 bpp palette */
        codec->bytes_per_line = width / 8;
        codec->scanline_func = scanline_raw_1;
        if(ctab->size < 2)
          {
          lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Palette missing or too small");
          return 0;
          }
        break;
      case 2: /* 2 bpp palette */
        codec->bytes_per_line = width / 4;
        codec->scanline_func = scanline_raw_2;
        if(ctab->size < 4)
          {
          lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Palette missing or too small");
          return 0;
          }
        break;
      case 4: /* 4 bpp palette */
        codec->bytes_per_line = width / 2;
        codec->scanline_func = scanline_raw_4;
        if(ctab->size < 16)
          {
          lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Palette missing or too small");
          return 0;
          }
        break;
      case 8: /* 8 bpp palette */
        codec->bytes_per_line = width;
        codec->scanline_func = scanline_raw_8;
        if(ctab->size < 256)
          {
          lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Palette missing or too small\n");
          return 0;
          }
        break;
      case 16: /* RGB565 */
        codec->bytes_per_line = width * 2;
        codec->scanline_func = scanline_raw_16;
        break;
      case 24: /* 24 RGB */
        codec->bytes_per_line = width * 3;
        codec->scanline_func = scanline_raw_24;
        break;
      case 32: /* 32 ARGB */
        codec->bytes_per_line = width * 4;
        codec->scanline_func = scanline_raw_32;
        break;
      case 34: /* 2 bit gray */
        codec->bytes_per_line = width / 4;
        //              codec->scanline_func = scanline_raw_2_gray;
        codec->scanline_func = scanline_raw_2;
        break;
      case 36: /* 4 bit gray */
        codec->bytes_per_line = width / 2;
        //              codec->scanline_func = scanline_raw_4_gray;
        codec->scanline_func = scanline_raw_4;
        break;
      case 40: /* 8 bit gray */
        codec->bytes_per_line = width;
        //              codec->scanline_func = scanline_raw_8_gray;
        codec->scanline_func = scanline_raw_8;
        break;
      }
    if(codec->bytes_per_line & 1)
      codec->bytes_per_line++;
    }
                


  /* Read data */

  bytes = lqt_read_video_frame(file, &codec->buffer, &codec->buffer_alloc,
                               file->vtracks[track].current_position, NULL, track);

  if(bytes <= 0)
    return -1;
        
  /* Do conversion of the scanlines */

  ptr = codec->buffer;

  for(i = 0; i < height; i++)
    {
    codec->scanline_func(ptr, row_pointers[i], width, ctab);
    ptr += codec->bytes_per_line;
    }
  return result;
  }

static int quicktime_encode_raw(quicktime_t *file, 
                                unsigned char **row_pointers, 
                                int track)
  {
  int i, j;
  uint8_t * in_ptr, *out_ptr;
  uint8_t padd = 0;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  int result = 0;
  int height = trak->tkhd.track_height;
  int width = trak->tkhd.track_width;
  quicktime_raw_codec_t *codec = file->vtracks[track].codec->priv;
        
  if(!row_pointers)
    {
    return 0;
    }

  if(!codec->bytes_per_line)
    {
    if(vtrack->stream_cmodel == BC_RGBA8888)
      {
      vtrack->track->mdia.minf.stbl.stsd.table[0].depth = 32;
      codec->bytes_per_line = width * 4;
      }
    else
      {
      vtrack->track->mdia.minf.stbl.stsd.table[0].depth = 24;
      codec->bytes_per_line = width * 3;
      }
    
    if(codec->bytes_per_line & 1)
      codec->bytes_per_line++;
    }
  
  lqt_write_frame_header(file, track,
                         vtrack->current_position,
                         -1, 0);

  if(vtrack->stream_cmodel == BC_RGBA8888)
    {
    if(!codec->buffer)
      codec->buffer = calloc(codec->bytes_per_line, 1);

    for(i = 0; i < height; i++)
      {
      in_ptr = row_pointers[i];
      out_ptr = codec->buffer;
      for(j = 0; j < width; j++)
        {
        out_ptr[1] = in_ptr[0]; /* R */
        out_ptr[2] = in_ptr[1]; /* G */
        out_ptr[3] = in_ptr[2]; /* B */
        out_ptr[0] = in_ptr[3]; /* A */
        out_ptr += 4;
        in_ptr  += 4;
        }
      result = !quicktime_write_data(file, codec->buffer, codec->bytes_per_line);
      }
    }
  else /* BC_RGB888 */
    {
    for(i = 0; i < height; i++)
      {
      result = !quicktime_write_data(file, row_pointers[i], width * 3);
      if(width & 1)
        result = !quicktime_write_data(file, &padd, 1);
      }
    }
  lqt_write_frame_footer(file, track);
  return result;
  }

void quicktime_init_codec_raw(quicktime_codec_t * codec_base,
                              quicktime_audio_map_t *atrack,
                              quicktime_video_map_t *vtrack)
  {
  
  codec_base->priv = calloc(1, sizeof(quicktime_raw_codec_t));
  codec_base->delete_codec = quicktime_delete_codec_raw;
  codec_base->decode_video = quicktime_decode_raw;
  codec_base->encode_video = quicktime_encode_raw;

  if(!vtrack)
    return;
  
  vtrack->stream_cmodel = BC_RGB888;
  }

void quicktime_init_codec_rawalpha(quicktime_codec_t * codec_base,
                                   quicktime_audio_map_t *atrack,
                                   quicktime_video_map_t *vtrack)
  {
  quicktime_init_codec_raw(codec_base, atrack, vtrack);

  if(!vtrack)
    return;
  vtrack->stream_cmodel = BC_RGBA8888;
  }

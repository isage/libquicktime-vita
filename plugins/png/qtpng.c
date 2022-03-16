/*******************************************************************************
 qtpng.c

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
#include <quicktime/colormodels.h>
#include <png.h>
#include <stdlib.h>
#include "qtpng.h"

typedef struct
  {
  int compression_level;
  unsigned char *buffer;
  // Read position
  long buffer_position;
  // Frame size
  long buffer_size;
  // Buffer allocation
  int buffer_alloc;
  unsigned char *temp_frame;

  int initialized;
  } quicktime_png_codec_t;


static int delete_codec(quicktime_codec_t *codec_base)
  {
  quicktime_png_codec_t *codec = codec_base->priv;
  if(codec->buffer)
    free(codec->buffer);
  if(codec->temp_frame)
    free(codec->temp_frame);
  free(codec);
  return 0;
  }

static int source_cmodel(quicktime_t *file, int track)
  {
  int depth = quicktime_video_depth(file, track);
  if(depth == 24) 
    return BC_RGB888;
  else
    return BC_RGBA8888;
  }


static void read_function(png_structp png_ptr, png_bytep data, png_uint_32 length)
  {
  quicktime_png_codec_t *codec = png_get_io_ptr(png_ptr);
	
  if((long)(length + codec->buffer_position) <= codec->buffer_size)
    {
    memcpy(data, codec->buffer + codec->buffer_position, length);
    codec->buffer_position += length;
    }
  }

static void write_function(png_structp png_ptr, png_bytep data, png_uint_32 length)
  {
  quicktime_png_codec_t *codec = png_get_io_ptr(png_ptr);

  if((long)(length + codec->buffer_size) > codec->buffer_alloc)
    {
    codec->buffer_alloc += length;
    codec->buffer = realloc(codec->buffer, codec->buffer_alloc);
    }
  memcpy(codec->buffer + codec->buffer_size, data, length);
  codec->buffer_size += length;
  }

static void flush_function(png_structp png_ptr)
  {
  }

static int decode(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  int result = 0;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info = 0;	
  quicktime_png_codec_t *codec = vtrack->codec->priv;

  if(!row_pointers)
    {
    vtrack->stream_cmodel = source_cmodel(file, track);

    /* Set compression info */
    vtrack->ci.id = LQT_COMPRESSION_PNG;
    
    return 0;
    }

  codec->buffer_size =
    lqt_read_video_frame(file, &codec->buffer, &codec->buffer_alloc,
                         vtrack->current_position, NULL, track);
  
  codec->buffer_position = 0;
  if(codec->buffer_size > 0)
    {
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    info_ptr = png_create_info_struct(png_ptr);
    png_set_read_fn(png_ptr, codec, (png_rw_ptr)read_function);
    png_read_info(png_ptr, info_ptr);

    /* read the image */
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    }
  
  return result;
  }

static int encode(quicktime_t *file, unsigned char **row_pointers, int track)
  {
  int result = 0;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  quicktime_png_codec_t *codec = vtrack->codec->priv;
  int height = trak->tkhd.track_height;
  int width = trak->tkhd.track_width;
  png_structp png_ptr;
  png_infop info_ptr;

  if(!row_pointers)
    {
    if(vtrack->ci.id)
      vtrack->stream_cmodel = vtrack->ci.colormodel;
    return 0;
    }

  if(!codec->initialized)
    {
    /* Set depth to 32 */
    if(vtrack->stream_cmodel == BC_RGBA8888)
      vtrack->track->mdia.minf.stbl.stsd.table[0].depth = 32;
    else
      vtrack->track->mdia.minf.stbl.stsd.table[0].depth = 24;
    }
  
  codec->buffer_size = 0;
  codec->buffer_position = 0;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  info_ptr = png_create_info_struct(png_ptr);
  png_set_write_fn(png_ptr,
                   codec, 
                   (png_rw_ptr)write_function,
                   (png_flush_ptr)flush_function);
  png_set_compression_level(png_ptr, codec->compression_level);
  png_set_IHDR(png_ptr, 
               info_ptr, 
               width, height,
               8, 
               vtrack->stream_cmodel == BC_RGB888 ? 
               PNG_COLOR_TYPE_RGB : 
               PNG_COLOR_TYPE_RGB_ALPHA, 
               PNG_INTERLACE_NONE, 
               PNG_COMPRESSION_TYPE_DEFAULT, 
               PNG_FILTER_TYPE_DEFAULT);

#if 0
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
#else
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
#endif
  png_destroy_write_struct(&png_ptr, &info_ptr);

  lqt_write_frame_header(file, track,
                         vtrack->current_position,
                         -1, 0);
        
  result = !quicktime_write_data(file, 
                                 codec->buffer, 
                                 codec->buffer_size);

  lqt_write_frame_footer(file, track);
  return result;
  }


static int set_parameter(quicktime_t *file, 
                         int track, 
                         const char *key, 
                         const void *value)
  {
  quicktime_png_codec_t *codec = file->vtracks[track].codec->priv;
  
  if(!strcasecmp(key, "png_compression_level"))
    codec->compression_level = *(int*)value;
  return 0;
  }

static int writes_compressed(lqt_file_type_t type, const lqt_compression_info_t * ci)
  {
  if(!(type & (LQT_FILE_QT | LQT_FILE_QT_OLD)))
    return 0;

  if((ci->colormodel != BC_RGB888) && (ci->colormodel != BC_RGBA8888))
    return 0;
  return 1;
  }

void quicktime_init_codec_png(quicktime_codec_t * codec_base,
                              quicktime_audio_map_t *atrack,
                              quicktime_video_map_t *vtrack)
  {
  quicktime_png_codec_t *codec;

  codec = calloc(1, sizeof(*codec));
  
  /* Init public items */
  codec_base->priv = codec;
  codec_base->delete_codec = delete_codec;
  codec_base->decode_video = decode;
  codec_base->encode_video = encode;
  codec_base->set_parameter = set_parameter;
  codec_base->writes_compressed = writes_compressed;
  
  /* Init private items */
  codec->compression_level = 9;

  if(!vtrack)
    return;
  vtrack->stream_cmodel = BC_RGB888;
  }

void quicktime_init_codec_pngalpha(quicktime_codec_t * codec_base,
                                   quicktime_audio_map_t *atrack,
                                   quicktime_video_map_t *vtrack)
  {
  /* Proceed as normal */
  quicktime_init_codec_png(codec_base, atrack, vtrack);
  
  if(!vtrack)
    return;
  vtrack->stream_cmodel = BC_RGBA8888;
  }

/*******************************************************************************
 lqt_color.c

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
#include "quicktime/colormodels.h"
#include <stdlib.h>
#include <string.h>

#define LOG_DOMAIN "color"

typedef struct
  {
  char * name;
  int colormodel;
  } lqt_colormodel_tab;

static lqt_colormodel_tab colormodel_table[] =
  {
    { "Compressed",                BC_COMPRESSED },
    { "16 bpp RGB 565",            BC_RGB565 },
    { "16 bpp BGR 565",            BC_BGR565 },
    { "24 bpp BGR",                BC_BGR888 },
    { "32 bpp BGR",                BC_BGR8888 },
    { "24 bpp RGB",                BC_RGB888 },
    { "32 bpp RGBA",               BC_RGBA8888 },
    { "48 bpp RGB",                BC_RGB161616  }, 
    { "64 bpp RGBA",               BC_RGBA16161616  },
    { "32 bpp YUVA",               BC_YUVA8888  },   
    { "YUV 4:2:2 packed (YUY2)",   BC_YUV422  },
    { "YUV 4:2:0 planar",          BC_YUV420P },
    { "YUV 4:2:2 planar",          BC_YUV422P },
    { "YUV 4:4:4 planar",          BC_YUV444P },
    { "YUV 4:2:2 planar (16 bit)", BC_YUV422P16 },
    { "YUV 4:4:4 planar (16 bit)", BC_YUV444P16 },
    { "YUV 4:2:2 planar (10 bit)", BC_YUV422P10 },
    { "YUV 4:2:2 planar (10 bit, jpeg)", BC_YUVJ422P10 },
    { "YUV 4:2:0 planar (jpeg)",   BC_YUVJ420P },
    { "YUV 4:2:2 planar (jpeg)",   BC_YUVJ422P },
    { "YUV 4:4:4 planar (jpeg)",   BC_YUVJ444P },
    { "YUV 4:1:1 planar",          BC_YUV411P },
    { "Undefined",                 LQT_COLORMODEL_NONE }
  };

/* Some functions to find out, how cheap a colorspace conversion can be */

int lqt_colormodel_is_yuv(int colormodel)
  {
  switch(colormodel)
    {
    case BC_YUVA8888:
    case BC_YUV422:
    case BC_YUV420P:
    case BC_YUV422P:
    case BC_YUV444P:
    case BC_YUV422P16:
    case BC_YUV444P16:
    case BC_YUVJ420P:
    case BC_YUVJ422P:
    case BC_YUVJ444P:
    case BC_YUV411P:
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      return 1;
    default:
      return 0;
    }
  }

int lqt_colormodel_is_rgb(int colormodel)
  {
  switch(colormodel)
    {
    case BC_RGB565:
    case BC_BGR565:
    case BC_BGR888:
    case BC_BGR8888:
    case BC_RGB888:
    case BC_RGBA8888:
    case BC_RGB161616:
    case BC_RGBA16161616:
      return 1;
    default:
      return 0;
    }
  }

int lqt_colormodel_has_alpha(int colormodel)
  {
  switch(colormodel)
    {
    case BC_RGBA8888:
    case BC_RGBA16161616:
    case BC_YUVA8888:   
      return 1;
    default:
      return 0;
    }
  }

int lqt_colormodel_is_planar(int colormodel)
  {
  switch(colormodel)
    {
    case BC_YUV420P:
    case BC_YUV422P:
    case BC_YUV444P:
    case BC_YUV422P16:
    case BC_YUV444P16:
    case BC_YUVJ422P:
    case BC_YUVJ444P:
    case BC_YUVJ420P:
    case BC_YUV411P:
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      return 1;
    default:
      return 0;
    }
  
  }

/* Get chroma subsampling factors */

void lqt_colormodel_get_chroma_sub(int colormodel, int * sub_h, int * sub_v)
  {
  switch(colormodel)
    {
    case BC_YUV420P:
    case BC_YUVJ420P:
      *sub_h = 2;
      *sub_v = 2;
      break;
    case BC_YUV422:
    case BC_YUV422P:
    case BC_YUVJ422P:
    case BC_YUV422P16:
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      *sub_h = 2;
      *sub_v = 1;
      break;
    case BC_YUV411P:
      *sub_h = 4;
      *sub_v = 1;
      break;
    default:
      *sub_h = 1;
      *sub_v = 1;
      break;
    }

  }


/*
 *   Return the bits of a colormodel. This is used only internally
 *   and returns the sum of the bits of all components. Downsampling isn't
 *   taken into account here, YUV 420 has 24 bits. We need to test this,
 *   because e.g. RGBA8888 -> RGBA16161616 cost extra.
 */

static int colormodel_get_bits(int colormodel)
  {
  switch(colormodel)
    {
    case BC_RGB565:
    case BC_BGR565:
      return 16;
    case BC_BGR888:
    case BC_BGR8888:
    case BC_RGB888:
    case BC_YUV422:
    case BC_YUV420P:
    case BC_YUV422P:
    case BC_YUV444P:
    case BC_YUVJ420P:
    case BC_YUVJ422P:
    case BC_YUVJ444P:
    case BC_YUV411P:
      return 24;
    case BC_RGBA8888:
    case BC_YUVA8888:   
      return 32;
    case BC_RGB161616: 
    case BC_YUV422P16:
    case BC_YUV444P16:
      return 48;
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      return 30;
    case BC_RGBA16161616:
      return 64;
    default:
      lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
              "Unknown colormodel (%d)\n",colormodel);
      return 0;
    }
  }

/*
 *  Get the "Price" of a colormodel conversion
 */

static int get_conversion_price(int in_colormodel, int out_colormodel)
  {
  int input_is_rgb  = lqt_colormodel_is_rgb(in_colormodel);
  int output_is_rgb = lqt_colormodel_is_rgb(out_colormodel);
  
  int input_is_yuv  = lqt_colormodel_is_yuv(in_colormodel);
  int output_is_yuv = lqt_colormodel_is_yuv(out_colormodel);

  int input_has_alpha  = lqt_colormodel_has_alpha(in_colormodel);
  int output_has_alpha = lqt_colormodel_has_alpha(out_colormodel);

 
  /* Zero conversions are for free :-) */
  
  if(in_colormodel == out_colormodel)
    return 0;

  /*
   *  Don't know what to do here. It can happen for very few
   *  colormodels which aren't supported by any codecs.
   */
  
  if(!input_is_rgb && !input_is_yuv)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
            "Input colorspace is neither RGB nor YUV, can't predict conversion price");
    return 7;
    }
  
  if(!output_is_rgb && !output_is_yuv)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
            "Output colorspace is neither RGB nor YUV, can't predict conversion price");
    return 7;
    }

  /*
   *  Adding or removing the alpha channel means losing information or
   *  adding unneccesary information -> too bad
   */

  if(input_has_alpha != output_has_alpha)
    return 6;
  
  /*
   *  YUV <-> RGB conversion costs 4-5
   */
  
  if((input_is_yuv && output_is_rgb) ||
     (input_is_rgb && output_is_yuv))
    {
    /* Added check to make sure that staying on the same bit depth
       is considered as "better" when doing a colorspace conversion. */
    if(colormodel_get_bits(in_colormodel) !=
      colormodel_get_bits(out_colormodel))
      /* With bit conversion: 5   */
      return 5;
    else 
      /* No bit conversion is better: 4   */
      return 4;
    }
  
  /*
   *  Alpha blending is a bit more simple
   */

  if((input_is_yuv && output_is_rgb) ||
     (input_is_rgb && output_is_yuv))
    return 3;
  
  /* Bit with conversion costs 2   */
  
  if(colormodel_get_bits(in_colormodel) !=
     colormodel_get_bits(out_colormodel))
    return 2;

  /* Reordering of components is cheapest */
  
  return 1;
  }

int lqt_get_decoder_colormodel(quicktime_t * file, int track)
  {
  return file->vtracks[track].stream_cmodel;
  }

int
lqt_get_best_target_colormodel(int source, int const* target_options)
  {
  int conversion_price = 0, best_conversion_price = 10;
  int ret = LQT_COLORMODEL_NONE;
  int i;

  if (!target_options) {
    /* Undocumented behavior, but from inside libquicktime we rely on it. */
    return ret;
  }

  for(i = 0; target_options[i] != LQT_COLORMODEL_NONE; ++i)
    {
    if (target_options[i] == source)
      return source;

    if(lqt_colormodel_has_conversion(source, target_options[i]))
      {
      conversion_price = get_conversion_price(source, target_options[i]);

      if(conversion_price < best_conversion_price)
        {
        best_conversion_price = conversion_price;
        ret = target_options[i];
        }
      }
    }

  return ret;
  }

int
lqt_get_best_source_colormodel(int const* source_options, int target)
  {
  int conversion_price = 0, best_conversion_price = 10;
  int ret = LQT_COLORMODEL_NONE;
  int i;

  if (!source_options) {
    /* Undocumented behavior, but from inside libquicktime we rely on it. */
    return ret;
  }

  for(i = 0; source_options[i] != LQT_COLORMODEL_NONE; ++i)
    {
    if(source_options[i] == target)
      return target;

    if(lqt_colormodel_has_conversion(source_options[i], target))
      {
      conversion_price = get_conversion_price(source_options[i], target);
        
      if(conversion_price < best_conversion_price)
        {
        best_conversion_price = conversion_price;
        ret = source_options[i];
        }
      }
    }
  
  return ret;
  }

int lqt_get_best_colormodel(quicktime_t * file, int track,
                            int * supported)
  {
  int ret;
  if((track >= file->total_vtracks) || (track < 0))
    return LQT_COLORMODEL_NONE;
  if(file->wr)
    ret = lqt_get_best_source_colormodel(supported, file->vtracks[track].stream_cmodel);
  else
    ret = lqt_get_best_target_colormodel(file->vtracks[track].stream_cmodel, supported);

  if(ret == LQT_COLORMODEL_NONE)
    {
    /* Backwards compatibility. */
    ret = BC_RGB888;
    }

  return ret;
  }

static const int num_colormodels =
  sizeof(colormodel_table)/sizeof(colormodel_table[0])-1;

const char * lqt_colormodel_to_string(int colormodel)
  {
  int i = 0;
  for(; i < num_colormodels; i++)
    if(colormodel_table[i].colormodel == colormodel)
      break;
  return colormodel_table[i].name;
  }

int lqt_string_to_colormodel(const char * str)
  {
  int i = 0;
  for(; i < num_colormodels; i++)
    if(!strcmp(colormodel_table[i].name, str))
      break;
  return colormodel_table[i].colormodel;
  }

int lqt_num_colormodels()
  {
  return num_colormodels;
  }

const char * lqt_get_colormodel_string(int index)
  {
  return colormodel_table[index].name;
  }

int lqt_get_colormodel(int index)
  {
  return colormodel_table[index].colormodel;
  }

static int get_bytes_per_element(int colormodel)
  {
  switch(colormodel)
    {
    case BC_RGB565:
    case BC_BGR565:
    case BC_YUV422:
    case BC_YUV422P16:
    case BC_YUV444P16:
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      return 2;
      break;
    case BC_BGR888:
    case BC_RGB888:
      return 3;
      break;
    case BC_BGR8888:
    case BC_RGBA8888:
    case BC_YUVA8888:
      return 4;
      break;
    case BC_RGB161616:
      return 6;
      break;
    case BC_RGBA16161616:
      return 8;
      break;
    default:
      return 1;
    }
  
  }

static int get_bytes_per_line(int colormodel, int width)
  {
  return get_bytes_per_element(colormodel) * width;
  }

/* Allocate and free row_pointers for use with libquicktime */

uint8_t ** lqt_rows_alloc(int width, int height, int colormodel, int * rowspan, int * rowspan_uv)
  {
  int bytes_per_line = 0;
  int i;
  int y_size = 0, uv_size = 0;
  uint8_t ** video_buffer;
  int sub_h = 0, sub_v = 0;

  /* Allocate frame buffer */
  bytes_per_line = get_bytes_per_line(colormodel, width);
    
  if(cmodel_is_planar(colormodel))
    {
    lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);

    if(*rowspan <= 0)
      *rowspan = bytes_per_line;

    if(*rowspan_uv <= 0)
      *rowspan_uv = (*rowspan + sub_h - 1) / sub_h;

    y_size = *rowspan * height;
    uv_size = (*rowspan_uv * (height + sub_v - 1))/sub_v;

    video_buffer    = malloc(3 * sizeof(unsigned char*));
    video_buffer[0] = malloc(y_size + 2 * uv_size);
    video_buffer[1] = &video_buffer[0][y_size];
    video_buffer[2] = &video_buffer[0][y_size+uv_size];
    }
  else
    {
    video_buffer    = malloc(height * sizeof(unsigned char*));
        
    if(*rowspan <= 0)
      *rowspan = bytes_per_line;
        
    video_buffer[0] = malloc(height * bytes_per_line);
    for(i = 1; i < height; i++)
      video_buffer[i] = &video_buffer[0][i*bytes_per_line];
    
    }
  return video_buffer;
  }

void lqt_get_default_rowspan(int colormodel, int width, int * rowspan, int * rowspan_uv)
  {
  int bytes_per_line;
  int sub_h = 0, sub_v = 0;
  bytes_per_line = get_bytes_per_line(colormodel, width);
  lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);

  *rowspan = bytes_per_line;

  if(lqt_colormodel_is_planar(colormodel))
    {
    *rowspan_uv = (bytes_per_line + sub_h - 1) / sub_h;
    }
  }

void lqt_rows_free(uint8_t ** rows)
  {
  free(rows[0]);
  free(rows);
  }

void lqt_rows_copy(uint8_t **out_rows, uint8_t **in_rows, int width, int height, int in_rowspan,
                   int in_rowspan_uv, int out_rowspan, int out_rowspan_uv, int colormodel)
  {
  lqt_rows_copy_sub(out_rows, in_rows,
                    width, height, in_rowspan,
                    in_rowspan_uv, out_rowspan,
                    out_rowspan_uv, colormodel, 0, 0, 0, 0);
  }

void lqt_rows_clear(uint8_t **rows,
                    int width, int height, int rowspan, int rowspan_uv, int colormodel)
  {
  int i, j, bits_per_sample, bytes_per_element, bytes_per_line;
  uint8_t * ptr;
  uint16_t * ptr_16;
  uint8_t * ptr_start;
  int sub_h, sub_v;
  uint16_t y_val, uv_val;

  bits_per_sample = colormodel_get_bits(colormodel) / (lqt_colormodel_has_alpha(colormodel) ? 4 : 3);
  bytes_per_element = get_bytes_per_element(colormodel);
  bytes_per_line = bytes_per_element * width;
  y_val = 1 << (bits_per_sample - 4);  // This one is good for non-jpeg colormodels.
  uv_val = 1 << (bits_per_sample - 1); // This one is good for both jpeg and non-jpeg colormodels.

  switch(colormodel)
    {
    case BC_RGB565:
    case BC_BGR565:
    case BC_BGR888:
    case BC_BGR8888:
    case BC_RGB888:
    case BC_RGB161616:
    case BC_RGBA8888:
    case BC_RGBA16161616:
      if(rows[1])
        {
        for(i = 0; i < height; i++)
          memset(rows[i], 0, bytes_per_line);
        }
      else
        {
        ptr = rows[0];
        for(i = 0; i < height; i++)
          {
          memset(ptr, 0, bytes_per_line);
          ptr += rowspan;
          }
        }
      break;
    case BC_YUVA8888:
      if(rows[1])
        {
        for(i = 0; i < height; i++)
          {
          ptr = rows[i];
          for(j = 0; j < width; j++)
            {
            ptr[0] = 0x10;
            ptr[1] = 0x80;
            ptr[2] = 0x80;
            ptr[3] = 0x00;
            ptr += 4;
            }
          }
        }
      else
        {
        ptr_start = rows[0];
        for(i = 0; i < height; i++)
          {
          ptr = ptr_start;

          for(j = 0; j < width; j++)
            {
            ptr[0] = 0x10;
            ptr[1] = 0x80;
            ptr[2] = 0x80;
            ptr[3] = 0x00;
            ptr += 4;
            }
          
          ptr_start += rowspan;
          }
        }
      break;
    case BC_YUV422:
      if(rows[1])
        {
        for(i = 0; i < height; i++)
          {
          ptr = rows[i];
          for(j = 0; j < width; j++)
            {
            ptr[0] = 0x10;
            ptr[1] = 0x80;
            ptr += 2;
            }
          }
        }
      else
        {
        ptr_start = rows[0];
        for(i = 0; i < height; i++)
          {
          ptr = ptr_start;
          for(j = 0; j < width; j++)
            {
            ptr[0] = 0x10;
            ptr[1] = 0x80;
            ptr += 2;
            }
          ptr_start += rowspan;
          }
        }
      break;
    case BC_YUV420P:
    case BC_YUV422P:
    case BC_YUV444P:
    case BC_YUV411P:
      lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);
      /* Y */
      ptr = rows[0];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x10, width);
        ptr += rowspan;
        }

      width /= sub_h;
      height /= sub_v;
      
      /* U */
      ptr = rows[1];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x80, width);
        ptr += rowspan_uv;
        }

      /* V */
      ptr = rows[2];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x80, width);
        ptr += rowspan_uv;
        }
      
      break;
    case BC_YUVJ420P:
    case BC_YUVJ422P:
    case BC_YUVJ444P:
      lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);
      /* Y */
      ptr = rows[0];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x00, width);
        ptr += rowspan;
        }

      width /= sub_h;
      height /= sub_v;
      
      /* U */
      ptr = rows[1];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x80, width);
        ptr += rowspan_uv;
        }

      /* V */
      ptr = rows[2];
      for(i = 0; i < height; i++)
        {
        memset(ptr, 0x80, width);
        ptr += rowspan_uv;
        }
      break;
    case BC_YUVJ422P10:
      y_val = 0; // For jpeg colormodels.
    case BC_YUV422P16:
    case BC_YUV444P16:
    case BC_YUV422P10:
      lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);

      /* Y */
      ptr_start = rows[0];
      for(i = 0; i < height; i++)
        {
        ptr_16 = (uint16_t*)ptr_start;
        for(j = 0; j < width; j++)
          {
          *ptr_16 = y_val;
          ptr_16++;
          }
        ptr_start += rowspan;
        }

      width /= sub_h;
      height /= sub_v; // Unneccesary actually

      /* U */
      ptr_start = rows[1];
      for(i = 0; i < height; i++)
        {
        ptr_16 = (uint16_t*)ptr_start;
        for(j = 0; j < width; j++)
          {
          *ptr_16 = uv_val;
          ptr_16++;
          }
        ptr_start += rowspan;
        }
      
      /* V */
      ptr_start = rows[2];
      for(i = 0; i < height; i++)
        {
        ptr_16 = (uint16_t*)ptr_start;
        for(j = 0; j < width; j++)
          {
          *ptr_16 = uv_val;
          ptr_16++;
          }
        ptr_start += rowspan;
        }
      

      break;
    }
  }

void lqt_rows_copy_sub(uint8_t **out_rows, uint8_t **in_rows,
                       int width, int height, int in_rowspan,
                       int in_rowspan_uv, int out_rowspan,
                       int out_rowspan_uv, int colormodel, int src_x, int src_y, int dst_x, int dst_y)
  {
  uint8_t * src_ptr, *dst_ptr;
  int i;
  int sub_h = 0, sub_v = 0;
  
  int bytes_per_line, bytes_per_element;
  
  bytes_per_line = get_bytes_per_line(colormodel, width);
  bytes_per_element = get_bytes_per_element(colormodel);
  
  if(lqt_colormodel_is_planar(colormodel))
    {
    lqt_colormodel_get_chroma_sub(colormodel, &sub_h, &sub_v);

    /* Align */
    src_x /= sub_h;
    src_x *= sub_h;

    src_y /= sub_v;
    src_y *= sub_v;
    
    /* Luma plane */
    
    src_ptr = in_rows[0] + src_x * bytes_per_element + src_y * in_rowspan;
    dst_ptr = out_rows[0] + src_x * bytes_per_element + dst_y * out_rowspan;
    for(i = 0; i < height; i++)
      {
      memcpy(dst_ptr, src_ptr, bytes_per_line);
      src_ptr += in_rowspan;
      dst_ptr += out_rowspan;
      }
    /* Chroma planes */

    src_ptr = in_rows[1] + src_x/sub_h * bytes_per_element + src_y * in_rowspan_uv;
    dst_ptr = out_rows[1] + dst_x/sub_h * bytes_per_element + dst_y * out_rowspan_uv;
    for(i = 0; i < (height + sub_v - 1)/sub_v; i++)
      {
      memcpy(dst_ptr, src_ptr, (bytes_per_line + sub_h - 1)/sub_h);
      src_ptr += in_rowspan_uv;
      dst_ptr += out_rowspan_uv;
      }
    
    src_ptr = in_rows[2] + src_x/sub_h * bytes_per_element + src_y * in_rowspan_uv;
    dst_ptr = out_rows[2] + dst_x/sub_h * bytes_per_element + dst_y * out_rowspan_uv;
    for(i = 0; i < (height + sub_v - 1)/sub_v; i++)
      {
      memcpy(dst_ptr, src_ptr, (bytes_per_line + sub_h - 1)/sub_h);
      src_ptr += in_rowspan_uv;
      dst_ptr += out_rowspan_uv;
      }
    
    }
  else /* Packed */
    {
    /* This is nasty: We don't know, how the frames are allocated.
       We test rows[1] to check this and handle all 4 combinations
       separately */
    
    if(in_rows[1] && out_rows[1])
      {
      for(i = 0; i < height; i++)
        memcpy(out_rows[i + dst_y] + dst_x * bytes_per_element,
               in_rows[i + src_y] + src_x * bytes_per_element, bytes_per_line);
      }
    else if(in_rows[1])
      {
      dst_ptr = out_rows[0] + dst_y * bytes_per_line + dst_x * bytes_per_element;
      
      for(i = 0; i < height; i++)
        {
        memcpy(dst_ptr, in_rows[i + src_y] + src_x * bytes_per_element, bytes_per_line);
        dst_ptr += out_rowspan;
        }
      }
    else if(out_rows[1])
      {
      src_ptr = in_rows[0] + src_y * bytes_per_line + src_x * bytes_per_element;
      
      for(i = 0; i < height; i++)
        {
        memcpy(out_rows[i + dst_y] + dst_x * bytes_per_element, src_ptr, bytes_per_line);
        src_ptr += in_rowspan;
        }
      }
    else
      {
      src_ptr = in_rows[0] + src_y * bytes_per_line + src_x * bytes_per_element;
      dst_ptr = out_rows[0] + dst_y * bytes_per_line + dst_x * bytes_per_element;
      
      for(i = 0; i < height; i++)
        {
        memcpy(dst_ptr, src_ptr, bytes_per_line);
        src_ptr += in_rowspan;
        dst_ptr += out_rowspan;
        }
      }
    }
  }


// for i in BC_RGB565 BC_BGR565 BC_BGR888 BC_BGR8888 BC_RGB888 BC_RGBA8888 BC_RGB161616 BC_RGBA16161616 BC_YUVA8888 BC_YUV422 BC_YUV420P BC_YUV422P BC_YUV444P BC_YUV411P BC_YUVJ420P BC_YUVJ422P BC_YUVJ444P BC_YUV422P16 BC_YUV444P16; do for j in BC_RGB565 BC_BGR565 BC_BGR888 BC_BGR8888 BC_RGB888 BC_RGBA8888 BC_RGB161616 BC_RGBA16161616 BC_YUVA8888 BC_YUV422 BC_YUV420P BC_YUV422P BC_YUV444P BC_YUV411P BC_YUVJ420P BC_YUVJ422P BC_YUVJ444P BC_YUV422P16 BC_YUV444P16; do echo $i"_to_"$j.png; gthumb $i"_to_"$j.png; done; done

int lqt_colormodel_has_conversion(int in_cmodel, int out_cmodel)
  {
  if(in_cmodel == out_cmodel)
    return 1;
  
  switch(in_cmodel)
    {
    case BC_RGB565:
      switch(out_cmodel)
        {
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_BGR565:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_BGR888:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_BGR8888:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_RGB888:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 1; break;
        case BC_YUVJ420P:     return 1; break;
        case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 1; break;
        case BC_YUV422P16:    return 1; break;
        case BC_YUV444P16:    return 1; break;
        case BC_YUV422P10:    return 1; break;
        case BC_YUVJ422P10:   return 1; break;
        }
      break;
    case BC_RGBA8888:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 1; break;
        case BC_YUVJ420P:     return 1; break;
        case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 1; break;
        case BC_YUV422P16:    return 1; break;
        case BC_YUV444P16:    return 1; break;
        }
      break;
    case BC_RGB161616:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 1; break;
        case BC_YUV444P16:    return 1; break;
        case BC_YUV422P10:    return 1; break;
        case BC_YUVJ422P10:   return 1; break;
        }
      break;
    case BC_RGBA16161616:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUVA8888:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV422:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV420P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV422P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV444P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 1; break;
        case BC_BGR565:       return 1; break;
        case BC_BGR888:       return 1; break;
        case BC_BGR8888:      return 1; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 1; break;
        case BC_YUVA8888:     return 1; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV411P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUVJ420P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUVJ422P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 1; break;
        case BC_YUV420P:      return 1; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUVJ444P:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 0; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV422P16:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 0; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV444P16:    return 0; break;
        }
      break;
    case BC_YUV444P16:
      switch(out_cmodel)
        {
        case BC_RGB565:       return 0; break;
        case BC_BGR565:       return 0; break;
        case BC_BGR888:       return 0; break;
        case BC_BGR8888:      return 0; break;
        case BC_RGB888:       return 1; break;
        case BC_RGBA8888:     return 1; break;
        case BC_RGB161616:    return 1; break;
        case BC_RGBA16161616: return 0; break;
        case BC_YUVA8888:     return 0; break;
        case BC_YUV422:       return 0; break;
        case BC_YUV420P:      return 0; break;
        case BC_YUV422P:      return 0; break;
        case BC_YUV444P:      return 1; break;
        case BC_YUV411P:      return 0; break;
        case BC_YUVJ420P:     return 0; break;
        case BC_YUVJ422P:     return 0; break;
        case BC_YUVJ444P:     return 0; break;
        case BC_YUV422P16:    return 0; break;
        }
      break;
    case BC_YUV422P10:
    case BC_YUVJ422P10:
      switch(out_cmodel)
        {
        case BC_RGB888:       return 1; break;
        case BC_RGB161616:    return 1; break;
        }
      break;
    }
  return 0;
  }

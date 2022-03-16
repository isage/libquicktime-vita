/*******************************************************************************
 lqt_png.c

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
#include <quicktime/lqt_codecapi.h>
#include <quicktime/colormodels.h>

#include "qtpng.h"


static char * fourccs_png[]  = { QUICKTIME_PNG, (char*)0 };

static lqt_parameter_info_static_t encode_parameters_png[] =
  {
     { 
       .name =        "png_compression_level",
       .real_name =   TRS("Compression Level"),
       .type =        LQT_PARAMETER_INT,
       .val_default = { .val_int = 9 },
       .val_min =     { .val_int = 0 },
       .val_max =     { .val_int = 9 },
     },
     { /* End of parameters */ }
  };

/* RGBA png is supported as a special codec, which supports encoding only.
   The normal png codec can decode both RGB and RGBA */

static lqt_codec_info_static_t codec_info_pngalpha =
  {
  .name =                "pngalpha",
  .long_name =           TRS("PNG (with alpha)"),
  .description =         TRS("Lossless video codec (RGBA mode)"),
  .fourccs =             fourccs_png,
  .type =                LQT_CODEC_VIDEO,
  .direction =           LQT_DIRECTION_ENCODE,
  .encoding_parameters = encode_parameters_png,
  .decoding_parameters = (lqt_parameter_info_static_t*)0,
  .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_CODEC_OBSOLETE,
  .compression_id      = LQT_COMPRESSION_PNG,
  };

static lqt_codec_info_static_t codec_info_png =
  {
  .name =                "png",
  .long_name =           TRS("PNG"),
  .description =         TRS("Lossless video codec (RGB mode)"),
  .fourccs =             fourccs_png,
  .type =                LQT_CODEC_VIDEO,
  .direction =           LQT_DIRECTION_BOTH,
  .encoding_parameters = encode_parameters_png,
  .decoding_parameters = (lqt_parameter_info_static_t*)0,
  .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  .encoding_colormodels = (int[]) { BC_RGB888,
                                    BC_RGBA8888,
                                    LQT_COLORMODEL_NONE},
  .compression_id      = LQT_COMPRESSION_PNG,
  };

/* These are called from the plugin loader */

LQT_EXTERN int get_num_codecs() { return 2; }

LQT_EXTERN lqt_codec_info_static_t * get_codec_info(int index)
  {
  switch(index)
    {
    case 0:
      return &codec_info_png;
    case 1:
      return &codec_info_pngalpha;
    }
  return (lqt_codec_info_static_t*)0;
  }


/*
 *   Return the actual codec constructor
 */

LQT_EXTERN lqt_init_codec_func_t get_codec(int index)
  {
  switch(index)
    {
    case 0:
      return quicktime_init_codec_png;
      break;
    case 1:
      return quicktime_init_codec_pngalpha;
      break;
    }
  return (lqt_init_codec_func_t)0;
  }

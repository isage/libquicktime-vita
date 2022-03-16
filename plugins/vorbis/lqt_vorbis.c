/*******************************************************************************
 lqt_vorbis.c

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
#include "qtvorbis.h"
#include <quicktime/lqt_codecapi.h>

static char * fourccs_vorbis[]     = { QUICKTIME_VORBIS, "OggV", (char*)0 };
static char * fourccs_vorbis_qt[]  = { "OggV", QUICKTIME_VORBIS, (char*)0 };

static lqt_parameter_info_static_t encode_parameters_vorbis[] =
  {
     { 
       .name =        "vorbis_bitrate",
       .real_name =   TRS("Nominal Bitrate"),
       .type =        LQT_PARAMETER_INT,
       .val_default = { .val_int = 128000 },
     },
     { 
       .name =        "vorbis_vbr",
       .real_name =   TRS("Use variable bitrate"),
       .type =        LQT_PARAMETER_INT,
       .val_default = { .val_int = 1 },
       .val_min =     { .val_int = 0 },
       .val_max =     { .val_int = 1 },
     },
     {          
       .name =        "vorbis_max_bitrate",
       .real_name =   TRS("Maximum Bitrate (-1 = no limit)"),
       .type =        LQT_PARAMETER_INT,
       .val_default = { .val_int = -1 },
       .val_min =     { .val_int = 0 },
       .val_max =     { .val_int = 0 },
     },
     { 
       .name =        "vorbis_min_bitrate",
       .real_name =   TRS("Minimum Bitrate (-1 = no limit)"),
       .type =        LQT_PARAMETER_INT,
       .val_default = { .val_int = -1 },
       .val_min =     { .val_int = 0 },
       .val_max =     { .val_int = 0 }
     },
     { /* End of paramaters */ }
  };

static lqt_codec_info_static_t codec_info_vorbis =
  {
    .name =                "vorbis",
    .long_name =           TRS("Ogg Vorbis (qt4l compatible)"),
    .description =         TRS("Patent free audio codec (see http://www.vorbis.com)"),
    .fourccs =             fourccs_vorbis,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = encode_parameters_vorbis,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_vorbis_qt =
  {
    .name =                "vorbis_qt",
    .long_name =           TRS("Ogg Vorbis (qtcomponents compatible)"),
    .description =         TRS("Patent free audio codec (see http://www.vorbis.com)"),
    .fourccs =             fourccs_vorbis_qt,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = encode_parameters_vorbis,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT,
  };


/* These are called from the plugin loader */

LQT_EXTERN int get_num_codecs() { return 2; }

LQT_EXTERN lqt_codec_info_static_t * get_codec_info(int index)
  {
  switch(index)
    {
    case 0:
      return &codec_info_vorbis;
      break;
    case 1:
      return &codec_info_vorbis_qt;
      break;
    }  
  return (lqt_codec_info_static_t*)0;
  }
     
/*
 *   Return the actual codec constructor
 */

LQT_EXTERN lqt_init_codec_func_t get_codec(int index)
  {
  return quicktime_init_codec_vorbis;
  }


/*******************************************************************************
 lqt_audiocodec.c

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
#include "audiocodec.h"

/* Encoding parameters */

#define PARAM_ENDIAN \
  { \
    .name = "pcm_little_endian", \
    .real_name = TRS("Little endian"),          \
    .type = LQT_PARAMETER_INT, \
    .val_min = { .val_int = 0 }, \
    .val_max = { .val_int = 1 }, \
    .val_default = { .val_int = 0 } \
  }

#define PARAM_FORMAT \
  { \
    .name = "pcm_format", \
    .real_name = TRS("Format"),       \
    .type = LQT_PARAMETER_STRINGLIST, \
    .val_default = { .val_string = "Integer (16 bit)" }, \
    .stringlist_options = (char*[]){ TRS("Integer (16 bit)"),   \
                                     TRS("Integer (24 bit)"),   \
                                     TRS("Integer (32 bit)"),   \
                                     TRS("Float (32 bit)"),     \
                                     TRS("Float (64 bit)"),     \
                                     (char*)0 } \
  }

/* Let's define prototypes here */



static char * fourccs_ima4[]  = { QUICKTIME_IMA4, (char*)0 };
static char * fourccs_raw[]   = { QUICKTIME_RAW,  (char*)0 };
static char * fourccs_twos[]  = { QUICKTIME_TWOS, (char*)0 };
static char * fourccs_ulaw[]  = { QUICKTIME_ULAW, (char*)0 };
static char * fourccs_sowt[]  = { "sowt",         (char*)0 };

static char * fourccs_alaw[]  = { "alaw", (char*)0 };

static char * fourccs_in24[]  = { "in24", (char*)0 };
static char * fourccs_in32[]  = { "in32", (char*)0 };

static char * fourccs_fl32[]  = { "fl32", (char*)0 };
static char * fourccs_fl64[]  = { "fl64", (char*)0 };

static char * fourccs_lpcm[]  = { "lpcm", (char*)0 };

static lqt_parameter_info_static_t enda_parameters[] =
  {
    PARAM_ENDIAN,
    { /* End of parameters */ },
  };

static lqt_parameter_info_static_t lpcm_parameters[] =
  {
    PARAM_FORMAT,
    PARAM_ENDIAN,
    { /* End of parameters */ },
  };



static lqt_codec_info_static_t codec_info_ima4 =
  {
    .name =                "ima4",
    .long_name =           TRS("ima4"),
    .description =         TRS("The IMA4 compressor reduces 16 bit audio data to \
1/4 size, with very good quality"),
    .fourccs =             fourccs_ima4,
    .wav_ids =             (int[]){ 0x11, LQT_WAV_ID_NONE },
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_AVI | LQT_FILE_AVI_ODML | LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_raw =
  {
    .name =                "rawaudio",
    .long_name =           TRS("Raw 8 bit audio"),
    .description =         TRS("Don't use this for anything better than telephone \
quality"),
    .fourccs =             fourccs_raw,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_twos =
  {
    .name =                "twos",
    .long_name =           TRS("Twos"),
    .description =         TRS("Twos is the preferred encoding for uncompressed \
audio"),
    .fourccs =             fourccs_twos,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_in24 =
  {
    .name =                "in24",
    .long_name =           TRS("24 bit PCM"),
    .description =         TRS("24 bit PCM"),
    .fourccs =             fourccs_in24,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = enda_parameters,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_in32 =
  {
    .name =                "in32",
    .long_name =           TRS("32 bit PCM"),
    .description =         TRS("32 bit PCM"),
    .fourccs =             fourccs_in32,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = enda_parameters,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

/* Floating point */


static lqt_codec_info_static_t codec_info_fl32 =
  {
    .name =                "fl32",
    .long_name =           TRS("32 bit float"),
    .description =         TRS("32 bit float"),
    .fourccs =             fourccs_fl32,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = enda_parameters,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_fl64 =
  {
    .name =                "fl64",
    .long_name =           TRS("64 bit float"),
    .description =         TRS("64 bit float"),
    .fourccs =             fourccs_fl64,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = enda_parameters,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
  };

static lqt_codec_info_static_t codec_info_ulaw =
  {
    .name =                "ulaw",
    .long_name =           TRS("Ulaw"),
    .description =         TRS("Ulaw"),
    .fourccs =             fourccs_ulaw,
    .wav_ids =             (int[]){ 0x07, LQT_WAV_ID_NONE },
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
    .compression_id = LQT_COMPRESSION_ULAW,
  };

static lqt_codec_info_static_t codec_info_alaw =
  {
    .name =                "alaw",
    .long_name =           "Alaw",
    .description =         "Alaw",
    .fourccs =             fourccs_alaw,
    .wav_ids =             (int[]){ 0x06, LQT_WAV_ID_NONE },
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
    .compression_id = LQT_COMPRESSION_ALAW,
  };


static lqt_codec_info_static_t codec_info_sowt =
  {
    .name =                "sowt",
    .long_name =           TRS("Sowt"),
    .description =         TRS("8/16/24 bit PCM Little endian"),
    .fourccs =             fourccs_sowt,
    .wav_ids =             (int[]){ 0x01, LQT_WAV_ID_NONE },
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = (lqt_parameter_info_static_t*)0,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
  };

static lqt_codec_info_static_t codec_info_lpcm =
  {
    .name =                "lpcm",
    .long_name =           TRS("Linear PCM (QT 7)"),
    .description =         TRS("Linear PCM encoder"),
    .fourccs =             fourccs_lpcm,
    .type =                LQT_CODEC_AUDIO,
    .direction =           LQT_DIRECTION_BOTH,
    .encoding_parameters = lpcm_parameters,
    .decoding_parameters = (lqt_parameter_info_static_t*)0,
    .compatibility_flags = LQT_FILE_QT,
  };


/* These are called from the plugin loader */

int audiocodec_get_num_codecs() { return 11; }

lqt_codec_info_static_t * audiocodec_get_codec_info(int index)
  {
  switch(index)
    {
    case 0: /* ima4 */
      return &codec_info_ima4;
      break;
    case 1: /* raw */
      return &codec_info_raw;
      break;
    case 2: /* Twos */
      return &codec_info_twos;
      break;
    case 3: /* Ulaw */
      return &codec_info_ulaw;
      break;
    case 4: /* Sowt */
      return &codec_info_sowt;
      break;
    case 5: /* alaw */
      return &codec_info_alaw;
      break;
    case 6: /* in24 */
      return &codec_info_in24;
      break;
    case 7: /* in32 */
      return &codec_info_in32;
      break;
    case 8: /* fl32 */
      return &codec_info_fl32;
      break;
    case 9: /* fl64 */
      return &codec_info_fl64;
      break;
    case 10: /* lpcm */
      return &codec_info_lpcm;
      break;
    }
  
  return (lqt_codec_info_static_t*)0;
  }
     
lqt_init_codec_func_t audiocodec_get_codec(int index)
  {
  switch(index)
    {
    case 0: /* ima4 */
      return quicktime_init_codec_ima4;
      break;
    case 1: /* raw */
      return quicktime_init_codec_rawaudio;
      break;
    case 2: /* Twos */
      return quicktime_init_codec_twos;
      break;
    case 3: /* Ulaw */
      return quicktime_init_codec_ulaw;
      break;
    case 4: /* Sowt */
      return quicktime_init_codec_sowt;
      break;
    case 5: /* Alaw */
      return quicktime_init_codec_alaw;
      break;
    case 6: /* in24 */
      return quicktime_init_codec_in24;
      break;
    case 7: /* in32 */
      return quicktime_init_codec_in32;
      break;
    case 8: /* fl32 */
      return quicktime_init_codec_fl32;
      break;
    case 9: /* fl64 */
      return quicktime_init_codec_fl64;
      break;
    case 10: /* lpcm */
      return quicktime_init_codec_lpcm;
      break;
    }
  return (lqt_init_codec_func_t)0;
  }

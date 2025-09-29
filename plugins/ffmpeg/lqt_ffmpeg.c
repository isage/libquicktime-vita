/*******************************************************************************
 lqt_ffmpeg.c

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

/* WARNING: Don't even think about adding support
   for ffmpeg's PCM codecs because they will crash.
   
*/

#include "lqt_private.h"
#include "ffmpeg.h"
#include "params.h"
#include <quicktime/lqt_codecapi.h>
#include <stdio.h>
#include <quicktime/colormodels.h>

#define LOG_DOMAIN "ffmpeg"

#define MAX_FOURCCS 30
#define MAX_WAV_IDS 4

static int ffmpeg_num_audio_codecs = -1;
static int ffmpeg_num_video_codecs = -1;

#if LIBAVCODEC_VERSION_MAJOR >= 53
#define CodecType AVMediaType
#define CODEC_TYPE_UNKNOWN    AVMEDIA_TYPE_UNKNOWN
#define CODEC_TYPE_VIDEO      AVMEDIA_TYPE_VIDEO
#define CODEC_TYPE_AUDIO      AVMEDIA_TYPE_AUDIO
#define CODEC_TYPE_DATA       AVMEDIA_TYPE_DATA
#define CODEC_TYPE_SUBTITLE   AVMEDIA_TYPE_SUBTITLE
#define CODEC_TYPE_ATTACHMENT AVMEDIA_TYPE_ATTACHMENT
#define CODEC_TYPE_NB         AVMEDIA_TYPE_NB
#endif

  
#define ENCODE_PARAM_AUDIO \
  { \
    .name =        "bit_rate_audio", \
    .real_name =   TRS("Bit rate (kbps)"),      \
    .type =        LQT_PARAMETER_INT, \
    .val_default = { .val_int = 128 }, \
  }

#define ENCODE_PARAM_VIDEO_RATECONTROL \
  { \
    .name =      "rate_control", \
    .real_name = TRS("Rate control"),   \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_BITRATE_VIDEO, \
    PARAM_BITRATE_TOLERANCE, \
    PARAM_RC_MIN_RATE, \
    PARAM_RC_MAX_RATE, \
    PARAM_RC_BUFFER_SIZE, \
    PARAM_RC_INITIAL_COMPLEX, \
    PARAM_RC_INITIAL_BUFFER_OCCUPANCY

#define ENCODE_PARAM_VIDEO_QUANTIZER_I \
  { \
    .name =      "quantizer", \
    .real_name = TRS("Quantizer"),      \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_QMIN, \
    PARAM_QMAX, \
    PARAM_MAX_QDIFF, \
    PARAM_FLAG_QSCALE, \
    PARAM_QSCALE, \
    PARAM_QCOMPRESS, \
    PARAM_QBLUR, \
    PARAM_QUANTIZER_NOISE_SHAPING, \
    PARAM_TRELLIS

#define ENCODE_PARAM_VIDEO_QUANTIZER_IP \
  ENCODE_PARAM_VIDEO_QUANTIZER_I, \
  PARAM_I_QUANT_FACTOR, \
  PARAM_I_QUANT_OFFSET

#define ENCODE_PARAM_VIDEO_QUANTIZER_IPB \
  ENCODE_PARAM_VIDEO_QUANTIZER_IP, \
  PARAM_B_QUANT_FACTOR, \
  PARAM_B_QUANT_OFFSET

#define ENCODE_PARAM_VIDEO_FRAMETYPES_IP \
  { \
    .name =      "frame_types", \
    .real_name = TRS("Frame types"),    \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
  PARAM_GOP_SIZE, \
  PARAM_SCENE_CHANGE_THRESHOLD, \
  PARAM_SCENECHANGE_FACTOR, \
  PARAM_FLAG_CLOSED_GOP, \
  PARAM_FLAG2_STRICT_GOP

#define ENCODE_PARAM_VIDEO_FRAMETYPES_IPB \
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP, \
  PARAM_MAX_B_FRAMES, \
  PARAM_B_FRAME_STRATEGY

#define ENCODE_PARAM_VIDEO_ME \
  { \
    .name =      "motion_estimation", \
    .real_name = TRS("Motion estimation"),      \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_ME_METHOD,                      \
    PARAM_ME_CMP,                      \
    PARAM_ME_CMP_CHROMA,\
    PARAM_ME_RANGE,\
    PARAM_ME_THRESHOLD,\
    PARAM_MB_DECISION,\
    PARAM_DIA_SIZE

#define ENCODE_PARAM_VIDEO_ME_PRE \
  { \
    .name =      "motion_estimation", \
    .real_name = TRS("ME pre-pass"),    \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_PRE_ME,\
    PARAM_ME_PRE_CMP,\
    PARAM_ME_PRE_CMP_CHROMA,\
    PARAM_PRE_DIA_SIZE

#define ENCODE_PARAM_VIDEO_QPEL \
  { \
    .name =      "qpel_motion_estimation", \
    .real_name = TRS("Qpel ME"),           \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_FLAG_QPEL, \
    PARAM_ME_SUB_CMP,\
    PARAM_ME_SUB_CMP_CHROMA,\
    PARAM_ME_SUBPEL_QUALITY


#define ENCODE_PARAM_VIDEO_MASKING \
  { \
    .name =      "masking", \
    .real_name = TRS("Masking"),        \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_LUMI_MASKING, \
    PARAM_DARK_MASKING, \
    PARAM_TEMPORAL_CPLX_MASKING, \
    PARAM_SPATIAL_CPLX_MASKING, \
    PARAM_BORDER_MASKING, \
    PARAM_P_MASKING, \
    PARAM_FLAG_NORMALIZE_AQP

#define ENCODE_PARAM_VIDEO_MISC \
  { \
    .name =      "misc", \
    .real_name = TRS("Misc"),           \
    .type =      LQT_PARAMETER_SECTION, \
  }, \
    PARAM_STRICT_STANDARD_COMPLIANCE, \
    PARAM_NOISE_REDUCTION, \
    PARAM_FLAG_GRAY, \
    PARAM_FLAG_BITEXACT, \
    PARAM_THREAD_COUNT

#define ENCODE_PARAM_IMX \
  { \
    .name = "imx_bitrate",                        \
    .real_name = TRS("Bitrate (Mbps)"),                \
    .type = LQT_PARAMETER_STRINGLIST, \
    .val_default = { .val_string =  "50" },             \
    .stringlist_options = (char *[]){ "30", "40", "50", \
                                      (char *)0 }       \
  }, \
  {  \
    .name = "imx_strip_vbi",         \
    .real_name = TRS("Strip VBI"),   \
    .type = LQT_PARAMETER_INT,       \
    .val_default = { .val_int = 1 }  \
  }

#define DECODE_PARAM_AUDIO

#define DECODE_PARAM_VIDEO \
  PARAM_FLAG_GRAY

static lqt_parameter_info_static_t encode_parameters_mpeg4[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IPB,
  PARAM_FLAG_AC_PRED_MPEG4,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IPB,
  PARAM_FLAG_CBP_RD,
  ENCODE_PARAM_VIDEO_ME,
  PARAM_FLAG_GMC,
  PARAM_FLAG_4MV,
  PARAM_FLAG_MV0,
  PARAM_FLAG_QP_RD,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_QPEL,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};

#if 0
static lqt_parameter_info_static_t encode_parameters_dx50[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,
  ENCODE_PARAM_VIDEO_ME,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};
#endif

static lqt_parameter_info_static_t encode_parameters_h263[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,
  ENCODE_PARAM_VIDEO_ME,
  PARAM_FLAG_4MV,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  PARAM_FLAG_OBMC,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t encode_parameters_h263p[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,
  ENCODE_PARAM_VIDEO_ME,
  PARAM_FLAG_4MV,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  PARAM_FLAG_H263P_AIV,
  PARAM_FLAG_OBMC,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t encode_parameters_msmpeg4v3[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,
  ENCODE_PARAM_VIDEO_ME,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};



static lqt_parameter_info_static_t encode_parameters_dvvideo[] = {
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t encode_parameters_mjpeg[] = {
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_I,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t encode_parameters_ffv1[] = {
  PARAM_FFV1_CODER_TYPE,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t encode_parameters_imx[] = {
  ENCODE_PARAM_IMX,
  { /* End of parameters */ }
};


static lqt_parameter_info_static_t encode_parameters_audio[] = {
  ENCODE_PARAM_AUDIO,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t decode_parameters_video[] = {
  DECODE_PARAM_VIDEO,
  { /* End of parameters */ }
};

static lqt_parameter_info_static_t decode_parameters_audio[] = {
  { /* End of parameters */ }
};

struct CODECIDMAP
  {
  int id;
  int index;
  AVCodec const *encoder;
  AVCodec const *decoder;
  lqt_parameter_info_static_t * encode_parameters;
  lqt_parameter_info_static_t * decode_parameters;
  lqt_image_size_static_t     * image_sizes;
  
  char *short_name;
  char *name;
  char *fourccs[MAX_FOURCCS];
  int   wav_ids[MAX_WAV_IDS];
  int compatibility_flags;
  /*
   *  We explicitely allow, if encoding is allowed.
   *  This prevents the spreading of broken files
   */
  int   do_encode;
  int   * encoding_colormodels;
  
  lqt_compression_id_t compression_id;
  };

/* Image sizes */

static lqt_image_size_static_t image_sizes_imx[] =
  {
    { 720, 576 }, // PAL without VBI
    { 720, 608 }, // PAL with VBi
    { 720, 486 }, // NTSC without VBI
    { 720, 512 }, // NTSC with VBi
    { /* End */ },
  };

#if 0 // TODO: Simpify DV encoders
static lqt_image_size_static_t image_sizes_dv[] =
  {
    { 720, 575 }, // PAL
    { 720, 480 }, // NTSC
    { /* End */ },
  };
#endif

static lqt_image_size_static_t image_sizes_h263[] =
  {
    {  128, 96 }, // sub-QCIF
    {  176, 144 }, // QCIF
    {  352, 288 }, // CIF
    {  704, 576 }, // 4CIF
    { 1408, 1152 }, // 16CIF
    { /* End */ },
  };

static lqt_image_size_static_t image_sizes_dnxhd[] =
  {
    { 1920, 1080 },
    { 1280,  720 },
    { /* End */ },
  };

static lqt_image_size_static_t image_sizes_dv[] =
  {
    {  720, 576 },
    {  720, 480 },
    { /* End */ },
  };


/* Video; tables from mplayers config... */

static struct CODECIDMAP codecidmap_v[] =
  {
    {
      .id = AV_CODEC_ID_MPEG1VIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "mpg1",
      .name = TRS("FFMPEG Mpeg 1 Video"),
      .fourccs = { "mpg1", "MPG1", "pim1", "PIM1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE }
    },
    {
      .id = AV_CODEC_ID_MPEG4,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_mpeg4,
      .decode_parameters = decode_parameters_video,
      .short_name = "mpg4",
      .name = TRS("FFMPEG MPEG-4"),
      .fourccs = { "mp4v", "divx", "DIV1", "div1", "MP4S", "mp4s",
                   "M4S2", "m4s2", "xvid", "XVID", "XviD", "DX50",
                   "dx50", "DIVX", "MP4V", "3IV2", "FMP4", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4 |
                             LQT_FILE_3GP  | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_MPEG4_ASP,
    },
    {
      .id = AV_CODEC_ID_MSMPEG4V1,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "msmpeg4v1",
      .name = TRS("FFMPEG MSMpeg 4v1"),
      .fourccs = { "DIV1", "div1", "MPG4", "mpg4", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_MSMPEG4V2,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "msmpeg4v2",
      .name = TRS("FFMPEG MSMpeg 4v2"),
      .fourccs = { "DIV2", "div2", "MP42", "mp42", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_MSMPEG4V3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_msmpeg4v3,
      .decode_parameters = decode_parameters_video,
      .short_name = "msmpeg4v3",
      .name = TRS("FFMPEG MSMpeg 4v3 (DivX 3 compatible)"),
      .fourccs = { "DIV3", "mpg3", "MP43", "mp43", "DIV5", "div5", "DIV6",
                   "MPG3", "div6", "div3", "DIV4", "div4", "AP41", "ap41",
                   "3IVD", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
    },
    {
      .id = AV_CODEC_ID_MSMPEG4V3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_msmpeg4v3,
      .decode_parameters = decode_parameters_video,
      .short_name = "msmpeg4v3_wmp",
      .name = TRS("FFMPEG MSMpeg 4v3 (WMP compatible)"),
      .fourccs = { "MP43", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
    },
#if 0
    {
      .id = AV_CODEC_ID_WMV1,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_mpegvideo,
      .decode_parameters = decode_parameters_video,
      .short_name = "wmv1",
      .name = TRS("FFMPEG WMV1"),
      .fourccs = { "WMV1", "wmv1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
#endif
    {
      .id = AV_CODEC_ID_H263,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "h263_dec",
      .name = TRS("FFMPEG H263"),
      .fourccs = { "H263", "h263", "U263", "u263", "s263", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4 | LQT_FILE_3GP,
    },
    {
      .id = AV_CODEC_ID_H263,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_h263,
      .decode_parameters = decode_parameters_video,
      .image_sizes = image_sizes_h263,
      .short_name = "h263",
      .name = TRS("FFMPEG H263"),
      .fourccs = { "H263", "h263", "s263", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4 | LQT_FILE_3GP,
      .do_encode = 1,
    },
    {
      .id = AV_CODEC_ID_H264,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "h264",
      .name = TRS("FFMPEG H264"),
      .fourccs = { "avc1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_H263P,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_h263p,
      .decode_parameters = decode_parameters_video,
      .short_name = "h263p",
      .name = TRS("FFMPEG H263+"),
      .fourccs = { "U263", "u263", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
      .do_encode = 1,
    },
    {
      .id = AV_CODEC_ID_H263I,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "i263",
      .name = TRS("FFMPEG I263"),
      .fourccs = { "I263", "i263", "viv1", "VIV1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_SVQ1,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "svq1",
      .name = TRS("FFMPEG Sorenson Video 1"),
      .fourccs = { "SVQ1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_SVQ3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "svq3",
      .name = TRS("FFMPEG Sorenson Video 3"),
      .fourccs = { "SVQ3", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_MJPEG,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_mjpeg,
      .decode_parameters = decode_parameters_video,
      .short_name = "mjpg",
      .name = TRS("FFMPEG MJPEG"),
      .fourccs = { "MJPG", "mjpg", "JPEG", "jpeg", "dmb1", "AVDJ", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
    },
    {
      .id = AV_CODEC_ID_MJPEGB,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_mjpeg,
      .short_name = "mjpegb",
      .name = TRS("FFMPEG Motion JPEG-B"),
      .fourccs = { "mjpb", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
      .do_encode = 1,
    },
#if LIBAVCODEC_BUILD >= 3346688
    {
      .id = AV_CODEC_ID_TARGA,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "TGA",
      .name = TRS("FFMPEG Targa"),
      .fourccs = { "tga ", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
#endif
#if LIBAVCODEC_BUILD >= 3347456
    {
      .id = AV_CODEC_ID_TIFF,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "TIFF",
      .name = TRS("FFMPEG TIFF"),
      .fourccs = { "tiff", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
#endif
    {
      .id = AV_CODEC_ID_8BPS,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "8BPS",
      .name = TRS("FFMPEG Quicktime Planar RGB (8BPS)"),
      .fourccs = { "8BPS", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_INDEO3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "indeo",
      .name = TRS("FFMPEG Intel Indeo 3"),
      .fourccs = { "IV31", "IV32", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_RPZA,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "rpza",
      .name = TRS("FFMPEG Apple Video"),
      .fourccs = { "rpza", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_SMC,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "smc",
      .name = TRS("FFMPEG Apple Graphics"),
      .fourccs = { "smc ", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_CINEPAK,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "cinepak",
      .name = TRS("FFMPEG Cinepak"),
      .fourccs = { "cvid", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_CYUV,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "cyuv",
      .name = TRS("FFMPEG Creative YUV"),
      .fourccs = { "CYUV", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_QTRLE,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "rle",
      .name = TRS("FFMPEG RLE"),
      .fourccs = { "rle ", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
      .do_encode = 1,
      .encoding_colormodels = (int[]){ BC_RGB888, BC_RGBA8888, LQT_COLORMODEL_NONE },
    },
    {
      .id = AV_CODEC_ID_MSRLE,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "wrle",
      .name = TRS("FFMPEG Microsoft RLE"),
      .fourccs = { "WRLE", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_DVVIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_dvvideo,
      .decode_parameters = decode_parameters_video,
      .short_name = "dv",
      .name = TRS("FFMPEG DV"),
      .fourccs = { "dvc ", "dvcp", "dvpp", "dvsd", "dv25", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_DV, // Also writes all other DV flavours
      .image_sizes = image_sizes_dv,
    },
    {
      .id = AV_CODEC_ID_DVVIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_dvvideo,
      .decode_parameters = decode_parameters_video,
      .short_name = "dvcpro",
      .name = TRS("FFMPEG DVCPRO"),
      .fourccs = { "dvpp", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_DV, // Also writes all other DV flavours
      .image_sizes = image_sizes_dv,
    },
    {
      .id = AV_CODEC_ID_DVVIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_dvvideo,
      .decode_parameters = decode_parameters_video,
      .short_name = "dv50",
      .name = TRS("FFMPEG DVCPRO50"),
      .fourccs = { "dv5p", "dv5n", "dv50", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
      .image_sizes = image_sizes_dv,
    },
    /* DVCPRO HD (decoding only for now) */
    {
      .id = AV_CODEC_ID_DVVIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_dvvideo,
      .decode_parameters = decode_parameters_video,
      .short_name = "dvcprohd",
      .name = TRS("FFMPEG DVCPROHD"),
      .fourccs = { "dvhq", // DVCPRO HD 720p50
                   "dvhp", // DVCPRO HD 720p60
                   "dvh5", // DVCPRO HD 50i produced by FCP
                   "dvh6", // DVCPRO HD 60i produced by FCP
                   "dvh3", // DVCPRO HD 30p produced by FCP
                   "dvh1", // 
                   "AVd1", // AVID DV100
                   (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
      // .do_encode = 1
    },
    {
      .id = AV_CODEC_ID_FFVHUFF,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .short_name = "ffvhuff",
      .name = TRS("FFMPEG modified huffyuv lossless"),
	    .fourccs = { "FFVH", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = 0,
      .do_encode = 1
    },
    {
      .id = AV_CODEC_ID_FFV1,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .encode_parameters = encode_parameters_ffv1,
      .short_name = "ffv1",
      .name = TRS("FFMPEG codec #1 (lossless)"),
      .fourccs = { "FFV1", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = 0,
      .do_encode = 1,
      .encoding_colormodels = (int[]){ BC_YUV420P, BC_YUV444P, BC_YUV422P, BC_YUV411P, LQT_COLORMODEL_NONE },
    },
#if LIBAVCODEC_BUILD >= 3352576
    {
      .id = AV_CODEC_ID_DNXHD,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .image_sizes = image_sizes_dnxhd,
      .short_name = "dnxhd",
      .name = TRS("FFMPEG dnxhd"),
            .fourccs = { "AVdn", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT,
      .do_encode = 1,
      .encoding_colormodels = (int[]){ BC_YUV422P, BC_YUVJ422P, BC_YUV422P10, BC_YUVJ422P10, LQT_COLORMODEL_NONE }
    },
#endif
    {
      .id = AV_CODEC_ID_MPEG2VIDEO,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_video,
      .encode_parameters = encode_parameters_imx,
      .image_sizes = image_sizes_imx,
      .short_name = "imx",
      .name = TRS("FFMPEG IMX"),
      .fourccs = { "mx3p", "mx3n", "mx4p", "mx4n", "mx5p", "mx5n", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_D10
    },
};

/* Audio */

static struct CODECIDMAP codecidmap_a[] =
  {
    {
      .id = AV_CODEC_ID_MP3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .decode_parameters = decode_parameters_audio,
      .short_name = "mp3",
      .name = TRS("FFMPEG MPEG-1/2 audio layer 1/2/3"),
      .fourccs = { ".mp2", ".MP2", "ms\0\x50", "MS\0\x50",".mp3", ".MP3",
                   "ms\0\x55", "MS\0\x55", (char *)0 },
      .wav_ids = { 0x50, 0x55, LQT_WAV_ID_NONE },
    },
    {
      .id = AV_CODEC_ID_MP2,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_audio,
      .decode_parameters = decode_parameters_audio,
      .short_name = "mp2",
      .name = TRS("FFMPEG Mpeg Layer 2 Audio"),
      .fourccs = { ".mp2", ".MP2", (char *)0 },
      .wav_ids = { 0x55, LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_AVI | LQT_FILE_AVI_ODML,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_MP2,
    },
    {
      .id = AV_CODEC_ID_AC3,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_audio,
      .decode_parameters = decode_parameters_audio,
      .short_name = "ac3",
      .name = TRS("FFMPEG AC3 Audio"),
      .fourccs = { "ac-3", ".ac3", ".AC3", (char *)0 },
      .wav_ids = { 0x2000, LQT_WAV_ID_NONE },
      .compatibility_flags = LQT_FILE_AVI | LQT_FILE_AVI_ODML | LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4,
      .do_encode = 1,
      .compression_id = LQT_COMPRESSION_AC3,
    },
    {
      .id = AV_CODEC_ID_QDM2,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .encode_parameters = encode_parameters_audio,
      .decode_parameters = decode_parameters_audio,
      .short_name = "qdm2",
      .name = TRS("FFMPEG QDM2 Audio"),
      .fourccs = { "QDM2", (char *)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
      //.do_encode = 1,
    },
#if 1
    /* Doesn't work as long as audio chunks are not split into VBR "Samples" */
    {
      .id = AV_CODEC_ID_ALAC,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "alac",
      .name = TRS("FFMPEG Apple lossless"),
      .fourccs = { "alac", (char*)0 },
      .wav_ids = { LQT_WAV_ID_NONE },
    },
#endif
#if 1
    /* Sounds ugly */
    {
      .id = AV_CODEC_ID_ADPCM_MS,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "adpcm_ms",
      .name = TRS("FFMPEG McRowsoft ADPCM"),
      .fourccs = { "ms\0\x02", "MS\0\x02", (char*)0 },
      .wav_ids = { 0x02, LQT_WAV_ID_NONE },
    },
#endif
#if 1
    /* Sounds ugly */
    {
      .id = AV_CODEC_ID_ADPCM_IMA_WAV,
      .index = -1,
      .encoder = NULL,
      .decoder = NULL,
      .short_name = "ima_adpcm_wav",
      .name = TRS("FFMPEG ADPCM ima WAV"),
      .fourccs = { "ms\0\x11", "MS\0\x11", (char*)0 },
      .wav_ids = { 0x11, LQT_WAV_ID_NONE },
    }
#endif
  };

#define NUMMAPS_A ((int)(sizeof(codecidmap_a)/sizeof(struct CODECIDMAP)))
#define NUMMAPS_V ((int)(sizeof(codecidmap_v)/sizeof(struct CODECIDMAP)))

// static void ffmpeg_map_init(void) __attribute__ ((constructor));

static void ffmpeg_map_init(void)
  {
  int i;
  if(ffmpeg_num_video_codecs >= 0)
    {
    return;
    }
    
//  avcodec_init();
#if LIBAVCODEC_VERSION_MAJOR < 59
  avcodec_register_all();
#endif

  ffmpeg_num_video_codecs = 0;
  ffmpeg_num_audio_codecs = 0;
  
  for(i = 0; i < NUMMAPS_V; i++)
    {
    if(codecidmap_v[i].do_encode)
      codecidmap_v[i].encoder = avcodec_find_encoder(codecidmap_v[i].id);
    codecidmap_v[i].decoder = avcodec_find_decoder(codecidmap_v[i].id);

    if(codecidmap_v[i].encoder || codecidmap_v[i].decoder)
      {
      codecidmap_v[i].index = ffmpeg_num_video_codecs++;
      }
    }
  for(i = 0; i < NUMMAPS_A; i++)
    {
    if(codecidmap_a[i].do_encode)
      codecidmap_a[i].encoder = avcodec_find_encoder(codecidmap_a[i].id);
    codecidmap_a[i].decoder = avcodec_find_decoder(codecidmap_a[i].id);

    if(codecidmap_a[i].encoder || codecidmap_a[i].decoder)
      {
      codecidmap_a[i].index = ffmpeg_num_audio_codecs++ + ffmpeg_num_video_codecs;
      }
    }
  }

/* Template */

static char ffmpeg_name[256];
static char ffmpeg_long_name[256];
static char ffmpeg_description[256];

static lqt_codec_info_static_t codec_info_ffmpeg =
  {
    .name =                ffmpeg_name,
    .long_name =           ffmpeg_long_name,
    .description =         ffmpeg_description,
    .fourccs =             NULL,
    .wav_ids =             NULL,
    .type =                0,
    .direction =           0,
    .encoding_parameters = NULL,
    .decoding_parameters = NULL
  };

/* These are called from the plugin loader */

int ffmpegcodec_get_num_codecs()
{
	ffmpeg_map_init();
	return ffmpeg_num_video_codecs + ffmpeg_num_audio_codecs;
}

static void set_codec_info(struct CODECIDMAP * map)
  {
  // char * capabilities = (char*)0;

  codec_info_ffmpeg.fourccs = map->fourccs;
  codec_info_ffmpeg.wav_ids = map->wav_ids;
  codec_info_ffmpeg.image_sizes = map->image_sizes;
  if(map->encoder && map->decoder)
    {
    codec_info_ffmpeg.direction = LQT_DIRECTION_BOTH;
    codec_info_ffmpeg.encoding_parameters = map->encode_parameters;
    codec_info_ffmpeg.decoding_parameters = map->decode_parameters;
    //    capabilities = "Codec";
    codec_info_ffmpeg.compatibility_flags = map->compatibility_flags;
    codec_info_ffmpeg.encoding_colormodels = map->encoding_colormodels;
    codec_info_ffmpeg.compression_id = map->compression_id;
    }
  else if(map->encoder)
    {
    codec_info_ffmpeg.direction = LQT_DIRECTION_ENCODE;
    codec_info_ffmpeg.encoding_parameters = map->encode_parameters;
    codec_info_ffmpeg.decoding_parameters = NULL;
    codec_info_ffmpeg.compatibility_flags = map->compatibility_flags;
    codec_info_ffmpeg.encoding_colormodels = map->encoding_colormodels;
    codec_info_ffmpeg.compression_id = map->compression_id;

    //    capabilities = "Encoder";
    }
  else if(map->decoder)
    {
    codec_info_ffmpeg.direction = LQT_DIRECTION_DECODE;
    codec_info_ffmpeg.encoding_parameters = NULL;
    codec_info_ffmpeg.decoding_parameters = map->decode_parameters;
    //    capabilities = "Decoder";
    }

  snprintf(ffmpeg_name, 256, "ffmpeg_%s", map->short_name);

  snprintf(ffmpeg_long_name, 256, "%s", map->name);
  snprintf(ffmpeg_description, 256, "%s", map->name);
  
  if((map->encoder && (map->encoder->type == CODEC_TYPE_VIDEO)) ||
     (map->decoder && (map->decoder->type == CODEC_TYPE_VIDEO))){
       codec_info_ffmpeg.type = LQT_CODEC_VIDEO;
  } else {
       codec_info_ffmpeg.type = LQT_CODEC_AUDIO;
  }
  }

static struct CODECIDMAP * find_codec(int index)
  {
  int i;
  for(i = 0; i < NUMMAPS_V; i++)
    {
    if(codecidmap_v[i].index == index)
      return &codecidmap_v[i];
    }
  for(i = 0; i < NUMMAPS_A; i++)
    {
    if(codecidmap_a[i].index == index)
      return &codecidmap_a[i];
    }
  return (struct CODECIDMAP *)0;
  }

lqt_codec_info_static_t * ffmpegcodec_get_codec_info(int index)
  {
  struct CODECIDMAP * map;
  ffmpeg_map_init();
  map = find_codec(index);
  //        memset(&codec_info_ffmpeg, 0, sizeof(codec_info_ffmpeg));
  if(map)
    {
    set_codec_info(map);
    return &codec_info_ffmpeg;
    }
  return NULL;
  }

/*
 *   Return the actual codec constructor
 */

/* 
   This is where it gets ugly - make sure there are enough dummys to 
   handle all codecs!
*/

#define IFUNC(x) \
static void quicktime_init_codec_ffmpeg ## x(quicktime_codec_t * codec, \
                                             quicktime_audio_map_t *atrack, \
                                             quicktime_video_map_t *vtrack) \
  {                                                                     \
  int i;                                                                \
  for(i = 0; i < NUMMAPS_V; i++)                          \
    {                                                                   \
    if(codecidmap_v[i].index == x)                                      \
      {                                                                 \
      quicktime_init_video_codec_ffmpeg(codec, vtrack,                  \
                                        codecidmap_v[i].encoder,        \
                                        codecidmap_v[i].decoder);       \
      }                                                                 \
    }                                                                   \
  for(i = 0; i < NUMMAPS_A; i++)                          \
    {                                                                   \
    if(codecidmap_a[i].index == x)                                      \
      {                                                                 \
      quicktime_init_audio_codec_ffmpeg(codec, atrack,                  \
                                        codecidmap_a[i].encoder,        \
                                        codecidmap_a[i].decoder);       \
      }                                                                 \
    }                                                                   \
  }

IFUNC(0)
IFUNC(1)
IFUNC(2)
IFUNC(3)
IFUNC(4)
IFUNC(5)
IFUNC(6)
IFUNC(7)
IFUNC(8)
IFUNC(9)
IFUNC(10)
IFUNC(11)
IFUNC(12)
IFUNC(13)
IFUNC(14)
IFUNC(15)
IFUNC(16)
IFUNC(17)
IFUNC(18)
IFUNC(19)
IFUNC(20)
IFUNC(21)
IFUNC(22)
IFUNC(23)
IFUNC(24)
IFUNC(25)
IFUNC(26)
IFUNC(27)
IFUNC(28)
IFUNC(29)
IFUNC(30)
IFUNC(31)
IFUNC(32)
IFUNC(33)
IFUNC(34)
IFUNC(35)
IFUNC(36)
IFUNC(37)
IFUNC(38)
IFUNC(39)
IFUNC(40)
#undef IFUNC
#define MAX_FUNC 40

lqt_init_codec_func_t ffmpegcodec_get_codec(int index)
{
	ffmpeg_map_init();
	if(index > MAX_FUNC) {
		lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
                        "Codec index too large: %d", index);
		return NULL;
	}
	switch(index) {
		case 0: return quicktime_init_codec_ffmpeg0;
		case 1: return quicktime_init_codec_ffmpeg1;
		case 2:	return quicktime_init_codec_ffmpeg2;
		case 3:	return quicktime_init_codec_ffmpeg3;
		case 4:	return quicktime_init_codec_ffmpeg4;
		case 5:	return quicktime_init_codec_ffmpeg5;
		case 6:	return quicktime_init_codec_ffmpeg6;
		case 7:	return quicktime_init_codec_ffmpeg7;
		case 8:	return quicktime_init_codec_ffmpeg8;
		case 9:	return quicktime_init_codec_ffmpeg9;
		case 10: return quicktime_init_codec_ffmpeg10;
		case 11: return quicktime_init_codec_ffmpeg11;
		case 12: return quicktime_init_codec_ffmpeg12;
		case 13: return quicktime_init_codec_ffmpeg13;
		case 14: return quicktime_init_codec_ffmpeg14;
		case 15: return quicktime_init_codec_ffmpeg15;
		case 16: return quicktime_init_codec_ffmpeg16;
		case 17: return quicktime_init_codec_ffmpeg17;
		case 18: return quicktime_init_codec_ffmpeg18;
		case 19: return quicktime_init_codec_ffmpeg19;
		case 20: return quicktime_init_codec_ffmpeg20;
		case 21: return quicktime_init_codec_ffmpeg21;
		case 22: return quicktime_init_codec_ffmpeg22;
		case 23: return quicktime_init_codec_ffmpeg23;
		case 24: return quicktime_init_codec_ffmpeg24;
		case 25: return quicktime_init_codec_ffmpeg25;
		case 26: return quicktime_init_codec_ffmpeg26;
		case 27: return quicktime_init_codec_ffmpeg27;
		case 28: return quicktime_init_codec_ffmpeg28;
		case 29: return quicktime_init_codec_ffmpeg29;
		case 30: return quicktime_init_codec_ffmpeg30;
		case 31: return quicktime_init_codec_ffmpeg31;
		case 32: return quicktime_init_codec_ffmpeg32;
		case 33: return quicktime_init_codec_ffmpeg33;
		case 34: return quicktime_init_codec_ffmpeg34;
		case 35: return quicktime_init_codec_ffmpeg35;
		case 36: return quicktime_init_codec_ffmpeg36;
		case 37: return quicktime_init_codec_ffmpeg37;
		case 38: return quicktime_init_codec_ffmpeg38;
		case 39: return quicktime_init_codec_ffmpeg39;
		case 40: return quicktime_init_codec_ffmpeg40;
		default:
			break;
	}
	return (lqt_init_codec_func_t)0;
}


/*******************************************************************************
 params.c

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
#include "params.h"
#include AVCODEC_HEADER
#include <string.h>
#include "ffmpeg.h"

typedef struct
  {
  char * s;
  int i;
  } enum_t;

#define PARAM_INT(name, var) \
  if(!strcasecmp(name, key)) \
    { \
    ctx->var = *(int*)value;                         \
    found = 1; \
    }

#define PARAM_INT_SCALE(name, var, scale)   \
  if(!strcasecmp(name, key)) \
    { \
    ctx->var = (*(int*)value) * scale;                 \
    found = 1; \
    }

#define PARAM_QP2LAMBDA(name, var)   \
  if(!strcasecmp(name, key)) \
    { \
    ctx->var = (int)((*(float*)value) * FF_QP2LAMBDA+0.5);  \
    found = 1; \
    }

#define PARAM_FLOAT(name, var)   \
  if(!strcasecmp(name, key)) \
    { \
    ctx->var = (*(float*)value);                 \
    found = 1; \
    }

#define PARAM_CMP_CHROMA(name,var)              \
  {                                             \
  if(!strcasecmp(name, key))                    \
    {                                           \
    if(*(int*)value)                            \
      ctx->var |= FF_CMP_CHROMA;                \
    else                                        \
      ctx->var &= ~FF_CMP_CHROMA;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

#define PARAM_FLAG(name,flag)                  \
  {                                             \
  if(!strcasecmp(name, key))                    \
    {                                           \
    if(*(int*)value)                            \
      ctx->flags |= flag;                \
    else                                        \
      ctx->flags &= ~flag;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

#define PARAM_FLAG2(name,flag)                  \
  {                                             \
  if(!strcasecmp(name, key))                    \
    {                                           \
    if(*(int*)value)                            \
      ctx->flags2 |= flag;                \
    else                                        \
      ctx->flags2 &= ~flag;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

#define PARAM_DICT_FLAG(name, dict_name)        \
  {                                             \
  if(!strcasecmp(name, key))                    \
    {                                           \
    char buf[128];                              \
    snprintf(buf, 128, "%d", *(int*)value);     \
    av_dict_set(options, dict_name, buf, 0);    \
    found = 1;                                  \
    }                                           \
  }


enum_t me_method[] =
  {
    { "Zero",  ME_ZERO },
    { "Phods", ME_PHODS },
    { "Log",   ME_LOG },
    { "X1",    ME_X1 },
    { "Epzs",  ME_EPZS },
    { "Full",  ME_FULL }
  };

enum_t prediction_method[] =
  {
    { "Left",   FF_PRED_LEFT },
    { "Plane",  FF_PRED_PLANE },
    { "Median", FF_PRED_MEDIAN }
  };

enum_t compare_func[] =
  {
    { "SAD",  FF_CMP_SAD },
    { "SSE",  FF_CMP_SSE },
    { "SATD", FF_CMP_SATD },
    { "DCT",  FF_CMP_DCT },
    { "PSNR", FF_CMP_PSNR },
    { "BIT",  FF_CMP_BIT },
    { "RD",   FF_CMP_RD },
    { "ZERO", FF_CMP_ZERO },
    { "VSAD", FF_CMP_VSAD },
    { "VSSE", FF_CMP_VSSE },
    { "NSSE", FF_CMP_NSSE }
  };

enum_t mb_decision[] =
  {
    { "Use compare function", FF_MB_DECISION_SIMPLE },
    { "Fewest bits",          FF_MB_DECISION_BITS },
    { "Rate distoration",     FF_MB_DECISION_RD }
  };

enum_t coder_type[] =
  {
    { "VLC",                  FF_CODER_TYPE_VLC },
    { "Arithmetic",           FF_CODER_TYPE_AC },
    { "Raw",                  FF_CODER_TYPE_RAW },
    { "RLE",                  FF_CODER_TYPE_RLE },
    { "Deflate",              FF_CODER_TYPE_DEFLATE },
  };

#define PARAM_ENUM(name, var, arr) \
  if(!strcasecmp(key, name)) \
    { \
    for(i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) \
      {                                             \
      if(!strcasecmp((char*)value, arr[i].s))       \
        {                                           \
        ctx->var = arr[i].i;                        \
        break;                                      \
        }                                           \
      }                                             \
    found = 1;                                      \
    }


void lqt_ffmpeg_set_parameter(AVCodecContext * ctx,
#if LIBAVCODEC_VERSION_MAJOR >= 54
                              AVDictionary ** options,
#endif
                              const char * key,
                              const void * value)
  {
  int found = 0, i;
/*
 *   IMPORTANT: To keep the mess at a reasonable level,
 *   *all* parameters *must* appear in the same order as in
 *   the AVCocecContext structure, except the flags, which come at the very end
 */
  PARAM_INT_SCALE("ff_bit_rate_audio",bit_rate,1000);
  PARAM_INT_SCALE("ff_bit_rate_video",bit_rate,1000);
  PARAM_INT_SCALE("ff_bit_rate_tolerance",bit_rate_tolerance,1000);
  PARAM_ENUM("ff_me_method",me_method,me_method);
  PARAM_INT("ff_gop_size",gop_size);
  PARAM_FLOAT("ff_qcompress",qcompress);
  PARAM_FLOAT("ff_qblur",qblur);
  PARAM_INT("ff_qmin",qmin);
  PARAM_INT("ff_qmax",qmax);
  PARAM_INT("ff_max_qdiff",max_qdiff);
  PARAM_INT("ff_max_b_frames",max_b_frames);
  PARAM_FLOAT("ff_b_quant_factor",b_quant_factor);
  PARAM_INT("ff_b_frame_strategy",b_frame_strategy);
  PARAM_INT("ff_luma_elim_threshold",luma_elim_threshold);
  PARAM_INT("ff_chroma_elim_threshold",chroma_elim_threshold);
  PARAM_INT("ff_strict_std_compliance",strict_std_compliance);
  PARAM_QP2LAMBDA("ff_b_quant_offset",b_quant_offset);
  PARAM_INT("ff_rc_min_rate",rc_min_rate);
  PARAM_INT("ff_rc_max_rate",rc_max_rate);
  PARAM_INT_SCALE("ff_rc_buffer_size",rc_buffer_size,1000);
  PARAM_FLOAT("ff_rc_buffer_aggressivity",rc_buffer_aggressivity);
  PARAM_FLOAT("ff_i_quant_factor",i_quant_factor);
  PARAM_QP2LAMBDA("ff_i_quant_offset",i_quant_offset);
  PARAM_FLOAT("ff_rc_initial_cplx",rc_initial_cplx);
  PARAM_FLOAT("ff_lumi_masking",lumi_masking);
  PARAM_FLOAT("ff_temporal_cplx_masking",temporal_cplx_masking);
  PARAM_FLOAT("ff_spatial_cplx_masking",spatial_cplx_masking);
  PARAM_FLOAT("ff_p_masking",p_masking);
  PARAM_FLOAT("ff_dark_masking",dark_masking);
  PARAM_ENUM("ff_prediction_method",prediction_method,prediction_method);
  PARAM_ENUM("ff_me_cmp",me_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_me_cmp_chroma",me_cmp);
  PARAM_ENUM("ff_me_sub_cmp",me_sub_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_me_sub_cmp_chroma",me_sub_cmp);
  PARAM_ENUM("ff_mb_cmp",mb_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_mb_cmp_chroma",mb_cmp);
  PARAM_ENUM("ff_ildct_cmp",ildct_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_ildct_cmp_chroma",ildct_cmp);
  PARAM_INT("ff_dia_size",dia_size);
  PARAM_INT("ff_last_predictor_count",last_predictor_count);
  PARAM_INT("ff_pre_me",pre_me);
  PARAM_ENUM("ff_me_pre_cmp",me_pre_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_pre_me_cmp_chroma",me_pre_cmp);
  PARAM_INT("ff_pre_dia_size",pre_dia_size);
  PARAM_INT("ff_me_subpel_quality",me_subpel_quality);
  PARAM_INT("ff_me_range",me_range);
  PARAM_ENUM("ff_mb_decision",mb_decision,mb_decision);
  PARAM_INT("ff_scenechange_threshold",scenechange_threshold);
  PARAM_QP2LAMBDA("ff_lmin", lmin);
  PARAM_QP2LAMBDA("ff_lmax", lmax);
  PARAM_INT("ff_noise_reduction",noise_reduction);
  PARAM_INT_SCALE("ff_rc_initial_buffer_occupancy",rc_initial_buffer_occupancy,1000);
  PARAM_INT("ff_inter_threshold",inter_threshold);
  PARAM_INT("ff_quantizer_noise_shaping",quantizer_noise_shaping);
  PARAM_INT("ff_thread_count",thread_count);
  PARAM_INT("ff_me_threshold",me_threshold);
  PARAM_INT("ff_mb_threshold",mb_threshold);
  PARAM_INT("ff_nsse_weight",nsse_weight);
  PARAM_FLOAT("ff_border_masking",border_masking);
  PARAM_QP2LAMBDA("ff_mb_lmin", mb_lmin);
  PARAM_QP2LAMBDA("ff_mb_lmax", mb_lmax);
  PARAM_INT("ff_me_penalty_compensation",me_penalty_compensation);
  PARAM_INT("ff_bidir_refine",bidir_refine);
  PARAM_INT("ff_brd_scale",brd_scale);
  PARAM_INT("ff_scenechange_factor",scenechange_factor);
  PARAM_FLAG("ff_flag_qscale",CODEC_FLAG_QSCALE);
  PARAM_FLAG("ff_flag_4mv",CODEC_FLAG_4MV);
  PARAM_FLAG("ff_flag_qpel",CODEC_FLAG_QPEL);
  PARAM_FLAG("ff_flag_gmc",CODEC_FLAG_GMC);
  PARAM_FLAG("ff_flag_mv0",CODEC_FLAG_MV0);
  //  PARAM_FLAG("ff_flag_part",CODEC_FLAG_PART); // Unused
  PARAM_FLAG("ff_flag_gray",CODEC_FLAG_GRAY);
  PARAM_FLAG("ff_flag_emu_edge",CODEC_FLAG_EMU_EDGE);
  PARAM_FLAG("ff_flag_normalize_aqp",CODEC_FLAG_NORMALIZE_AQP);
  //  PARAM_FLAG("ff_flag_alt_scan",CODEC_FLAG_ALT_SCAN); // Unused
#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
  PARAM_FLAG("ff_flag_trellis_quant",CODEC_FLAG_TRELLIS_QUANT);
#else
  PARAM_INT("ff_trellis",trellis);
#endif
  PARAM_FLAG("ff_flag_bitexact",CODEC_FLAG_BITEXACT);
  PARAM_FLAG("ff_flag_ac_pred",CODEC_FLAG_AC_PRED);
  //  PARAM_FLAG("ff_flag_h263p_umv",CODEC_FLAG_H263P_UMV); // Unused
  PARAM_FLAG("ff_flag_cbp_rd",CODEC_FLAG_CBP_RD);
  PARAM_FLAG("ff_flag_qp_rd",CODEC_FLAG_QP_RD);

#if LIBAVCODEC_VERSION_MAJOR >= 54
  PARAM_DICT_FLAG("ff_flag_h263p_aiv", "aiv");
  PARAM_DICT_FLAG("ff_flag_obmc","obmc");
  PARAM_DICT_FLAG("ff_flag_h263p_slice_struct","structured_slices");
#else
  PARAM_FLAG("ff_flag_h263p_aiv",CODEC_FLAG_H263P_AIV);
  PARAM_FLAG("ff_flag_obmc",CODEC_FLAG_OBMC);
  PARAM_FLAG("ff_flag_h263p_slice_struct",CODEC_FLAG_H263P_SLICE_STRUCT);
#endif

  PARAM_FLAG("ff_flag_loop_filter",CODEC_FLAG_LOOP_FILTER);
  PARAM_FLAG("ff_flag_closed_gop",CODEC_FLAG_CLOSED_GOP);
  PARAM_FLAG2("ff_flag2_fast",CODEC_FLAG2_FAST);
  PARAM_FLAG2("ff_flag2_strict_gop",CODEC_FLAG2_STRICT_GOP);
  PARAM_ENUM("ff_coder_type",coder_type,coder_type);
  
  }

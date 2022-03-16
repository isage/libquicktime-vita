/*******************************************************************************
 video.c

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
#include "ffmpeg.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define LOG_DOMAIN "ffmpeg_video"

#ifdef HAVE_LIBSWSCALE
#include SWSCALE_HEADER
#endif


#ifdef  AV_PIX_FMT_YUV422P10
#define AV_PIX_FMT_YUV422P10_OR_DUMMY AV_PIX_FMT_YUV422P10
#else
#define AV_PIX_FMT_YUV422P10_OR_DUMMY -1234
#endif

#if LIBAVCODEC_VERSION_INT >= ((54<<16)|(1<<8)|0)
#define ENCODE_VIDEO2 1
#else
#define ENCODE_VIDEO 1
#endif

/* We keep the numeric values the same as in the ACLR atom.
   The interpretation of values is based on trial and error. */
enum AvidYuvRange
  {
  AVID_UNKNOWN_YUV_RANGE = 0, /* This one is not valid in the ACLR atom. */
  AVID_FULL_YUV_RANGE    = 1, /* [0, 255] for Y, U and V. */
  AVID_NORMAL_YUV_RANGE  = 2  /* [16, 235] for Y, [16, 240] for U and V. */
  };

typedef struct
  {
  AVCodecContext * avctx;
  AVCodec * encoder;
  AVCodec * decoder;
  int initialized;

  int decoding_delay;

  uint8_t * buffer;
  int buffer_alloc;
  
  AVFrame * frame;
  uint8_t * frame_buffer;

  /* Colormodel */

  int do_imgconvert;
#ifdef HAVE_LIBSWSCALE
  struct SwsContext *swsContext;
#endif

  uint8_t ** tmp_rows;
  int tmp_row_span;
  int tmp_row_span_uv;
  
  /* Quality must be passed to the individual frame */

  int qscale;
  int imx_bitrate;
  int imx_strip_vbi;

  /* In some cases FFMpeg would report something like AV_PIX_FMT_YUV422P, while
     we would like to treat it as AV_PIX_FMT_YUVJ422P. It's only used for decoding */
  enum AVPixelFormat reinterpret_pix_fmt;
  
  int is_imx;
  int y_offset;

#if LIBAVCODEC_VERSION_MAJOR < 54
  AVPaletteControl palette;
#else
  int palette_sent;
#endif

#if LIBAVCODEC_VERSION_MAJOR >= 54
  AVDictionary * options;
#endif
  
  /* We decode the first frame during the init() function to
     obtain the stream colormodel */

  int have_frame;

  int write_global_header;
  int global_header_written;

  uint8_t * extradata; /* For decoding only, for encoding extradata is owned by lavc */

  /* Multipass control */
  int total_passes;
  int pass;
  char * stats_filename;
  FILE * stats_file;

#if LIBAVCODEC_BUILD >= ((52<<16)+(26<<8)+0)
  AVPacket pkt;
#endif

  /* Stuff for compressed H.264 reading */
  int nal_size_length;
  } quicktime_ffmpeg_video_codec_t;

/* ffmpeg <-> libquicktime colormodels */

/* Exact entries MUST come first */

static const struct
  {
  enum AVPixelFormat ffmpeg_id;
  int              lqt_id;
  int              exact;
  }
colormodels[] =
  {
    { AV_PIX_FMT_YUV420P,   BC_YUV420P,   1 }, ///< Planar YUV 4:2:0 (1 Cr & Cb sample per 2x2 Y samples)
#if LIBAVUTIL_VERSION_INT < (50<<16)
    { AV_PIX_FMT_YUV422,    BC_YUV422,    1 },
#else
    { AV_PIX_FMT_YUYV422,   BC_YUV422,    1 },
#endif
    { AV_PIX_FMT_RGB24,     BC_RGB888,    1 }, ///< Packed pixel, 3 bytes per pixel, RGBRGB...
    { AV_PIX_FMT_BGR24,     BC_BGR888,    1 }, ///< Packed pixel, 3 bytes per pixel, BGRBGR...
    { AV_PIX_FMT_YUV422P,   BC_YUV422P,   1 }, ///< Planar YUV 4:2:2 (1 Cr & Cb sample per 2x1 Y samples)
    { AV_PIX_FMT_YUV444P,   BC_YUV444P,   1 }, ///< Planar YUV 4:4:4 (1 Cr & Cb sample per 1x1 Y samples)
    { AV_PIX_FMT_YUV411P,   BC_YUV411P,   1 }, ///< Planar YUV 4:1:1 (1 Cr & Cb sample per 4x1 Y samples)
    { AV_PIX_FMT_YUV422P16, BC_YUV422P16, 1 }, ///< Planar 16 bit YUV 4:2:2 (1 Cr & Cb sample per 2x1 Y samples)
#ifdef AV_PIX_FMT_YUV422P10
    { AV_PIX_FMT_YUV422P10, BC_YUV422P10, 1 }, ///< 10 bit samples in uint16_t containers, planar 4:2:2
#endif
    { AV_PIX_FMT_RGB565,    BC_RGB565,    1 }, ///< always stored in cpu endianness
    { AV_PIX_FMT_YUVJ420P,  BC_YUVJ420P,  1 }, ///< Planar YUV 4:2:0 full scale (jpeg)
    { AV_PIX_FMT_YUVJ422P,  BC_YUVJ422P,  1 }, ///< Planar YUV 4:2:2 full scale (jpeg)
    { AV_PIX_FMT_YUVJ444P,  BC_YUVJ444P,  1 }, ///< Planar YUV 4:4:4 full scale (jpeg)
#if LIBAVUTIL_VERSION_INT < (50<<16)
    { AV_PIX_FMT_RGBA32,    BC_RGBA8888,  0 }, ///< Packed pixel, 4 bytes per pixel, BGRABGRA...
#else
    { AV_PIX_FMT_RGB32,     BC_RGBA8888,  0 }, ///< Packed pixel, 4 bytes per pixel, BGRABGRA...
#endif
    { AV_PIX_FMT_RGB555,    BC_RGB888,    0 }, ///< always stored in cpu endianness, most significant bit to 1
    { AV_PIX_FMT_GRAY8,     BC_RGB888,    0 },
    { AV_PIX_FMT_MONOWHITE, BC_RGB888,    0 }, ///< 0 is white
    { AV_PIX_FMT_MONOBLACK, BC_RGB888,    0 }, ///< 0 is black
    { AV_PIX_FMT_PAL8,      BC_RGB888,    0 }, ///< 8 bit with RGBA palette
    { AV_PIX_FMT_YUV410P,   BC_YUV420P,   0 }, ///< Planar YUV 4:1:0 (1 Cr & Cb sample per 4x4 Y samples)
  };

static const struct
  {
  int width;
  int height;
  int colormodel;
  char qt_fourcc[4];
  char avi_fourcc[4];
  }
dv_fourccs[] =
  {
    {  720,  480, BC_YUV411P, "dvc ", "dvsd" }, /* DV NTSC SD */
    {  720,  576, BC_YUV420P, "dvcp", "dvsd" }, /* DV PAL SD */
    {  720,  576, BC_YUV411P, "dvpp", "dv25" }, /* DVCPRO PAL SD */
    {  720,  480, BC_YUV422P, "dv5n", "dv50" }, /* DVCPRO50 NTSC SD */
    {  720,  576, BC_YUV422P, "dv5p", "dv50" }, /* DVCPRO50 PAL SD */
    {  960,  720, BC_YUV422P, "dvh1", "dvh1" }, /* DVCPROHD */
    { 1280, 1080, BC_YUV422P, "dvh1", "dvh1" }, /* DVCPROHD */
    { 1440, 1080, BC_YUV422P, "dvh1", "dvh1" }, /* DVCPROHD */
  };

static void set_dv_fourcc(int width, int height, int colormodel,
                                quicktime_trak_t * trak)
  {
  int i, index = -1;
  
  for(i = 0; i < sizeof(dv_fourccs)/sizeof(dv_fourccs[0]); i++)
    {
    if((dv_fourccs[i].width == width) &&
       (dv_fourccs[i].height == height) &&
       (dv_fourccs[i].colormodel == colormodel))
      {
      index = i;
      break;
      }
    }
  if(index < 0)
    return;
  
  if(trak->strl)
    {
    strncpy(trak->strl->strh.fccHandler, dv_fourccs[i].avi_fourcc, 4);
    strncpy(trak->strl->strf.bh.biCompression, dv_fourccs[i].avi_fourcc, 4);
    }
  else
    {
    strncpy(trak->mdia.minf.stbl.stsd.table[0].format, dv_fourccs[i].qt_fourcc, 4);
    }
  }

static int lqt_ffmpeg_delete_video(quicktime_codec_t *codec_base)
  {
  quicktime_ffmpeg_video_codec_t *codec = codec_base->priv;
  
  if(codec->extradata)
    free(codec->extradata);

  if(codec->stats_filename)
    free(codec->stats_filename);
        
  if(codec->stats_file)
    fclose(codec->stats_file);
        
  if(codec->initialized)
    {
    if(codec->avctx->stats_in)
      av_free(codec->avctx->stats_in);
    avcodec_close(codec->avctx);
    }
  av_free(codec->avctx);
        
  if(codec->frame_buffer) free(codec->frame_buffer);
  if(codec->buffer) free(codec->buffer);

  if(codec->frame) av_free(codec->frame);

#ifdef HAVE_LIBSWSCALE
  if(codec->swsContext)
    sws_freeContext(codec->swsContext);
#endif

#if LIBAVCODEC_VERSION_MAJOR >= 54
  if(codec->options)
    av_dict_free(&codec->options);
#endif
  
  if(codec->tmp_rows)
    lqt_rows_free(codec->tmp_rows);
  
  free(codec);
	
  return 0;
  }

static uint32_t lqt_ffmpeg_read_be32(uint8_t const* p)
  {
  return ((uint32_t)p[0] << 24) + ((uint32_t)p[1] << 16) + ((uint32_t)p[2] << 8) + (uint32_t)p[3];
  }

static void lqt_ffmpeg_write_be32(uint8_t** p, uint32_t val)
  {
  *(*p + 0) = (uint8_t)(val >> 24);
  *(*p + 1) = (uint8_t)(val >> 16);
  *(*p + 2) = (uint8_t)(val >> 8);
  *(*p + 3) = (uint8_t)val;
  *p += 4;
  }

static void lqt_ffmpeg_write_tag(uint8_t** p, char const* tag)
  {
  *(*p + 0) = tag[0];
  *(*p + 1) = tag[1];
  *(*p + 2) = tag[2];
  *(*p + 3) = tag[3];
  *p += 4;
  }

#ifndef HAVE_LIBSWSCALE
static void fill_avpicture(AVPicture * ret, unsigned char ** rows,
                           int lqt_colormodel,
                           int row_span, int row_span_uv)
  {
  switch(lqt_colormodel)
    {
    case BC_YUV420P:
    case BC_YUV422P:
      ret->data[0] = rows[0];
      ret->data[1] = rows[1];
      ret->data[2] = rows[2];
      ret->linesize[0] = row_span;
      ret->linesize[1] = row_span_uv ? row_span_uv : row_span/2;
      ret->linesize[2] = row_span_uv ? row_span_uv : row_span/2;
      break;
    case BC_YUV444P:
      ret->data[0] = rows[0];
      ret->data[1] = rows[1];
      ret->data[2] = rows[2];
      ret->linesize[0] = row_span;
      ret->linesize[1] = row_span_uv ? row_span_uv : row_span;
      ret->linesize[2] = row_span_uv ? row_span_uv : row_span;
      break;
    case BC_YUV411P:
      ret->data[0] = rows[0];
      ret->data[1] = rows[1];
      ret->data[2] = rows[2];
      ret->linesize[0] = row_span;
      ret->linesize[1] = row_span_uv ? row_span_uv : row_span/4;
      ret->linesize[2] = row_span_uv ? row_span_uv : row_span/4;
      break;
    case BC_YUV422:
    case BC_RGB888:  ///< Packed pixel, 3 bytes per pixel, RGBRGB...
    case BC_BGR888:  ///< Packed pixel, 3 bytes per pixel, BGRBGR...
    case BC_RGBA8888:  ///< Packed pixel, 4 bytes per pixel, BGRABGRA...
    case BC_RGB565:  ///< always stored in cpu endianness
      ret->data[0]     = rows[0];
      ret->linesize[0] = (int)(rows[1] - rows[0]);
      break;
    default:
      break;
    }
  }
#endif

static int lqt_tenbit_dnxhd_supported(AVCodec const* codec)
  {
  int i;
  if (!codec->pix_fmts)
    return 0;

  for (i = 0; codec->pix_fmts[i] != AV_PIX_FMT_NONE; ++i)
    {
    if (codec->pix_fmts[i] == AV_PIX_FMT_YUV422P10_OR_DUMMY)
      return 1;
    }

  return 0;
  }

static enum AVPixelFormat lqt_ffmpeg_get_ffmpeg_colormodel(int id)
  {
  int i;

  for(i = 0; i < sizeof(colormodels)/sizeof(colormodels[0]); i++)
    {
    if(colormodels[i].lqt_id == id)
      return colormodels[i].ffmpeg_id;
    }
  return AV_PIX_FMT_NB;
  }

static int lqt_ffmpeg_get_lqt_colormodel(enum AVPixelFormat id, int * exact)
  {
  int i;

  for(i = 0; i < sizeof(colormodels)/sizeof(colormodels[0]); i++)
    {
    if(colormodels[i].ffmpeg_id == id)
      {
      *exact = colormodels[i].exact;
      return colormodels[i].lqt_id;
      }
    }
  return LQT_COLORMODEL_NONE;
  }

static enum AvidYuvRange lqt_ffmpeg_get_avid_yuv_range(quicktime_trak_t *trak)
  {
  uint32_t len = 0;
  uint8_t const* ACLR = quicktime_stsd_get_user_atom(trak, "ACLR", &len);
  if (len >= 24)
    {
    switch (lqt_ffmpeg_read_be32(ACLR + 16))
      {
      case AVID_NORMAL_YUV_RANGE: return AVID_NORMAL_YUV_RANGE;
      case AVID_FULL_YUV_RANGE:   return AVID_FULL_YUV_RANGE;
      }
    }
  return AVID_UNKNOWN_YUV_RANGE;
  }

static void lqt_ffmpeg_setup_decoding_colormodel(quicktime_t *file, quicktime_video_map_t *vtrack, int* exact)
  {
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  codec->reinterpret_pix_fmt = codec->avctx->pix_fmt;

  /* First we try codec-specific colormodel matching. */
  if(codec->decoder->id == AV_CODEC_ID_DNXHD)
    {
    /* FFMpeg supports AV_PIX_FMT_YUV422P and AV_PIX_FMT_YUV422P10 for DNxHD, which
       we sometimes interpret as AV_PIX_FMT_YUVJ422P and AV_PIX_FMT_YUVJ422P10. */
    if (codec->avctx->pix_fmt == AV_PIX_FMT_YUV422P || codec->avctx->pix_fmt == AV_PIX_FMT_YUV422P10_OR_DUMMY)
      {
      int p10 = (codec->avctx->pix_fmt == AV_PIX_FMT_YUV422P10_OR_DUMMY);
      *exact = 1;
      if (lqt_ffmpeg_get_avid_yuv_range(vtrack->track) == AVID_FULL_YUV_RANGE)
        {
        vtrack->stream_cmodel = p10 ? BC_YUVJ422P10 : BC_YUVJ422P;
        codec->reinterpret_pix_fmt = p10 ? AV_PIX_FMT_YUV422P10_OR_DUMMY : AV_PIX_FMT_YUVJ422P;
        // Note: reinterpret_pix_fmt should really be AV_PIX_FMT_YUVJ422P10, except
        // there is no such colormodel in FFMpeg. Fortunately, it's not a problem
        // in this case, as reinterpret_pix_fmt is only used when *exact == 0.
        }
      else
        {
        vtrack->stream_cmodel = p10 ? BC_YUV422P10 : BC_YUV422P;
        codec->reinterpret_pix_fmt = p10 ? AV_PIX_FMT_YUV422P10_OR_DUMMY : AV_PIX_FMT_YUV422P;
        }
      return;
      }
    else
      {
      lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN, "Unexpected pixel format for DNxHD.");
      }
    }

    /* Fall back to generic colormodel matching. */
    vtrack->stream_cmodel = lqt_ffmpeg_get_lqt_colormodel(codec->avctx->pix_fmt, exact);
  }

static void lqt_ffmpeg_setup_encoding_colormodel(quicktime_video_map_t *vtrack)
  {
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  codec->avctx->pix_fmt = lqt_ffmpeg_get_ffmpeg_colormodel(vtrack->stream_cmodel);

  if (codec->encoder->id == AV_CODEC_ID_DNXHD)
    {
    /* FFMpeg's DNxHD encoder only supports AV_PIX_FMT_YUV422P and AV_PIX_FMT_YUV422P10
       and doesn't know anything about AV_PIX_FMT_YUVJ422P and AV_PIX_FMT_YUVJ422P10
       (in fact, the latter doesn't even exist) */
    codec->avctx->pix_fmt = AV_PIX_FMT_YUV422P;
    if (vtrack->stream_cmodel == BC_YUV422P10 || vtrack->stream_cmodel == BC_YUVJ422P10)
      {
      if (lqt_tenbit_dnxhd_supported(codec->encoder))
        codec->avctx->pix_fmt = AV_PIX_FMT_YUV422P10_OR_DUMMY;
      }
    }
  }


/* Convert ffmpeg RGBA32 to BC_RGBA888 */

/* From avcodec.h: */

/*
 * AV_PIX_FMT_RGBA32 is handled in an endian-specific manner. A RGBA
 * color is put together as:
 *  (A << 24) | (R << 16) | (G << 8) | B
 * This is stored as BGRA on little endian CPU architectures and ARGB on
 * big endian CPUs.
 */

/* The only question is: WHY? */

static void convert_image_decode_rgba(AVFrame * in_frame,
                                      unsigned char ** out_frame,
                                      int width, int height, int y_shift)
  {
  uint32_t r, g, b, a;
  uint32_t * src_ptr;
  uint8_t * dst_ptr;
  int i, j;
  for(i = y_shift; i < height + y_shift; i++)
    {
    src_ptr = (uint32_t*)(in_frame->data[0] + i * in_frame->linesize[0]);
    dst_ptr = out_frame[i];
    for(j = 0; j < width; j++)
      {
      a = ((*src_ptr) & 0xff000000) >> 24;
      r = ((*src_ptr) & 0x00ff0000) >> 16;
      g = ((*src_ptr) & 0x0000ff00) >> 8;
      b = ((*src_ptr) & 0x000000ff);
      dst_ptr[0] = r;
      dst_ptr[1] = g;
      dst_ptr[2] = b;
      dst_ptr[3] = a;
      dst_ptr += 4;
      src_ptr++;
      }
    }
  }

/* Convert RR GG BB AA -> AA RR GG BB */
static void convert_rgba_to_argb(uint8_t const* src, int src_bytes_per_line,
                                 uint8_t* dst, int dst_bytes_per_line,
                                 int rows, int cols)
  {
  int i, j;
  uint32_t const* src_line = (uint32_t const*)src;
  uint32_t* dst_line = (uint32_t*)dst;
  int const src_stride = src_bytes_per_line / 4;
  int const dst_stride = dst_bytes_per_line / 4;

  assert(src_bytes_per_line % 4 == 0);
  assert(dst_bytes_per_line % 4 == 0);

  for(i = 0; i < rows; ++i, src_line += src_stride, dst_line += dst_stride)
    {
    for(j = 0; j < cols; ++j)
      {
#ifdef WORDS_BIGENDIAN
      /* RR GG BB AA -> 0xRRGGBBAA -> 0xAARRGGBB -> AA RR GG BB */
      dst_line[j] = (src_line[j] << 24) | (src_line[j] >> 8);
#else
      /* RR GG BB AA -> 0xAABBGGRR -> 0xBBGGRRAA -> AA RR GG BB */
      dst_line[j] = (src_line[j] >> 24) | (src_line[j] << 8);
#endif
      }
    }
  }

/*
 *  Do a conversion from a ffmpeg special colorspace
 *  to a libquicktime special one
 */

static void convert_image_decode(quicktime_ffmpeg_video_codec_t *codec,
                                 AVFrame * in_frame, enum AVPixelFormat in_format,
                                 unsigned char ** out_frame, int out_format,
                                 int width, int height, int row_span, int row_span_uv)
  {
#ifdef HAVE_LIBSWSCALE
  uint8_t * out_planes[4];
  int out_strides[4];
#else
  AVPicture in_pic;
  AVPicture out_pic;
#endif
  
  /*
   *  Could someone please tell me, how people can make such a brain dead
   *  RGBA format like in ffmpeg??
   */
#if LIBAVUTIL_VERSION_INT < (50<<16)
  if((in_format == AV_PIX_FMT_RGBA32) && (out_format == BC_RGBA8888))
#else
    if((in_format == AV_PIX_FMT_RGB32) && (out_format == BC_RGBA8888))
#endif
      {
      convert_image_decode_rgba(in_frame, out_frame, width, height, codec->y_offset);
      return;
      }

#ifdef HAVE_LIBSWSCALE
  out_planes[0] = out_frame[0];
  out_planes[1] = out_frame[1];
  out_planes[2] = out_frame[2];
  out_planes[3] = (uint8_t*)0;

  out_strides[0] = row_span;
  out_strides[1] = row_span_uv;
  out_strides[2] = row_span_uv;
  out_strides[3] = 0;
  
  sws_scale(codec->swsContext,
            (const uint8_t * const*)in_frame->data, in_frame->linesize,
            codec->y_offset, height, out_planes, out_strides);
#else
  
  memset(&in_pic,  0, sizeof(in_pic));
  memset(&out_pic, 0, sizeof(out_pic));
  
  in_pic.data[0]      = in_frame->data[0] + codec->y_offset * in_frame->linesize[0];
  in_pic.data[1]      = in_frame->data[1] + codec->y_offset * in_frame->linesize[1];
  in_pic.data[2]      = in_frame->data[2] + codec->y_offset * in_frame->linesize[2];
  in_pic.linesize[0]  = in_frame->linesize[0];
  in_pic.linesize[1]  = in_frame->linesize[1];
  in_pic.linesize[2]  = in_frame->linesize[2];
  
  fill_avpicture(&out_pic, out_frame, out_format, row_span, row_span_uv);
  img_convert(&out_pic, lqt_ffmpeg_get_ffmpeg_colormodel(out_format),
              &in_pic,  in_format,
              width, height);
#endif
  
  }

/* Stuff for reading compressed H.264 */

#define PTR_2_16BE(p) \
((*(p) << 8) | \
*(p+1))

#define PTR_2_32BE(p) \
((*(p) << 24) | \
(*(p+1) << 16) | \
(*(p+2) << 8) | \
*(p+3))


static const uint8_t nal_header[4] = { 0x00, 0x00, 0x00, 0x01 };

static void append_h264_header(lqt_compression_info_t * ci,
                               uint8_t * data, int len)
  {
  ci->global_header = realloc(ci->global_header, ci->global_header_len + len + 4);
  memcpy(ci->global_header + ci->global_header_len, nal_header, 4);
  ci->global_header_len += 4;
  memcpy(ci->global_header + ci->global_header_len, data, len);
  ci->global_header_len += len;
  }
                               

static void set_h264_header(quicktime_video_map_t * vtrack,
                            uint8_t * extradata, int extradata_size)
  {
  int i, len;
  int num_units;
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  
  uint8_t * ptr;
  ptr = extradata;

  ptr += 4; // Version, profile, profile compat, level
  codec->nal_size_length = (*ptr & 0x3) + 1;
  ptr++;

  /* SPS */
  num_units = *ptr & 0x1f; ptr++;
  for(i = 0; i < num_units; i++)
    {
    len = PTR_2_16BE(ptr); ptr += 2;
    append_h264_header(&vtrack->ci, ptr, len);
    ptr += len;
    }

  /* PPS */
  num_units = *ptr; ptr++;
  for(i = 0; i < num_units; i++)
    {
    len = PTR_2_16BE(ptr); ptr += 2;
    append_h264_header(&vtrack->ci, ptr, len);
    ptr += len;
    }

  }

static void lqt_ffmpeg_imx_setup_decoding_frame(quicktime_t *file, int track)
{
    /* Note that this function may be called more than once. */

    quicktime_video_map_t *vtrack = &file->vtracks[track];
    quicktime_trak_t *trak = vtrack->track;
    quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;

    if (codec->imx_strip_vbi) {
        codec->y_offset = codec->avctx->height - trak->tkhd.track_height;
        vtrack->height_extension = 0;
    } else {
        codec->y_offset = 0;
        if (vtrack->height_extension == codec->avctx->height - trak->tkhd.track_height) {
            return;
        }

        vtrack->height_extension = codec->avctx->height - trak->tkhd.track_height;

        /* Now we need a larger temp_frame */
        if (vtrack->temp_frame) {
            lqt_rows_free(vtrack->temp_frame);
        }

        vtrack->temp_frame = lqt_rows_alloc(
                codec->avctx->width, codec->avctx->height,
                vtrack->stream_cmodel,
                &vtrack->stream_row_span,
                &vtrack->stream_row_span_uv);
    }
}

/* Just for the curious: This function can be called with NULL as row_pointers.
   In this case, have_frame is set to 1 and a subsequent call will take the
   already decoded frame. This madness is necessary because sometimes ffmpeg
   doesn't tells us the true colormodel before decoding the first frame */

static int lqt_ffmpeg_decode_video(quicktime_t *file, unsigned char **row_pointers,
                                   int track)
  {
  uint8_t * user_atom;
  uint32_t user_atom_len;
  int i, imax;
  int result = 0;
  int buffer_size;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  int height;
  int width;
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  int got_pic;
  //  int do_cmodel_transfer;
  quicktime_ctab_t * ctab;
  int exact = 0;
  
  uint8_t * extradata = (uint8_t*)0;
  int extradata_size = 0;
  
  uint8_t * cpy_rows[3];
  
  height = quicktime_video_height(file, track);
  width =  quicktime_video_width(file, track);
  
  /* Initialize decoder */
  
  if(!codec->initialized)
    {
    codec->avctx->width           = width;
    codec->avctx->height          = height;

#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
    codec->avctx->bits_per_sample = quicktime_video_depth(file, track);
#else
    codec->avctx->bits_per_coded_sample = quicktime_video_depth(file, track);
#endif

    /* Set extradata: It's done differently for each codec */

    if(codec->decoder->id == AV_CODEC_ID_SVQ3)
      {
      extradata       = trak->mdia.minf.stbl.stsd.table[0].table_raw + 4;
      extradata_size  = trak->mdia.minf.stbl.stsd.table[0].table_raw_size - 4;
      
      }
    else if(codec->decoder->id == AV_CODEC_ID_H264)
      {
      user_atom = quicktime_stsd_get_user_atom(trak, "avcC", &user_atom_len);

      if(!user_atom)
        {
        if(!IS_AVI(file->file_type))
          lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
                  "No avcC atom present, decoding is likely to fail");
        }
      else
        {
        extradata = user_atom + 8;
        extradata_size = user_atom_len - 8;
        vtrack->ci.id = LQT_COMPRESSION_H264;
        set_h264_header(vtrack, extradata, extradata_size);
        }
      
      }
    else if(codec->decoder->id == AV_CODEC_ID_MPEG4)
      {
      if(trak->mdia.minf.stbl.stsd.table[0].has_esds)
        {
        extradata = trak->mdia.minf.stbl.stsd.table[0].esds.decoderConfig;
        extradata_size =
          trak->mdia.minf.stbl.stsd.table[0].esds.decoderConfigLen;

        if(quicktime_match_32(vtrack->track->mdia.minf.stbl.stsd.table[0].format,
                              "mp4v"))
          {
          vtrack->ci.id = LQT_COMPRESSION_MPEG4_ASP;
          lqt_compression_info_set_header(&vtrack->ci,
                                          extradata,
                                          extradata_size);
          }
        }
      }
    else if((user_atom =
             quicktime_stsd_get_user_atom(trak, "glbl", &user_atom_len)))
      {
      extradata = user_atom + 8;
      extradata_size = user_atom_len - 8;
      }
    
    if(extradata)
      {
      codec->extradata =
        calloc(1, extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
      memcpy(codec->extradata, extradata, extradata_size);
      codec->avctx->extradata_size = extradata_size;
      codec->avctx->extradata = codec->extradata;
      }
    /* Add palette info */
    
    ctab = &trak->mdia.minf.stbl.stsd.table->ctab;
#if LIBAVCODEC_VERSION_MAJOR < 54
    if(ctab->size)
      {
      codec->avctx->palctrl = &codec->palette;
      codec->palette.palette_changed = 1;
      imax =
        (ctab->size > AVPALETTE_COUNT)
        ? AVPALETTE_COUNT : ctab->size;

      for(i = 0; i < imax; i++)
        {
        codec->palette.palette[i] =
          ((ctab->alpha[i] >> 8) << 24) |
          ((ctab->red[i] >> 8) << 16) |
          ((ctab->green[i] >> 8) << 8) |
          ((ctab->blue[i] >> 8));
        }
      }
#else
    /* Palette will be sent later */
    if(!ctab->size)
      codec->palette_sent = 1;
#endif
    //    codec->avctx->get_buffer = avcodec_default_get_buffer;
    //    codec->avctx->release_buffer = avcodec_default_release_buffer;

    codec->avctx->codec_id = codec->decoder->id;
    codec->avctx->codec_type = codec->decoder->type;

#if LIBAVCODEC_VERSION_INT < ((52<<16)+(112<<8)+0)
    avcodec_thread_init(codec->avctx, codec->avctx->thread_count);
#endif
    
#if LIBAVCODEC_VERSION_MAJOR < 54
    if(avcodec_open(codec->avctx, codec->decoder) != 0)
      return -1;
#else
    if(avcodec_open2(codec->avctx, codec->decoder, NULL) != 0)
      return -1;
#endif
    codec->frame = av_frame_alloc();
    vtrack->stream_cmodel = LQT_COLORMODEL_NONE;
    codec->initialized = 1;
    }
  
  /* Read the frame from file and decode it */

  got_pic = 0;
  
  if(!codec->have_frame)
    {
    while(!got_pic)
      {
      buffer_size = lqt_read_video_frame(file, &codec->buffer,
                                         &codec->buffer_alloc,
                                         vtrack->current_position + codec->decoding_delay,
                                         NULL, track);

      codec->decoding_delay++;

#if LIBAVCODEC_BUILD >= ((52<<16)+(26<<8)+0)
      codec->pkt.data = codec->buffer;
      codec->pkt.size = buffer_size;

#if LIBAVCODEC_VERSION_MAJOR >= 54
      if(!codec->palette_sent)
        {
        uint32_t * pal_i;
        ctab = &trak->mdia.minf.stbl.stsd.table->ctab;
        pal_i =
          (uint32_t*)av_packet_new_side_data(&codec->pkt, AV_PKT_DATA_PALETTE,
                                             AVPALETTE_COUNT * 4);
        imax =
          (ctab->size > AVPALETTE_COUNT)
            ? AVPALETTE_COUNT : ctab->size;

        for(i = 0; i < imax; i++)
          {
          pal_i[i] =
            ((ctab->alpha[i] >> 8) << 24) |
            ((ctab->red[i] >> 8) << 16) |
            ((ctab->green[i] >> 8) << 8) |
            ((ctab->blue[i] >> 8));
          }
        for(i = imax; i < AVPALETTE_COUNT; i++)
          pal_i[i] = 0;
        }
#endif
      
      if(avcodec_decode_video2(codec->avctx,
                               codec->frame,
                               &got_pic,
                               &codec->pkt) < 0)
        {
        lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Skipping corrupted frame");
        continue;
        }

#if LIBAVCODEC_VERSION_MAJOR >= 54
      /* Ugly hack: Need to free the side data elements manually because
         ffmpeg has no public API for that */
      if(!codec->palette_sent)
        {
        av_free(codec->pkt.side_data[0].data);
        av_freep(&codec->pkt.side_data);
        codec->pkt.side_data_elems = 0;
        }
#endif
      

#else
    
      if(avcodec_decode_video(codec->avctx,
                              codec->frame,
                              &got_pic,
                              codec->buffer,
                              buffer_size) < 0)
        {
        lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Skipping corrupted frame");
        continue;
        }
#endif      
      if(got_pic)
        codec->decoding_delay--;
      
      if((buffer_size <= 0) && !got_pic)
        return 0;
      }
    }
  
  if(vtrack->stream_cmodel == LQT_COLORMODEL_NONE)
    {
    lqt_ffmpeg_setup_decoding_colormodel(file, vtrack, &exact);
    if(!exact)
      {
      codec->do_imgconvert = 1;
      
#ifdef HAVE_LIBSWSCALE

#if LIBAVUTIL_VERSION_INT < (50<<16)
      if(!((codec->avctx->pix_fmt == AV_PIX_FMT_RGBA32) &&
           (vtrack->stream_cmodel == BC_RGBA8888)))
#else
        if(!((codec->avctx->pix_fmt == AV_PIX_FMT_RGB32) &&
             (vtrack->stream_cmodel == BC_RGBA8888)))
#endif
          {
          codec->swsContext =
            sws_getContext(width, height + vtrack->height_extension,
                           codec->avctx->pix_fmt,
                           width, height + vtrack->height_extension,
                           lqt_ffmpeg_get_ffmpeg_colormodel(vtrack->stream_cmodel),
                           SWS_FAST_BILINEAR, (SwsFilter*)0,
                           (SwsFilter*)0,
                           (double*)0);
          }
#endif
      }
    if(codec->decoder->id == AV_CODEC_ID_DVVIDEO)
      {
      if(vtrack->stream_cmodel == BC_YUV420P)
        vtrack->chroma_placement = LQT_CHROMA_PLACEMENT_DVPAL;
      vtrack->interlace_mode = LQT_INTERLACE_BOTTOM_FIRST;
      vtrack->ci.id = LQT_COMPRESSION_DV;
      }
    else if((codec->decoder->id == AV_CODEC_ID_MPEG4) ||
            (codec->decoder->id == AV_CODEC_ID_H264))
      {
      if(vtrack->stream_cmodel == BC_YUV420P)
        vtrack->chroma_placement = LQT_CHROMA_PLACEMENT_MPEG2;
      }
    else if(codec->is_imx)
      {
      /* Note that at this point in time, set_parameter_video() is yet to be called,
         and therefore we don't yet know the "imx_strip_vbi" value.  It's better to
         initially assume it's on and let lqt_ffmpeg_imx_setup_decoding_frame() act
         accordingly.  Later, set_parameter_video() will call
         lqt_ffmpeg_imx_setup_decoding_frame() again. */
      codec->imx_strip_vbi = 1;
      lqt_ffmpeg_imx_setup_decoding_frame(file, track);

      vtrack->chroma_placement = LQT_CHROMA_PLACEMENT_MPEG2;
      vtrack->ci.id = LQT_COMPRESSION_D10;
      vtrack->ci.bitrate = 
        (trak->mdia.minf.stbl.stsd.table[0].format[2] - '0') *
        10000000;
      }
    
    if(codec->avctx->sample_aspect_ratio.num)
      {
      trak->mdia.minf.stbl.stsd.table[0].pasp.hSpacing = codec->avctx->sample_aspect_ratio.num;
      trak->mdia.minf.stbl.stsd.table[0].pasp.vSpacing = codec->avctx->sample_aspect_ratio.den;
      }
    }
  
  if(!row_pointers)
    {
    codec->have_frame = 1;
    return 1;
    }

  /*
   * Check for the colormodel
   * There are 2 possible cases:
   *
   *  1. The decoded frame can be memcopied directly to row_pointers
   *     this is the (most likely) case, when the colormodel of
   *     the decoder is supported by lqt
   *
   *  2. The decoder colormodel is not supported by libquicktime,
   *     (e.g. YUV410P for sorenson). We must then use avcodec's
   *     image conversion routines to convert to row_pointers.
   */
  
  if(!codec->do_imgconvert)
    {
    cpy_rows[0] = codec->frame->data[0];
    cpy_rows[1] = codec->frame->data[1];
    cpy_rows[2] = codec->frame->data[2];
    
    lqt_rows_copy_sub(row_pointers, cpy_rows, width, height + vtrack->height_extension,
                      codec->frame->linesize[0], codec->frame->linesize[1],
                      vtrack->stream_row_span, vtrack->stream_row_span_uv,
                      vtrack->stream_cmodel,
                      0,               // src_x
                      codec->y_offset, // src_y
                      0,               // dst_x
                      0                // dst_y
                      );
    }
  else
    {
    convert_image_decode(codec, codec->frame, codec->reinterpret_pix_fmt,
                         row_pointers, vtrack->stream_cmodel,
                         width, height + vtrack->height_extension,
                         vtrack->stream_row_span, vtrack->stream_row_span_uv);
    }
  codec->have_frame = 0;
  return result;
  }

static void resync_ffmpeg(quicktime_t *file, int track)
  {
  int64_t keyframe, frame;
  int buffer_size, got_pic;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  
  /* Forget about previously decoded frame */
  codec->have_frame = 0;
  codec->decoding_delay = 0;
  
  /* Reset lavc */
  avcodec_flush_buffers(codec->avctx);

  
  if(quicktime_has_keyframes(file, track))
    {
    keyframe = quicktime_get_keyframe_before(file, vtrack->current_position, track);

    frame = keyframe;
    
    while(frame < vtrack->current_position)
      {
      buffer_size = lqt_read_video_frame(file, &codec->buffer,
                                         &codec->buffer_alloc,
                                         frame + codec->decoding_delay,
                                         NULL, track);
      if(buffer_size > 0)
        {
#if LIBAVCODEC_BUILD >= ((52<<16)+(26<<8)+0)
        codec->pkt.data = codec->buffer;
        codec->pkt.size = buffer_size;
        avcodec_decode_video2(codec->avctx,
                              codec->frame,
                              &got_pic,
                              &codec->pkt);
#else
        avcodec_decode_video(codec->avctx,
                             codec->frame,
                             &got_pic,
                             codec->buffer,
                             buffer_size);
#endif
        if(!got_pic)
          {
          codec->decoding_delay++;
          frame--;
          }
        }
      frame++;
      }
    }
  }

static int set_pass_ffmpeg(quicktime_t *file, 
                           int track, int pass, int total_passes,
                           const char * stats_file)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;

  codec->total_passes = total_passes;
  codec->pass = pass;
  codec->stats_filename = malloc(strlen(stats_file)+1);
  strcpy(codec->stats_filename, stats_file);
  
  return 1;
  }

static void set_imx_fourcc(quicktime_t *file, int track, int bitrate,
                          int height)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  char *compressor = trak->mdia.minf.stbl.stsd.table[0].format;

  compressor[0] = 'm';
  compressor[1] = 'x';

  switch(bitrate)
    {
    case 30000000:
      compressor[2] = '3';
      break;
    case 40000000:
      compressor[2] = '4';
      break;
    case 50000000:
      compressor[2] = '5';
      break;
    }
  if((height == 512) || (height == 486))
    compressor[3] = 'n';
  else
    compressor[3] = 'p';
  
  }

static int init_imx_encoder(quicktime_t *file, int track)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  int height = trak->tkhd.track_height;
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  
  codec->avctx->gop_size = 0;
  codec->avctx->intra_dc_precision = 2;
  codec->avctx->qmin = 1;
  codec->avctx->qmax = 3;
  codec->avctx->rtp_payload_size = 1; // ??
  av_dict_set(&codec->options, "rc_buf_aggressivity", "0.25", 0);
  codec->avctx->flags |= AV_CODEC_FLAG_INTERLACED_DCT|AV_CODEC_FLAG_LOW_DELAY;

#if (LIBAVCODEC_VERSION_MAJOR < 54)
  codec->avctx->flags2 |= CODEC_FLAG2_INTRA_VLC|CODEC_FLAG2_NON_LINEAR_QUANT;
#else
  av_dict_set(&codec->options, "non_linear_quant", "1", 0);
  av_dict_set(&codec->options, "intra_vlc", "1", 0);
#endif
  
  codec->avctx->bit_rate = codec->imx_bitrate * 1000000;
  
  codec->avctx->rc_max_rate = codec->avctx->bit_rate;
  codec->avctx->rc_min_rate = codec->avctx->bit_rate;
  codec->avctx->rc_buffer_size = codec->avctx->bit_rate / 25;
  codec->avctx->rc_initial_buffer_occupancy = codec->avctx->rc_buffer_size;

  set_imx_fourcc(file, track, codec->avctx->bit_rate, height);
  
  /* Go through all formats */
  switch(height)
    {
    case 576: // PAL without VBI
      codec->avctx->height = 608;
      codec->y_offset = 32;
      break;
    case 608: // PAL with VBI
      trak->tkhd.track_height = 576;
      trak->mdia.minf.stbl.stsd.table[0].height = 576;
      vtrack->height_extension = 32;
      break;
    case 486: // NTSC without VBI
      codec->avctx->height = 512;
      codec->y_offset = 26;
      break;
    case 512: // NTSC with VBI
      trak->tkhd.track_height = 486;
      trak->mdia.minf.stbl.stsd.table[0].height = 486;
      vtrack->height_extension = 26;
      break;
    }

  return 0;
  }

static void setup_header_mpeg4(quicktime_t *file, int track,
                               const uint8_t * header, int header_len,
                               int advanced)
  {
  quicktime_esds_t * esds;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  
  esds = quicktime_set_esds(trak, header, header_len);
  
  esds->version         = 0;
  esds->flags           = 0;
  
  esds->esid            = 0;
  esds->stream_priority = 0;
  
  esds->objectTypeId    = 32; /* MPEG-4 video */
  esds->streamType      = 0x11; /* from qt4l and CDR_Dinner_350k-994.mp4 */
  esds->bufferSizeDB    = 64000; /* Hopefully not important :) */
  
  /* Maybe correct these later? */
  esds->maxBitrate      = 200000;
  esds->avgBitrate      = 200000;
  
  /* Set iods profile */
  if(advanced)
    file->moov.iods.videoProfileId = 0xf3; // Advanced Simple Profile @ Level 3
  else
    file->moov.iods.videoProfileId = 0x03; // Simple Profile @ Level 3
  
  }

static void setup_avid_atoms(quicktime_t* file,
                             quicktime_video_map_t *vtrack,
                             uint8_t const* enc_data, int enc_data_size)
  {
  uint8_t* p;
  uint8_t ACLR_atom[16];
  uint8_t APRG_atom[16];
  uint8_t ARES_atom[112];
  int full_range_yuv = (vtrack->stream_cmodel == BC_YUVJ422P ||
                        vtrack->stream_cmodel == BC_YUVJ422P10);

  if (enc_data_size < 0x2c)
    {
    lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN, "Unexpected stream data from DNxHD!");
    return;
    }

  p = ACLR_atom;
  lqt_ffmpeg_write_tag(&p, "ACLR");
  lqt_ffmpeg_write_tag(&p, "0001");
  lqt_ffmpeg_write_be32(&p, full_range_yuv ? AVID_FULL_YUV_RANGE : AVID_NORMAL_YUV_RANGE);
  lqt_ffmpeg_write_be32(&p, 0); /* unknown */
  quicktime_stsd_set_user_atom(vtrack->track, "ACLR", ACLR_atom, sizeof(ACLR_atom));

  p = APRG_atom;
  lqt_ffmpeg_write_tag(&p, "APRG");
  lqt_ffmpeg_write_tag(&p, "0001");
  lqt_ffmpeg_write_be32(&p, 2); /* FFMpeg puts 1 here, but on all files I've seen it's 2. */
  lqt_ffmpeg_write_be32(&p, 0); /* unknown */
  quicktime_stsd_set_user_atom(vtrack->track, "APRG", APRG_atom, sizeof(APRG_atom));

  p = ARES_atom;
  lqt_ffmpeg_write_tag(&p, "ARES");
  lqt_ffmpeg_write_tag(&p, "0001");
  lqt_ffmpeg_write_be32(&p, lqt_ffmpeg_read_be32(enc_data + 0x28)); /* cid */
  lqt_ffmpeg_write_be32(&p, vtrack->track->tkhd.track_width);
  /* values below are based on samples created with quicktime and avid codecs */
  if (enc_data[5] & 2)
    { /* interlaced */
    lqt_ffmpeg_write_be32(&p, vtrack->track->tkhd.track_height/2);
    lqt_ffmpeg_write_be32(&p, 2); /* unknown */
    lqt_ffmpeg_write_be32(&p, 0); /* unknown */
    lqt_ffmpeg_write_be32(&p, 4); /* unknown */
    }
  else
    { /* progressive */
    lqt_ffmpeg_write_be32(&p, vtrack->track->tkhd.track_height);
    lqt_ffmpeg_write_be32(&p, 1); /* unknown */
    lqt_ffmpeg_write_be32(&p, 0); /* unknown */
    if (vtrack->track->tkhd.track_height == 1080)
      lqt_ffmpeg_write_be32(&p, 5); /* unknown */
    else
      lqt_ffmpeg_write_be32(&p, 6); /* unknown */
    }
  memset(p, 0, 80); /* Fill the rest with zeros. */
  quicktime_stsd_set_user_atom(vtrack->track, "ARES", ARES_atom, sizeof(ARES_atom));
  }

static int lqt_ffmpeg_encode_video(quicktime_t *file, unsigned char **row_pointers,
                                   int track)
  {
  int result = 0;
  int pixel_width, pixel_height;
  int bytes_encoded;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_trak_t *trak = vtrack->track;
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  int was_initialized = codec->initialized;
  int height = trak->tkhd.track_height;
  int width = trak->tkhd.track_width;
  int stats_len;
#if ENCODE_VIDEO2
  AVPacket pkt;
  int got_packet;
#endif
  int64_t pts;
  int kf;
  
  if(!row_pointers)
    {
    if(vtrack->stream_cmodel == BC_YUV420P)
      {
      if(codec->encoder->id == AV_CODEC_ID_MPEG4)
        {
        vtrack->chroma_placement = LQT_CHROMA_PLACEMENT_MPEG2;
        /* enable interlaced encoding */
        vtrack->interlace_mode = LQT_INTERLACE_NONE;
        }
      else if(codec->encoder->id == AV_CODEC_ID_DVVIDEO)
        {
        vtrack->chroma_placement = LQT_CHROMA_PLACEMENT_DVPAL;
        }
      else
        /* enable interlaced encoding */
        vtrack->interlace_mode = LQT_INTERLACE_NONE;
      }
    return 0;
    }
        
  if(!codec->initialized)
    {
    codec->frame = av_frame_alloc();

    /* time_base is 1/framerate for constant framerate */
          
    codec->avctx->time_base.den = lqt_video_time_scale(file, track);
    codec->avctx->time_base.num = lqt_frame_duration(file, track, NULL);

    //          codec->avctx->time_base.den = 1;
    //          codec->avctx->time_base.num = lqt_video_time_scale(file, track);

    if(codec->avctx->flags & AV_CODEC_FLAG_QSCALE)
      codec->avctx->global_quality = codec->qscale;
                              
    codec->avctx->width = width;
    codec->avctx->height = height;

    lqt_ffmpeg_setup_encoding_colormodel(vtrack);

    lqt_get_pixel_aspect(file, track, &pixel_width, &pixel_height);
    codec->avctx->sample_aspect_ratio.num = pixel_width;
    codec->avctx->sample_aspect_ratio.den = pixel_height;
    /* Use global headers for mp4v */
    if(codec->encoder->id == AV_CODEC_ID_MPEG4)
      {
      if(!(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML)))
        {
        codec->avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        codec->write_global_header = 1;
        }
            
      /* Tweak some stream headers so even bad players play this */
      else
        {
        strncpy(trak->strl->strh.fccHandler, "divx", 4);
        strncpy(trak->strl->strf.bh.biCompression, "DX50", 4);
        }
#ifdef DO_INTERLACE // Seems not to work yet
      /* enable interlaced encoding */
      if(vtrack->interlace_mode != LQT_INTERLACE_NONE)
        {
        lqt_log(file, LQT_LOG_INFO, LOG_DOMAIN, "Enabling interlaced encoding");
        codec->avctx->flags |=
          (AV_CODEC_FLAG_INTERLACED_DCT|AV_CODEC_FLAG_INTERLACED_ME|AV_CODEC_FLAG_ALT_SCAN);
        }
#endif
      }
    else if((codec->encoder->id == AV_CODEC_ID_MSMPEG4V3) && (trak->strl) &&
            !strncmp(trak->strl->strf.bh.biCompression, "DIV3", 4))
      {
      strncpy(trak->strl->strh.fccHandler, "div3", 4);
      }
    else if((codec->encoder->id == AV_CODEC_ID_H263) &&
            (file->file_type & (LQT_FILE_MP4|LQT_FILE_3GP)))
      {
      uint8_t d263_data[] =
        { 'l', 'q', 't', ' ', /* Vendor? */
          0, /* Decoder version */
          0xa, /* Level */
          0 /* Profile */ };
      quicktime_user_atoms_add_atom(&trak->mdia.minf.stbl.stsd.table[0].user_atoms,
                                    "d263", d263_data,
                                    sizeof(d263_data));
      strncpy(trak->mdia.minf.stbl.stsd.table[0].format,
              "s263", 4);
      }
    else if(codec->encoder->id == AV_CODEC_ID_FFVHUFF)
      {
      if(!(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML)))
        {
        codec->avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        codec->write_global_header = 1;
        }
      }
    else if(codec->encoder->id == AV_CODEC_ID_QTRLE)
      {
      if(vtrack->stream_cmodel == BC_RGBA8888)
        {
        /* Libquicktime doesn't natively support a color model equivalent
           to AV_PIX_FMT_ARGB, which is required for QTRLE with alpha channel.
           So, we use BC_RGBA8888 and do ad hoc conversion below. */
        codec->avctx->pix_fmt = AV_PIX_FMT_ARGB;
        vtrack->track->mdia.minf.stbl.stsd.table[0].depth = 32;
        }
      }
    else if(codec->encoder->id == AV_CODEC_ID_DVVIDEO)
      {
      set_dv_fourcc(width, height, vtrack->stream_cmodel, trak);
      }
    else if(codec->encoder->id == AV_CODEC_ID_DNXHD)
      {
      if(vtrack->interlace_mode != LQT_INTERLACE_NONE)
        {
        codec->avctx->flags |= AV_CODEC_FLAG_INTERLACED_DCT;
        }
      }
    else if(codec->is_imx)
      init_imx_encoder(file, track);
    
    /* Initialize 2-pass */
    if(codec->total_passes)
      {
      if(codec->pass == 1)
        {
        codec->stats_file = fopen(codec->stats_filename, "w");
        codec->avctx->flags |= AV_CODEC_FLAG_PASS1;
        }
      else if(codec->pass == codec->total_passes)
        {
        codec->stats_file = fopen(codec->stats_filename, "r");
        fseek(codec->stats_file, 0, SEEK_END);
        stats_len = ftell(codec->stats_file);
        fseek(codec->stats_file, 0, SEEK_SET);

        codec->avctx->stats_in = av_malloc(stats_len + 1);
        fread(codec->avctx->stats_in, stats_len, 1, codec->stats_file);
        codec->avctx->stats_in[stats_len] = '\0';

        fclose(codec->stats_file);
        codec->stats_file = (FILE*)0;
              
        codec->avctx->flags |= AV_CODEC_FLAG_PASS2;
        }
      }
    /* Open codec */

    codec->avctx->codec_id = codec->decoder->id;
    codec->avctx->codec_type = codec->decoder->type;

#if LIBAVCODEC_VERSION_INT < ((52<<16)+(112<<8)+0)
    avcodec_thread_init(codec->avctx, codec->avctx->thread_count);
#endif

#if LIBAVCODEC_VERSION_MAJOR < 54
    if(avcodec_open(codec->avctx, codec->encoder) != 0)
      return -1;
#else
    if(avcodec_open2(codec->avctx, codec->encoder, &codec->options) != 0)
      return -1;
#endif
    codec->buffer_alloc = codec->avctx->width * codec->avctx->height * 4 + 1024*256;
    codec->buffer = malloc(codec->buffer_alloc);
    if(!codec->buffer)
      return -1;
          
    codec->initialized = 1;
          
    }
  //        codec->lqt_colormodel = ffmepg_2_lqt(codec->com.ffcodec_enc);

  if(codec->y_offset != 0 || codec->avctx->pix_fmt == AV_PIX_FMT_ARGB)
    {
    if(!codec->tmp_rows)
      {
      /* Allocate temp buffer */
      codec->tmp_rows =
        lqt_rows_alloc(codec->avctx->width, codec->avctx->height,
                       vtrack->stream_cmodel, &codec->tmp_row_span, &codec->tmp_row_span_uv);
      /* Make it black */
      lqt_rows_clear(codec->tmp_rows,
                     codec->avctx->width, codec->avctx->height,
                     codec->tmp_row_span, codec->tmp_row_span_uv, vtrack->stream_cmodel);
      }

    if(codec->y_offset)
      {
      lqt_rows_copy_sub(codec->tmp_rows, row_pointers,
                        width, height + vtrack->height_extension,
                        vtrack->stream_row_span,
                        vtrack->stream_row_span_uv,
                        codec->tmp_row_span,
                        codec->tmp_row_span_uv,
                        vtrack->stream_cmodel,
                        0, 0, 0, codec->y_offset);
      }
    else if(codec->avctx->pix_fmt == AV_PIX_FMT_ARGB)
      {
      convert_rgba_to_argb(row_pointers[0], vtrack->stream_row_span,
                           codec->tmp_rows[0], codec->tmp_row_span,
                           codec->avctx->height, codec->avctx->width);
      }
    codec->frame->data[0] = codec->tmp_rows[0];
    codec->frame->data[1] = codec->tmp_rows[1];
    codec->frame->data[2] = codec->tmp_rows[2];
    codec->frame->linesize[0] = codec->tmp_row_span;
    codec->frame->linesize[1] = codec->tmp_row_span_uv;
    codec->frame->linesize[2] = codec->tmp_row_span_uv;
    }
  else
    {
    codec->frame->data[0] = row_pointers[0];
    codec->frame->data[1] = row_pointers[1];
    codec->frame->data[2] = row_pointers[2];
    codec->frame->linesize[0] = vtrack->stream_row_span;
    codec->frame->linesize[1] = vtrack->stream_row_span_uv;
    codec->frame->linesize[2] = vtrack->stream_row_span_uv;
    }
  
  codec->frame->pts = vtrack->timestamp;
  if(codec->avctx->flags & AV_CODEC_FLAG_QSCALE)
    codec->frame->quality = codec->qscale;
#ifdef DO_INTERLACE
  if(vtrack->interlace_mode != LQT_INTERLACE_NONE)
    {
    codec->frame->interlaced_frame = 1;
    if(vtrack->interlace_mode == LQT_INTERLACE_TOP_FIRST)
      codec->frame->top_field_first = 1;
    }
#endif

#if ENCODE_VIDEO2 // New
  av_init_packet(&pkt);
  pkt.data = codec->buffer;
  pkt.size = codec->buffer_alloc;

  if(avcodec_encode_video2(codec->avctx, &pkt, codec->frame, &got_packet) < 0)
    return -1;

  if(got_packet)
    bytes_encoded = pkt.size;
  else
    bytes_encoded = 0;
  
  pts = pkt.pts;
  kf = !!(pkt.flags & AV_PKT_FLAG_KEY);
    
#else // Old
  
  bytes_encoded = avcodec_encode_video(codec->avctx,
                                       codec->buffer,
                                       codec->buffer_alloc,
                                       codec->frame);
  
  if(bytes_encoded < 0)
    return -1;
  
  pts = codec->avctx->coded_frame->pts;
  kf = codec->avctx->coded_frame->key_frame;
  
#endif
  
  if(!was_initialized && codec->encoder->id == AV_CODEC_ID_DNXHD)
    setup_avid_atoms(file, vtrack, codec->buffer, bytes_encoded);
  
  if(bytes_encoded)
    {
    if (pts == AV_NOPTS_VALUE || (codec->encoder->id == AV_CODEC_ID_DNXHD && pts == 0))
      {
      /* Some codecs don't bother generating presentation timestamps.
         FFMpeg's DNxHD encoder doesn't even bother to set it to AV_NOPTS_VALUE. */
      pts = vtrack->timestamp;
      }

    lqt_write_frame_header(file, track,
                           -1, (int)pts,
                           kf);
          
    result = !quicktime_write_data(file, 
                                   codec->buffer, 
                                   bytes_encoded);

    lqt_write_frame_footer(file, track);
          
    /* Write stats */
          
    if((codec->pass == 1) && codec->avctx->stats_out && codec->stats_file)
      fprintf(codec->stats_file, "%s", codec->avctx->stats_out);
    }
        
  /* Check whether to write the global header */

  if(codec->write_global_header && !codec->global_header_written)
    {
    if(codec->encoder->id == AV_CODEC_ID_FFVHUFF)
      {
      quicktime_user_atoms_add_atom(&trak->mdia.minf.stbl.stsd.table[0].user_atoms,
                                    "glbl",
                                    codec->avctx->extradata, codec->avctx->extradata_size );
      }
    else if(codec->encoder->id == AV_CODEC_ID_MPEG4)
      {
      int advanced = 0;
      if(codec->avctx->max_b_frames)
        advanced = 1;

      setup_header_mpeg4(file, track, codec->avctx->extradata,
                         codec->avctx->extradata_size, advanced);
      }
    codec->global_header_written = 1;
    }
  return result;
  }

static int flush(quicktime_t *file, int track)
  {
  int result = 0;
  int bytes_encoded;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  int64_t pts;
  int kf;
  
#if ENCODE_VIDEO2
  AVPacket pkt;
  int got_packet;
#endif
  
  /* Do nothing if we didn't encode anything yet */
  if(!codec->initialized)
    return 0;

#if ENCODE_VIDEO2
  av_init_packet(&pkt);
  pkt.data = codec->buffer;
  pkt.size = codec->buffer_alloc;
  
  if(avcodec_encode_video2(codec->avctx, &pkt, (AVFrame*)0, &got_packet) < 0)
    return -1;

  if(got_packet)
    bytes_encoded = pkt.size;
  else
    return 0;
  
  pts = pkt.pts;

  kf = !!(pkt.flags & AV_PKT_FLAG_KEY);
  
#else
  
  bytes_encoded = avcodec_encode_video(codec->avctx,
                                       codec->buffer,
                                       codec->buffer_alloc,
                                       (AVFrame*)0);
  if(bytes_encoded <= 0)
    return 0;

  pts = codec->avctx->coded_frame->pts;
  kf = codec->avctx->coded_frame->key_frame;
  
#endif
  
  if(bytes_encoded)
    {
    lqt_write_frame_header(file, track,
                           -1, pts, kf);
    
    result = !quicktime_write_data(file, 
                                   codec->buffer, 
                                   bytes_encoded);

    lqt_write_frame_footer(file, track);
          
    if((codec->pass == 1) && codec->avctx->stats_out && codec->stats_file)
      fprintf(codec->stats_file, "%s", codec->avctx->stats_out);
          
    return 1;
    }
  return 0;
        
  }

static int set_parameter_video(quicktime_t *file, 
                               int track, 
                               const char *key, 
                               const void *value)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  if(!strcasecmp(key, "ff_qscale"))
    {
    codec->qscale = *(int*)(value) * FF_QP2LAMBDA;
    return 0;
    }
  else if(!strcasecmp(key, "imx_bitrate"))
    {
    codec->imx_bitrate = atoi(value);
    return 0;
    }
  else if(!strcasecmp(key, "imx_strip_vbi"))
    {
    codec->imx_strip_vbi = *(int*)value;
    if (codec->is_imx && file->rd)
      {
      lqt_ffmpeg_imx_setup_decoding_frame(file, track);
      }
    return 0;
    }
  
  lqt_ffmpeg_set_parameter(codec->avctx,
#if LIBAVCODEC_VERSION_MAJOR >= 54
                           &codec->options,
#endif
                           key, value);



  return 0;
  }

static int writes_compressed_mpeg4(lqt_file_type_t type,
                                   const lqt_compression_info_t * ci)
  {
  /* AVI doesn't support B-frames */
  if(type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML))
    {
    if(ci->flags & LQT_COMPRESSION_HAS_B_FRAMES)
      return 0;
    }
  /* Quicktime/mov needs global header */
  if(type & (LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4 |
             LQT_FILE_M4A | LQT_FILE_3GP))
    
    {
    if(!ci->global_header_len)
      return 0;
    }
  return 1;
  }

static int init_compressed_mpeg4(quicktime_t * file, int track)
  {
  setup_header_mpeg4(file, track,
                     file->vtracks[track].ci.global_header,
                     file->vtracks[track].ci.global_header_len, 1);
  return 0;
  }

static int write_packet_mpeg4(quicktime_t * file, lqt_packet_t * p, int track)
  {
  int result;
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  
  /* Prepend header to keyframe */
  if(file->file_type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML)) 
    {
    if(p->flags & LQT_PACKET_KEYFRAME)
      result = !quicktime_write_data(file, vtrack->ci.global_header,
                                     vtrack->ci.global_header_len);

    if(!vtrack->current_position)
      {
      strncpy(vtrack->track->strl->strh.fccHandler, "divx", 4);
      strncpy(vtrack->track->strl->strf.bh.biCompression, "DX50", 4);
      }
    }
  
  result = !quicktime_write_data(file, p->data, p->data_len);
  return result;
  }

static int init_compressed_imx(quicktime_t * file, int track)
  {
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  
  set_imx_fourcc(file, track, vtrack->ci.bitrate,
                 vtrack->ci.height);
  return 0;
  }

static int writes_compressed_imx(lqt_file_type_t type,
                                 const lqt_compression_info_t * ci)
  {
  /* AVI doesn't support IMX */
  if(type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML))
    return 0;

  switch(ci->bitrate)
    {
    case 30000000:
    case 40000000:
    case 50000000:
      return 1;
      break;
    }
  return 0;
  }


static void append_data_h264(lqt_packet_t * p, uint8_t * data, int len,
                             int header_len)
  {
  switch(header_len)
    {
    case 3:
      lqt_packet_alloc(p, p->data_len + 3 + len);
      memcpy(p->data + p->data_len, &nal_header[1], 3);
      p->data_len += 3;
      break;
    case 4:
      lqt_packet_alloc(p, p->data_len + 4 + len);
      memcpy(p->data + p->data_len, nal_header, 4);
      p->data_len += 4;
      break;
    }
  memcpy(p->data + p->data_len, data, len);
  p->data_len += len;
  }

static int read_packet_h264(quicktime_t * file, lqt_packet_t * p, int track)
  {
  int nals_sent = 0;
  uint8_t * ptr, *end;
  int len;
  int buffer_size;
  
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  quicktime_ffmpeg_video_codec_t *codec = vtrack->codec->priv;
  
  if(!(buffer_size = lqt_read_video_frame(file, &codec->buffer,
                                          &codec->buffer_alloc,
                                          vtrack->current_position, NULL, track)))
    return 0;
  
  ptr = codec->buffer;
  end = codec->buffer + buffer_size;
  
  p->data_len = 0;
  
  while(ptr < end - codec->nal_size_length)
    {
    switch(codec->nal_size_length)
      {
      case 1:
        len = *ptr;
        ptr++;
        break;
      case 2:
        len = PTR_2_16BE(ptr);
        ptr += 2;
        break;
      case 4:
        len = PTR_2_32BE(ptr);
        ptr += 4;
        break;
      default:
        break;
      }
    append_data_h264(p, ptr, len, nals_sent ? 3 : 4);
    nals_sent++;
    ptr += len;
    }
  return 1;
  }

static int init_compressed_dv(quicktime_t * file, int track)
  {
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  set_dv_fourcc(vtrack->ci.width, vtrack->ci.height, vtrack->ci.colormodel,
                vtrack->track);
  return 0;
  }

void quicktime_init_video_codec_ffmpeg(quicktime_codec_t * codec_base,
                                       quicktime_video_map_t *vtrack,
                                       AVCodec *encoder,
                                       AVCodec *decoder)
  {
  quicktime_ffmpeg_video_codec_t *codec;
  char *compressor;
  quicktime_stsd_t * stsd;

  printf("quicktime_init_video_codec_ffmpeg\n");
  // avcodec_init();

  codec = calloc(1, sizeof(*codec));
  if(!codec)
  {
    printf("quicktime_init_video_codec_ffmpeg: calloc failed\n");

    return;
  }
#if LIBAVCODEC_VERSION_INT < ((53<<16)|(8<<8)|0)
  codec->avctx = avcodec_alloc_context();
#else
  codec->avctx = avcodec_alloc_context3(NULL);
#endif
  codec->encoder = encoder;
  codec->decoder = decoder;
  
  codec_base->priv = codec;
  codec_base->delete_codec = lqt_ffmpeg_delete_video;
  codec_base->flush = flush;
  codec_base->resync = resync_ffmpeg;
  
  if(encoder)
    {
    codec_base->encode_video = lqt_ffmpeg_encode_video;
    codec_base->set_pass = set_pass_ffmpeg;

    if(encoder->id == AV_CODEC_ID_MPEG4)
      {
      codec_base->writes_compressed = writes_compressed_mpeg4;
      codec_base->init_compressed   = init_compressed_mpeg4;
      codec_base->write_packet = write_packet_mpeg4;
      }
    else if(encoder->id == AV_CODEC_ID_MPEG2VIDEO)
      {
      codec_base->writes_compressed = writes_compressed_imx;
      codec_base->init_compressed   = init_compressed_imx;
      }
    else if(encoder->id == AV_CODEC_ID_DVVIDEO)
      {
      codec_base->init_compressed = init_compressed_dv;
      }
    
    }
  if(decoder)
    {
    if(decoder->id == AV_CODEC_ID_H264)
      codec_base->read_packet = read_packet_h264;
    codec_base->decode_video = lqt_ffmpeg_decode_video;
    } else {
        printf("quicktime_init_video_codec_ffmpeg: no decoder!\n");

    }
  codec_base->set_parameter = set_parameter_video;

  if(!vtrack)
  {
    printf("quicktime_init_video_codec_ffmpeg: no vtrack!\n");

    return;
  }

  stsd = &vtrack->track->mdia.minf.stbl.stsd;
  
  compressor = stsd->table[0].format;
  
  if (quicktime_match_32(compressor, "dvc "))
    {
    if(stsd->table[0].height == 480)
      vtrack->stream_cmodel = BC_YUV411P;
    else
      vtrack->stream_cmodel = BC_YUV420P;
    }
  else if (quicktime_match_32(compressor, "dvpp"))
    vtrack->stream_cmodel = BC_YUV411P;
  else if (quicktime_match_32(compressor, "dv5n") ||
           quicktime_match_32(compressor, "dv5p") ||
           quicktime_match_32(compressor, "AVdn"))
    vtrack->stream_cmodel = BC_YUV422P;
  else if (quicktime_match_32(compressor, "MJPG"))
    vtrack->stream_cmodel = BC_YUVJ420P;
  else if (quicktime_match_32(compressor, "rle "))
    vtrack->stream_cmodel = BC_RGB888;
  //        else if(quicktime_match_32(compressor, "dvcp"))
  //          codec->encode_colormodel = BC_YUV411P;
  else if(quicktime_match_32(compressor, "mx3p") ||
          quicktime_match_32(compressor, "mx4p") ||
          quicktime_match_32(compressor, "mx5p") ||
          quicktime_match_32(compressor, "mx3n") ||
          quicktime_match_32(compressor, "mx4n") ||
          quicktime_match_32(compressor, "mx5n"))
    {
    vtrack->stream_cmodel = BC_YUV422P;
    codec->is_imx = 1;
    }
  else
    vtrack->stream_cmodel = BC_YUV420P;
  
  }


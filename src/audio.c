/*******************************************************************************
 audio.c

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

#define LOG_DOMAIN "audio"

/***************************************************
 * Audio conversion functions
 ***************************************************/

#define RECLIP(val, min, max) (val<min?min:((val>max)?max:val))

/* Conversion Macros */

/* int8 -> */

#define INT8_TO_INT16(src, dst) dst = src * 0x0101

#define INT8_TO_FLOAT(src, dst) dst = (float)src / 128.0

/* uint8 -> */

#define UINT8_TO_INT16(src, dst) dst = (src - 128) * 0x0101

#define UINT8_TO_FLOAT(src, dst) dst = (float)src / 127.0 - 1.0

/* int16 -> */

#define INT16_TO_INT8(src, dst) dst = src >> 8

#define INT16_TO_UINT8(src, dst) dst = (src ^ 0x8000) >> 8

#define INT16_TO_INT16(src, dst) dst = src

#define INT16_TO_INT32(src, dst) dst = src * 0x00010001

#define INT16_TO_FLOAT(src, dst) dst = (float)src / 32767.0f

#define INT16_TO_DOUBLE(src, dst) dst = (double)src / 32767.0

/* int32 -> */

#define INT32_TO_INT16(src, dst) dst = src >> 16

#define INT32_TO_FLOAT(src, dst) dst = (float)src / 2147483647.0

/* float -> */

#define FLOAT_TO_INT8(src, dst) tmp = (int)(src * 127.0); dst = RECLIP(tmp, -128, 127)

#define FLOAT_TO_UINT8(src, dst) tmp = (int)((src + 1.0) * 127.0); dst = RECLIP(tmp, 0, 255)

#define FLOAT_TO_INT16(src, dst) tmp = (int)((src) * 32767.0); dst = RECLIP(tmp, -32768, 32767)

#define FLOAT_TO_INT32(src, dst) tmp = (int64_t)((src) * 2147483647.0); dst = RECLIP(tmp, -2147483648LL, 2147483647LL)

#define FLOAT_TO_FLOAT(src, dst) dst = src

#define FLOAT_TO_DOUBLE(src, dst) dst = src


#define DOUBLE_TO_INT16(src, dst) tmp = (int)((src) * 32767.0); dst = RECLIP(tmp, -32768, 32767)

#define DOUBLE_TO_FLOAT(src, dst) dst = src


/* Encoding */

static void encode_int16_to_int8(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  int8_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int8_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_INT8(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_int16_to_uint8(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  uint8_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((uint8_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_UINT8(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_int16_to_int16(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  int16_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int16_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_INT16(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_int16_to_int32(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  int32_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int32_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_INT32(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_int16_to_float(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  float * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((float*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_FLOAT(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_int16_to_double(int16_t ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  double * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((double*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      INT16_TO_DOUBLE(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }


static void encode_float_to_int8(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j, tmp;
  int8_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int8_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_INT8(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_float_to_uint8(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j, tmp;
  uint8_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((uint8_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_UINT8(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_float_to_int16(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j, tmp;
  int16_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int16_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_INT16(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_float_to_int32(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  int64_t tmp;
  int32_t * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((int32_t*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_INT32(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_float_to_float(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  float * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((float*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_FLOAT(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

static void encode_float_to_double(float ** in, void * _out, int num_channels, int num_samples)
  {
  int i, j;
  double * out;
  for(i = 0; i < num_channels; i++)
    {
    out = ((double*)_out) + i;
    for(j = 0; j < num_samples; j++)
      {
      FLOAT_TO_DOUBLE(in[i][j], (*out));
      out+=num_channels;
      }
    }
  }

void lqt_convert_audio_encode(quicktime_t * file, int16_t ** in_int, float ** in_float, void * out,
                              int num_channels, int num_samples,
                              lqt_sample_format_t stream_format)
  {
  switch(stream_format)
    {
    case LQT_SAMPLE_INT8:
      if(in_int)
        encode_int16_to_int8(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_int8(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_UINT8:
      if(in_int)
        encode_int16_to_uint8(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_uint8(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_INT16:
      if(in_int)
        encode_int16_to_int16(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_int16(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_INT32:
      if(in_int)
        encode_int16_to_int32(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_int32(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_FLOAT:
      if(in_int)
        encode_int16_to_float(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_float(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_DOUBLE:
      if(in_int)
        encode_int16_to_double(in_int, out, num_channels, num_samples);
      else if(in_float)
        encode_float_to_double(in_float, out, num_channels, num_samples);
      break;
    case LQT_SAMPLE_UNDEFINED:
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot encode samples: Stream format undefined");
      break;
    }
  }

/* Decoding */

static void decode_int8_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j;
  int8_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int8_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT8_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_uint8_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j;
  uint8_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((uint8_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        UINT8_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_int16_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j;
  int16_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int16_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT16_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_int32_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j;
  int32_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int32_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT32_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_float_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j, tmp;
  float * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((float*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        FLOAT_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_double_to_int16(void * _in, int16_t ** out, int num_channels, int num_samples)
  {
  int i, j, tmp;
  double * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((double*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        DOUBLE_TO_INT16((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_int8_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  int8_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int8_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT8_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_uint8_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  uint8_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((uint8_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        UINT8_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_int16_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  int16_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int16_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT16_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_int32_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  int32_t * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((int32_t*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        INT32_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_float_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  float * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((float*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        FLOAT_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

static void decode_double_to_float(void * _in, float ** out, int num_channels, int num_samples)
  {
  int i, j;
  double * in;
  for(i = 0; i < num_channels; i++)
    {
    if(out[i])
      {
      in = ((double*)_in) + i;
      for(j = 0; j < num_samples; j++)
        {
        DOUBLE_TO_FLOAT((*in), out[i][j]);
        in += num_channels;
        }
      }
    }
  }

void lqt_convert_audio_decode(quicktime_t * file,
                              void * in, int16_t ** out_int, float ** out_float,
                              int num_channels, int num_samples,
                              lqt_sample_format_t stream_format)
  {
  switch(stream_format)
    {
    case LQT_SAMPLE_INT8:
      if(out_int)
        decode_int8_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_int8_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_UINT8:
      if(out_int)
        decode_uint8_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_uint8_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_INT16:
      if(out_int)
        decode_int16_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_int16_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_INT32:
      if(out_int)
        decode_int32_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_int32_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_FLOAT: /* Float is ALWAYS machine native */
      if(out_int)
        decode_float_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_float_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_DOUBLE: /* Float is ALWAYS machine native */
      if(out_int)
        decode_double_to_int16(in, out_int, num_channels, num_samples);
      if(out_float)
        decode_double_to_float(in, out_float, num_channels, num_samples);
      break;
    case LQT_SAMPLE_UNDEFINED:
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot decode samples: Stream format undefined");
      break;
    }
  }

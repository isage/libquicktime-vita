/*******************************************************************************
 pcm.c

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
#include "ulaw_tables.h"
#include "alaw_tables.h"
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "audiocodec.h"

#define LOG_DOMAIN "pcm"

#ifndef HAVE_LRINT
#define lrint(x) ((long int)(x))
#endif

typedef enum
  {
    FORMAT_INT_16,
    FORMAT_INT_24,
    FORMAT_INT_32,
    FORMAT_FLOAT_32,
    FORMAT_FLOAT_64,
  } format_t;

typedef struct quicktime_pcm_codec_s
  {
  uint8_t * chunk_buffer;
  uint8_t * chunk_buffer_ptr;
  int chunk_buffer_size;
  int chunk_buffer_alloc;
  
  int sample_buffer_size;
  int last_chunk_samples;

  void (*encode)(struct quicktime_pcm_codec_s*, int num_samples, void * input);
  void (*decode)(struct quicktime_pcm_codec_s*, int num_samples, void ** output);

  void (*init_encode)(quicktime_t * file, int track);
  void (*init_decode)(quicktime_t * file, int track);

  int initialized;

  /* Encoding parameters for lpcm */
  format_t format;
  int little_endian;

  lqt_compression_id_t cid;
  
  } quicktime_pcm_codec_t;

/* 8 bit per sample, signedness and endian neutral */

static void encode_8(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  memcpy(codec->chunk_buffer_ptr, _input, num_samples);
  }

static void decode_8(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  uint8_t * output = (uint8_t *)(*_output);

  memcpy(output, codec->chunk_buffer_ptr, num_samples);
  codec->chunk_buffer_ptr += num_samples;

  output += num_samples;
  *_output = output;
  }

/* 16 bit per sample, without swapping */

static void encode_s16(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  memcpy(codec->chunk_buffer_ptr, _input, 2 * num_samples);
  }

static void decode_s16(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  uint8_t * output = (uint8_t *)(*_output);

  memcpy(output, codec->chunk_buffer_ptr, 2 * num_samples);
  codec->chunk_buffer_ptr += 2 * num_samples;
  
  output += 2 * num_samples;
  *_output = output;
  }

/* 16 bit per sample with swapping */

static void encode_s16_swap(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  uint8_t * input = (uint8_t*)_input;

  for(i = 0; i < num_samples; i++)
    {
    codec->chunk_buffer_ptr[0] = input[1];
    codec->chunk_buffer_ptr[1] = input[0];
    codec->chunk_buffer_ptr+=2;
    input+=2;
    }
  }

static void decode_s16_swap(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  uint8_t * output = (uint8_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    output[0] = codec->chunk_buffer_ptr[1];
    output[1] = codec->chunk_buffer_ptr[0];
    codec->chunk_buffer_ptr+=2;
    output+=2;
    }
  *_output = output;
  }

/* 24 bit per sample (Big Endian) */

static void encode_s24_be(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  uint32_t * input = (uint32_t*)_input;

  for(i = 0; i < num_samples; i++)
    {
    codec->chunk_buffer_ptr[0] = (*input & 0xff000000) >> 24;
    codec->chunk_buffer_ptr[1] = (*input & 0xff0000) >> 16;
    codec->chunk_buffer_ptr[2] = (*input & 0xff00) >> 8;
    codec->chunk_buffer_ptr+=3;
    input++;
    }
  
  }

static void decode_s24_be(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  uint32_t * output = (uint32_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output  = (uint32_t)(codec->chunk_buffer_ptr[0]) << 24;
    *output |= (uint32_t)(codec->chunk_buffer_ptr[1]) << 16;
    *output |= (uint32_t)(codec->chunk_buffer_ptr[2]) <<  8;
    codec->chunk_buffer_ptr+=3;
    output++;
    }
  *_output = output;
  }

/* 24 bit per sample (Little Endian) */

static void encode_s24_le(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  uint32_t * input = (uint32_t*)_input;

  for(i = 0; i < num_samples; i++)
    {
    codec->chunk_buffer_ptr[2] = (*input & 0xff000000) >> 24;
    codec->chunk_buffer_ptr[1] = (*input & 0xff0000) >> 16;
    codec->chunk_buffer_ptr[0] = (*input & 0xff00) >> 8;
    codec->chunk_buffer_ptr+=3;
    input++;
    }

  }

static void decode_s24_le(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  uint32_t * output = (uint32_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output  = (uint32_t)(codec->chunk_buffer_ptr[2]) << 24;
    *output |= (uint32_t)(codec->chunk_buffer_ptr[1]) << 16;
    *output |= (uint32_t)(codec->chunk_buffer_ptr[0]) <<  8;
    codec->chunk_buffer_ptr+=3;
    output++;
    }
  *_output = output;
  }


/* 32 bit per sample, without swapping */

static void encode_s32(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  memcpy(codec->chunk_buffer_ptr, _input, 4 * num_samples);
  }

static void decode_s32(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  uint8_t * output = (uint8_t *)(*_output);

  memcpy(output, codec->chunk_buffer_ptr, 4 * num_samples);
  codec->chunk_buffer_ptr += 4 * num_samples;
  
  output += 4 * num_samples;
  *_output = output;
  }

/* 32 bit per sample with swapping */

static void encode_s32_swap(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  uint8_t * input = (uint8_t*)_input;

  for(i = 0; i < num_samples; i++)
    {
    codec->chunk_buffer_ptr[0] = input[3];
    codec->chunk_buffer_ptr[1] = input[2];
    codec->chunk_buffer_ptr[2] = input[1];
    codec->chunk_buffer_ptr[3] = input[0];
    codec->chunk_buffer_ptr+=4;
    input+=4;
    }
  }

static void decode_s32_swap(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  uint8_t * output = (uint8_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    output[0] = codec->chunk_buffer_ptr[3];
    output[1] = codec->chunk_buffer_ptr[2];
    output[2] = codec->chunk_buffer_ptr[1];
    output[3] = codec->chunk_buffer_ptr[0];
    codec->chunk_buffer_ptr+=4;
    output+=4;
    }
  *_output = output;
  }

/* Floating point formats */

/* Sample read/write functions, taken from libsndfile */

static float
float32_be_read (unsigned char *cptr)
{       int             exponent, mantissa, negative ;
        float   fvalue ;

        negative = cptr [0] & 0x80 ;
        exponent = ((cptr [0] & 0x7F) << 1) | ((cptr [1] & 0x80) ? 1 : 0) ;
        mantissa = ((cptr [1] & 0x7F) << 16) | (cptr [2] << 8) | (cptr [3]) ;

        if (! (exponent || mantissa))
                return 0.0 ;

        mantissa |= 0x800000 ;
        exponent = exponent ? exponent - 127 : 0 ;

        fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

        if (negative)
                fvalue *= -1 ;

        if (exponent > 0)
                fvalue *= (1 << exponent) ;
        else if (exponent < 0)
                fvalue /= (1 << abs (exponent)) ;

        return fvalue ;
} /* float32_be_read */

static float
float32_le_read (unsigned char *cptr)
{       int             exponent, mantissa, negative ;
        float   fvalue ;

        negative = cptr [3] & 0x80 ;
        exponent = ((cptr [3] & 0x7F) << 1) | ((cptr [2] & 0x80) ? 1 : 0) ;
        mantissa = ((cptr [2] & 0x7F) << 16) | (cptr [1] << 8) | (cptr [0]) ;

        if (! (exponent || mantissa))
                return 0.0 ;

        mantissa |= 0x800000 ;
        exponent = exponent ? exponent - 127 : 0 ;

        fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

        if (negative)
                fvalue *= -1 ;

        if (exponent > 0)
                fvalue *= (1 << exponent) ;
        else if (exponent < 0)
                fvalue /= (1 << abs (exponent)) ;

        return fvalue ;
} /* float32_le_read */

static double
double64_be_read (unsigned char *cptr)
{       int             exponent, negative ;
        double  dvalue ;

        negative = (cptr [0] & 0x80) ? 1 : 0 ;
        exponent = ((cptr [0] & 0x7F) << 4) | ((cptr [1] >> 4) & 0xF) ;

        /* Might not have a 64 bit long, so load the mantissa into a double. */
        dvalue = (((cptr [1] & 0xF) << 24) | (cptr [2] << 16) | (cptr [3] << 8) | cptr [4]) ;
        dvalue += ((cptr [5] << 16) | (cptr [6] << 8) | cptr [7]) / ((double) 0x1000000) ;

        if (exponent == 0 && dvalue == 0.0)
                return 0.0 ;

        dvalue += 0x10000000 ;

        exponent = exponent - 0x3FF ;

        dvalue = dvalue / ((double) 0x10000000) ;

        if (negative)
                dvalue *= -1 ;

        if (exponent > 0)
                dvalue *= (1 << exponent) ;
        else if (exponent < 0)
                dvalue /= (1 << abs (exponent)) ;

        return dvalue ;
} /* double64_be_read */

static double
double64_le_read (unsigned char *cptr)
{       int             exponent, negative ;
        double  dvalue ;

        negative = (cptr [7] & 0x80) ? 1 : 0 ;
        exponent = ((cptr [7] & 0x7F) << 4) | ((cptr [6] >> 4) & 0xF) ;

        /* Might not have a 64 bit long, so load the mantissa into a double. */
        dvalue = (((cptr [6] & 0xF) << 24) | (cptr [5] << 16) | (cptr [4] << 8) | cptr [3]) ;
        dvalue += ((cptr [2] << 16) | (cptr [1] << 8) | cptr [0]) / ((double) 0x1000000) ;

        if (exponent == 0 && dvalue == 0.0)
                return 0.0 ;

        dvalue += 0x10000000 ;

        exponent = exponent - 0x3FF ;

        dvalue = dvalue / ((double) 0x10000000) ;

        if (negative)
                dvalue *= -1 ;

        if (exponent > 0)
                dvalue *= (1 << exponent) ;
        else if (exponent < 0)
                dvalue /= (1 << abs (exponent)) ;

        return dvalue ;
} /* double64_le_read */

static void
float32_le_write (float in, unsigned char *out)
{       int             exponent, mantissa, negative = 0 ;

        memset (out, 0, sizeof (int)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                negative = 1 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 126 ;

        in *= (float) 0x1000000 ;
        mantissa = (((int) in) & 0x7FFFFF) ;

        if (negative)
                out [3] |= 0x80 ;

        if (exponent & 0x01)
                out [2] |= 0x80 ;

        out [0] = mantissa & 0xFF ;
        out [1] = (mantissa >> 8) & 0xFF ;
        out [2] |= (mantissa >> 16) & 0x7F ;
        out [3] |= (exponent >> 1) & 0x7F ;

        return ;
} /* float32_le_write */

static void
float32_be_write (float in, unsigned char *out)
{       int             exponent, mantissa, negative = 0 ;

        memset (out, 0, sizeof (int)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                negative = 1 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 126 ;

        in *= (float) 0x1000000 ;
        mantissa = (((int) in) & 0x7FFFFF) ;

        if (negative)
                out [0] |= 0x80 ;

        if (exponent & 0x01)
                out [1] |= 0x80 ;

        out [3] = mantissa & 0xFF ;
        out [2] = (mantissa >> 8) & 0xFF ;
        out [1] |= (mantissa >> 16) & 0x7F ;
        out [0] |= (exponent >> 1) & 0x7F ;

        return ;
} /* float32_be_write */


static void
double64_be_write (double in, unsigned char *out)
{       int             exponent, mantissa ;

        memset (out, 0, sizeof (double)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                out [0] |= 0x80 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 1022 ;

        out [0] |= (exponent >> 4) & 0x7F ;
        out [1] |= (exponent << 4) & 0xF0 ;

        in *= 0x20000000 ;
        mantissa = lrint (floor (in)) ;

        out [1] |= (mantissa >> 24) & 0xF ;
        out [2] = (mantissa >> 16) & 0xFF ;
        out [3] = (mantissa >> 8) & 0xFF ;
        out [4] = mantissa & 0xFF ;

        in = fmod (in, 1.0) ;
        in *= 0x1000000 ;
        mantissa = lrint (floor (in)) ;

        out [5] = (mantissa >> 16) & 0xFF ;
        out [6] = (mantissa >> 8) & 0xFF ;
        out [7] = mantissa & 0xFF ;

        return ;
} /* double64_be_write */

static void
double64_le_write (double in, unsigned char *out)
{       int             exponent, mantissa ;

        memset (out, 0, sizeof (double)) ;

        if (in == 0.0)
                return ;

        if (in < 0.0)
        {       in *= -1.0 ;
                out [7] |= 0x80 ;
                } ;

        in = frexp (in, &exponent) ;

        exponent += 1022 ;

        out [7] |= (exponent >> 4) & 0x7F ;
        out [6] |= (exponent << 4) & 0xF0 ;

        in *= 0x20000000 ;
        mantissa = lrint (floor (in)) ;

        out [6] |= (mantissa >> 24) & 0xF ;
        out [5] = (mantissa >> 16) & 0xFF ;
        out [4] = (mantissa >> 8) & 0xFF ;
        out [3] = mantissa & 0xFF ;

        in = fmod (in, 1.0) ;
        in *= 0x1000000 ;
        mantissa = lrint (floor (in)) ;

        out [2] = (mantissa >> 16) & 0xFF ;
        out [1] = (mantissa >> 8) & 0xFF ;
        out [0] = mantissa & 0xFF ;

        return ;
} /* double64_le_write */

/* 32 bit float (Big Endian) */

static void encode_fl32_be(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  float * input = (float*)_input;

  for(i = 0; i < num_samples; i++)
    {
    float32_be_write(*input, codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=4;
    input++;
    }
  
  }

static void decode_fl32_be(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  float * output = (float*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output = float32_be_read(codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=4;
    output++;
    }
  *_output = output;
  }

/* 32 bit float (Little Endian) */

static void encode_fl32_le(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  float * input = (float*)_input;

  for(i = 0; i < num_samples; i++)
    {
    float32_le_write(*input, codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=4;
    input++;
    }

  }

static void decode_fl32_le(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  float * output = (float*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output = float32_le_read(codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=4;
    output++;
    }
  *_output = output;
  }

/* 64 bit float (Big Endian) */

static void encode_fl64_be(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  double * input = (double*)_input;

  for(i = 0; i < num_samples; i++)
    {
    double64_be_write(*input, codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=8;
    input++;
    }
  
  }

static void decode_fl64_be(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  double * output = (double*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output = double64_be_read(codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=8;
    output++;
    }
  *_output = output;
  }

/* 64 bit float (Little Endian) */

static void encode_fl64_le(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  double * input = (double*)_input;

  for(i = 0; i < num_samples; i++)
    {
    double64_le_write(*input, codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=8;
    input++;
    }

  }

static void decode_fl64_le(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  double * output = (double*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    *output = double64_le_read(codec->chunk_buffer_ptr);
    codec->chunk_buffer_ptr+=8;
    output++;
    }
  *_output = output;
  }



/* ulaw */

/* See ulaw_tables.h for the tables references here */

#define ENCODE_ULAW(src, dst) if(src >= 0) dst = ulaw_encode[src / 4]; else dst = 0x7F & ulaw_encode[src / -4] 
#define DECODE_ULAW(src, dst) dst = ulaw_decode [src]

static void encode_ulaw(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  int16_t * input = (int16_t*)_input;
  
  for(i = 0; i < num_samples; i++)
    {
    ENCODE_ULAW(input[0], codec->chunk_buffer_ptr[0]);
    codec->chunk_buffer_ptr++;
    input++;
    }
  }

static void decode_ulaw(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  int16_t * output = (int16_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    DECODE_ULAW(codec->chunk_buffer_ptr[0], output[0]);
    codec->chunk_buffer_ptr++;
    output++;
    }
  *_output = output;
  }


/* alaw */

/* See alaw_tables.h for the tables references here */

#define ENCODE_ALAW(src, dst) if(src >= 0) dst = alaw_encode[src / 16]; else dst = 0x7F & alaw_encode[src / -16] 
#define DECODE_ALAW(src, dst) dst = alaw_decode [src]

static void encode_alaw(quicktime_pcm_codec_t*codec, int num_samples, void * _input)
  {
  int i;
  int16_t * input = (int16_t*)_input;
  
  for(i = 0; i < num_samples; i++)
    {
    ENCODE_ALAW(input[0], codec->chunk_buffer_ptr[0]);
    codec->chunk_buffer_ptr++;
    input++;
    }
  }

static void decode_alaw(quicktime_pcm_codec_t*codec, int num_samples, void ** _output)
  {
  int i;
  /* The uint32_t is intentional: Interpreting integers as unsigned has less pitfalls */
  int16_t * output = (int16_t*)(*_output);
  
  for(i = 0; i < num_samples; i++)
    {
    DECODE_ALAW(codec->chunk_buffer_ptr[0], output[0]);
    codec->chunk_buffer_ptr++;
    output++;
    }
  *_output = output;
  }


/* Generic decode function */

static int read_audio_chunk(quicktime_t * file, int track,
                            long chunk,
                            uint8_t ** buffer, int * buffer_alloc)
  {
  int bytes, samples = 0, bytes_from_samples;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  
  bytes = lqt_read_audio_chunk(file, track, chunk, buffer, buffer_alloc, &samples);

  bytes_from_samples = samples * atrack->block_align;
  if(bytes > bytes_from_samples)
    return bytes_from_samples;
  else
    return bytes;
  }

static int decode_pcm(quicktime_t *file, void * _output, long samples, int track)
  {
  int64_t chunk, chunk_sample;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  void * output;
  int64_t samples_to_skip = 0;
  int samples_in_chunk;
  int samples_decoded, samples_to_decode;
    
  if(!codec->initialized)
    {
    if(codec->init_decode)
      codec->init_decode(file, track);
    
    /* Read the first audio chunk */

    codec->chunk_buffer_size = read_audio_chunk(file,
                                                track, atrack->cur_chunk,
                                                &codec->chunk_buffer,
                                                &codec->chunk_buffer_alloc);
    if(codec->chunk_buffer_size <= 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "EOF at the beginning of track");
      return 0;
      }
    codec->chunk_buffer_ptr = codec->chunk_buffer;
    codec->initialized = 1;

    atrack->ci.id = codec->cid;
    
    }

  if(!_output) /* Global initialization */
    {
    return 0;
    }
  
  if(atrack->current_position != atrack->last_position)
    {
    /* Seeking happened */
    quicktime_chunk_of_sample(&chunk_sample, &chunk,
                              atrack->track,
                              atrack->current_position);

    /* Read the chunk */
    
    if(atrack->cur_chunk != chunk)
      {
      atrack->cur_chunk = chunk;
      codec->chunk_buffer_size = read_audio_chunk(file,
                                                  track, atrack->cur_chunk,
                                                  &codec->chunk_buffer,
                                                  &codec->chunk_buffer_alloc);
      if(codec->chunk_buffer_size <= 0)
        return 0;
      
      codec->chunk_buffer_ptr = codec->chunk_buffer;
      }
    else
      {
      codec->chunk_buffer_ptr = codec->chunk_buffer;
      }
    
    /* Skip samples */
    
    samples_to_skip = atrack->current_position - chunk_sample;
    if(samples_to_skip < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot skip backwards");
      samples_to_skip = 0;
      }
    codec->chunk_buffer_ptr = codec->chunk_buffer + samples_to_skip * atrack->block_align;
    }

  samples_decoded = 0;

  output = _output;
  
  while(samples_decoded < samples)
    {
    /* Get new chunk if necessary */
    if(codec->chunk_buffer_ptr - codec->chunk_buffer >= codec->chunk_buffer_size)
      {
      atrack->cur_chunk++;
      codec->chunk_buffer_size = read_audio_chunk(file,
                                                  track, atrack->cur_chunk,
                                                  &codec->chunk_buffer,
                                                  &codec->chunk_buffer_alloc);
      if(codec->chunk_buffer_size <= 0)
        break;
      codec->chunk_buffer_ptr = codec->chunk_buffer;
      }

    /* Decode */

    samples_in_chunk = ((codec->chunk_buffer_size-(int)(codec->chunk_buffer_ptr - codec->chunk_buffer)))
      /atrack->block_align;
    
    samples_to_decode = samples - samples_decoded;
    
    if(samples_to_decode > samples_in_chunk)
      samples_to_decode = samples_in_chunk;

    if(!samples_to_decode) // EOF
      break;

    codec->decode(codec, samples_to_decode * atrack->channels, &output);
    samples_decoded += samples_to_decode;
    }
  atrack->last_position = atrack->current_position + samples_decoded;
  return samples_decoded;
  }

/* Generic encode function */

static int encode_pcm(quicktime_t *file, void * input, long samples, int track)
  {
  int result;
  
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_trak_t *trak = atrack->track;

  
  /* Initialize */

  if(!codec->initialized)
    {
    /* Initialize AVI header */
    
    if(trak->strl)
      {
      /* strh stuff */
      trak->strl->strh.dwRate = atrack->samplerate;
      trak->strl->strh.dwScale = 1;
      trak->strl->strh.dwSampleSize = atrack->block_align / atrack->channels;
      
      /* WAVEFORMATEX stuff */
      
      trak->strl->strf.wf.f.WAVEFORMAT.nBlockAlign = atrack->block_align;
      trak->strl->strf.wf.f.WAVEFORMAT.nAvgBytesPerSec = atrack->block_align * atrack->samplerate;
      trak->strl->strf.wf.f.PCMWAVEFORMAT.wBitsPerSample = trak->strl->strh.dwSampleSize * 8;
      }


    
    if(codec->init_encode)
      codec->init_encode(file, track);
    codec->initialized = 1;
    }
  if(!input || !samples)
    return 0;
  
  /* Allocate chunk buffer */

  if(codec->chunk_buffer_alloc < samples * atrack->block_align)
    {
    codec->chunk_buffer_alloc = samples * atrack->block_align + 1024;
    codec->chunk_buffer = realloc(codec->chunk_buffer, codec->chunk_buffer_alloc);
    }

  codec->chunk_buffer_ptr = codec->chunk_buffer;
  codec->encode(codec, samples * atrack->channels, input);

  quicktime_write_chunk_header(file, trak);
  result = quicktime_write_data(file, codec->chunk_buffer,
                                samples * atrack->block_align);
  trak->chunk_samples = samples;
  
  quicktime_write_chunk_footer(file, trak);
  atrack->cur_chunk++;		
  
  /* defeat fwrite's return */
  if(result) 
    result = 0; 
  else 
    result = 1; 
  
  
  return result;
  }

/* Set parameters */

static int set_parameter_pcm(quicktime_t *file, 
                             int track, 
                             const char *key, 
                             const void *value)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;

  if(!strcasecmp(key, "pcm_little_endian"))
    {
    codec->little_endian = *((int*)(value));
    }
  else if(!strcasecmp(key, "pcm_format"))
    {
    if(!strcasecmp((char*)(value), "Integer (16 bit)"))
      codec->format = FORMAT_INT_16;
    else if(!strcasecmp((char*)(value), "Integer (24 bit)"))
      codec->format = FORMAT_INT_24;
    else if(!strcasecmp((char*)(value), "Integer (32 bit)"))
      codec->format = FORMAT_INT_32;
    else if(!strcasecmp((char*)(value), "Float (32 bit)"))
      codec->format = FORMAT_FLOAT_32;
    else if(!strcasecmp((char*)(value), "Float (64 bit)"))
      codec->format = FORMAT_FLOAT_64;
    }
  return 0;
  }

/* Destructor */

static int delete_pcm(quicktime_codec_t *codec_base)
  {
  quicktime_pcm_codec_t *codec = codec_base->priv;
  if(codec->chunk_buffer)
    {
    free(codec->chunk_buffer);
    }
  free(codec);
  return 0;
  }


void quicktime_init_codec_twos(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  
  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;

  if(!atrack)
    return;
  
  switch(atrack->track->mdia.minf.stbl.stsd.table[0].sample_size)
    {
    case 8:
      atrack->block_align = atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT8;
      codec->encode = encode_8;
      codec->decode = decode_8;
      break;
    case 16:
      atrack->block_align = 2 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT16;
#ifdef WORDS_BIGENDIAN
      codec->encode = encode_s16;
      codec->decode = decode_s16;
#else
      codec->encode = encode_s16_swap;
      codec->decode = decode_s16_swap;
#endif
      break;
    case 24:
      atrack->block_align = 3 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT32;
      codec->encode = encode_s24_be;
      codec->decode = decode_s24_be;
      break;
    }
  }


void quicktime_init_codec_sowt(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  //  codec_base->wav_id = 0x01;

  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;

  if(!atrack)
    return;
  
  switch(atrack->track->mdia.minf.stbl.stsd.table[0].sample_size)
    {
    case 8:
      atrack->block_align = atrack->channels;
      atrack->sample_format = LQT_SAMPLE_UINT8;
      codec->encode = encode_8;
      codec->decode = decode_8;
      break;
    case 16:
      atrack->block_align = 2 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT16;
#ifdef WORDS_BIGENDIAN
      codec->encode = encode_s16_swap;
      codec->decode = decode_s16_swap;
#else
      codec->encode = encode_s16;
      codec->decode = decode_s16;
#endif
      break;
    case 24:
      atrack->block_align = 3 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT32;
      codec->encode = encode_s24_le;
      codec->decode = decode_s24_le;
      break;
    }
  }

static void init_encode_in24(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];
  
  /* Initialize version 1 stsd */
  quicktime_set_stsd_audio_v1(table, 1, 3, 3 * atrack->channels, 2);

  quicktime_set_frma(atrack->track, "in24");

  if(codec->little_endian)
    {
    /* Set enda */
    quicktime_set_enda(&atrack->track->mdia.minf.stbl.stsd.table[0], 1);
    }

  if(codec->little_endian)
    codec->encode = encode_s24_le;
  else
    codec->encode = encode_s24_be;
  }

void quicktime_init_codec_in24(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;


  if(quicktime_get_enda(&atrack->track->mdia.minf.stbl.stsd.table[0]))
    codec->decode = decode_s24_le;
  else
    codec->decode = decode_s24_be;
  codec->init_encode = init_encode_in24;

  if(!atrack)
    return;

  atrack->block_align = 3 * atrack->channels;
  atrack->sample_format = LQT_SAMPLE_INT32;

  }

static void init_encode_in32(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];
  
  /* Initialize version 1 stsd */
  quicktime_set_stsd_audio_v1(table, 1, 4, 4 * atrack->channels, 2);

  quicktime_set_frma(atrack->track, "in32");

  if(codec->little_endian)
    {
    /* Set enda */
    quicktime_set_enda(&atrack->track->mdia.minf.stbl.stsd.table[0], 1);
    }

#ifdef WORDS_BIGENDIAN
  if(codec->little_endian)
    codec->encode = encode_s32_swap;
  else
    codec->encode = encode_s32;
#else
  if(codec->little_endian)
    codec->encode = encode_s32;
  else
    codec->encode = encode_s32_swap;
#endif

  
  }


void quicktime_init_codec_in32(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;

  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;


  codec->init_encode = init_encode_in32;

  if(!atrack)
    return;
  
  atrack->block_align = 4 * atrack->channels;
  atrack->sample_format = LQT_SAMPLE_INT32;

#ifdef WORDS_BIGENDIAN
  if(quicktime_get_enda(&atrack->track->mdia.minf.stbl.stsd.table[0]))
    codec->decode = decode_s32_swap;
  else
    codec->decode = decode_s32;
#else
  if(quicktime_get_enda(&atrack->track->mdia.minf.stbl.stsd.table[0]))
    codec->decode = decode_s32;
  else
    codec->decode = decode_s32_swap;
#endif
  }

/* Floating point */

static void init_encode_fl32(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];
  
  /* Initialize version 1 stsd */
  quicktime_set_stsd_audio_v1(table, 1, 4, 4 * atrack->channels, 2);

  quicktime_set_frma(atrack->track, "fl32");

  if(codec->little_endian)
    {
    codec->encode = encode_fl32_le;
    /* Set enda */
    quicktime_set_enda(&atrack->track->mdia.minf.stbl.stsd.table[0], 1);
    }
  else
    {
    codec->encode = encode_fl32_be;
    }
  }


void quicktime_init_codec_fl32(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;

  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;

  codec->init_encode = init_encode_fl32;
  
  if(!atrack)
    return;
  
  atrack->sample_format = LQT_SAMPLE_FLOAT;
  atrack->block_align = 4 * atrack->channels;

  if(quicktime_get_enda(&atrack->track->mdia.minf.stbl.stsd.table[0]))
    codec->decode = decode_fl32_le;
  else
    codec->decode = decode_fl32_be;
  }



static void init_encode_fl64(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];
  
  /* Initialize version 1 stsd */
  quicktime_set_stsd_audio_v1(table, 1, 8, 8 * atrack->channels, 2);

  quicktime_set_frma(atrack->track, "fl64");

  if(codec->little_endian)
    {
    codec->encode = encode_fl64_le;
    /* Set enda */
    quicktime_set_enda(&atrack->track->mdia.minf.stbl.stsd.table[0], 1);
    }
  else
    {
    codec->encode = encode_fl64_be;
    }
  }

void quicktime_init_codec_fl64(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;

  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;

  codec->init_encode = init_encode_fl64;
  
  if(!atrack)
    return;

  atrack->sample_format = LQT_SAMPLE_DOUBLE;
  atrack->block_align = 8 * atrack->channels;

  if(quicktime_get_enda(&atrack->track->mdia.minf.stbl.stsd.table[0]))
    codec->decode = decode_fl64_le;
  else
    codec->decode = decode_fl64_be;
  }


/* raw */

void quicktime_init_codec_rawaudio(quicktime_codec_t *codec_base,
                                   quicktime_audio_map_t *atrack,
                                   quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm; 

  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;

  if(!atrack)
    return;
  
  switch(atrack->track->mdia.minf.stbl.stsd.table[0].sample_size)
    {
    case 8:
      atrack->block_align = atrack->channels;
      atrack->sample_format = LQT_SAMPLE_UINT8;
      codec->encode = encode_8;
      codec->decode = decode_8;
      break;
    case 16:
      atrack->block_align = 2 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT16;
#ifdef WORDS_BIGENDIAN
      codec->encode = encode_s16;
      codec->decode = decode_s16;
#else
      codec->encode = encode_s16_swap;
      codec->decode = decode_s16_swap;
#endif
      break;
    case 24:
      atrack->block_align = 3 * atrack->channels;
      atrack->sample_format = LQT_SAMPLE_INT32;
      codec->encode = encode_s24_le;
      codec->decode = decode_s24_le;
      break;
    }
  }

static void init_encode_aulaw(quicktime_t * file, int track)
  {
  file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_size = 16;
  }

static int writes_compressed_aulaw(lqt_file_type_t type,
                                  const lqt_compression_info_t * ci)
  {
  return 1;
  }

void quicktime_init_codec_ulaw(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  codec_base->writes_compressed = writes_compressed_aulaw;
  
  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;
  
  codec->encode = encode_ulaw;
  codec->decode = decode_ulaw;
  codec->init_encode = init_encode_aulaw;
  codec->cid = LQT_COMPRESSION_ULAW;
  
  if(!atrack)
    return;

  atrack->block_align = atrack->channels;
  atrack->sample_format = LQT_SAMPLE_INT16;
  }

void quicktime_init_codec_alaw(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  codec_base->writes_compressed = writes_compressed_aulaw;
  
  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;
  
  codec->encode = encode_alaw;
  codec->decode = decode_alaw;
  codec->init_encode = init_encode_aulaw;
  codec->cid = LQT_COMPRESSION_ALAW;
  
  if(!atrack)
    return;

  atrack->block_align = atrack->channels;
  atrack->sample_format = LQT_SAMPLE_INT16;
  }

/* SampleDescription V2 definitions */
#define kAudioFormatFlagIsFloat          (1L<<0) 
#define kAudioFormatFlagIsBigEndian      (1L<<1) 
#define kAudioFormatFlagIsSignedInteger  (1L<<2) 
#define kAudioFormatFlagIsPacked         (1L<<3) 
#define kAudioFormatFlagIsAlignedHigh    (1L<<4) 
#define kAudioFormatFlagIsNonInterleaved (1L<<5) 
#define kAudioFormatFlagIsNonMixable     (1L<<6) 
#define kAudioFormatFlagsAreAllClear     (1L<<31)  

static void init_decode_lpcm(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];
  
  if(table->formatSpecificFlags & kAudioFormatFlagIsFloat)
    {
    switch(table->sample_size)
      {
      case 32:
        if(!(table->formatSpecificFlags & kAudioFormatFlagIsBigEndian))
          {
          codec->decode = decode_fl32_le;
          }
        else
          {
          codec->decode = decode_fl32_be;
          }
        atrack->sample_format = LQT_SAMPLE_FLOAT;
        break;
      case 64:
        if(!(table->formatSpecificFlags & kAudioFormatFlagIsBigEndian))
          {
          codec->decode = decode_fl64_le;
          }
        else
          {
          codec->decode = decode_fl64_be;
          }
        atrack->sample_format = LQT_SAMPLE_DOUBLE;
        break;
      }
    }
  else
    {
    switch(table->sample_size)
      {
      case 16:
        if(table->formatSpecificFlags & kAudioFormatFlagIsBigEndian)
          {
#ifdef WORDS_BIGENDIAN
          codec->decode = decode_s16;
#else
          codec->decode = decode_s16_swap;
#endif
          }
        else
          {
#ifdef WORDS_BIGENDIAN
          codec->decode = decode_s16_swap;
#else
          codec->decode = decode_s16;
#endif
          }
        atrack->sample_format = LQT_SAMPLE_INT16;
        break;
      case 24:
        if(table->formatSpecificFlags & kAudioFormatFlagIsBigEndian)
          {
          codec->decode = decode_s24_be;
          }
        else
          {
          codec->decode = decode_s24_le;
          }
        atrack->sample_format = LQT_SAMPLE_INT32;
        break;
      case 32:
        if(table->formatSpecificFlags & kAudioFormatFlagIsBigEndian)
          {
#ifdef WORDS_BIGENDIAN
          codec->decode = decode_s32;
#else
          codec->decode = decode_s32_swap;
#endif
          }
        else
          {
#ifdef WORDS_BIGENDIAN
          codec->decode = decode_s32_swap;
#else
          codec->decode = decode_s32;
#endif
          }
        atrack->sample_format = LQT_SAMPLE_INT32;
        break;
      }
    }

  atrack->block_align = (table->sample_size/8) * atrack->channels;

  }

static void init_encode_lpcm(quicktime_t * file, int track)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_pcm_codec_t *codec = atrack->codec->priv;
  quicktime_stsd_table_t *table = &atrack->track->mdia.minf.stbl.stsd.table[0];

  uint32_t formatSpecificFlags = 0;

  switch(codec->format)
    {
    case FORMAT_INT_16:
      formatSpecificFlags =
        kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
      if(!codec->little_endian)
        {
        formatSpecificFlags |= kAudioFormatFlagIsBigEndian;
#ifdef WORDS_BIGENDIAN
        codec->encode = encode_s16;
#else
        codec->encode = encode_s16_swap;
#endif
        }
      else
        {
#ifdef WORDS_BIGENDIAN
        codec->encode = encode_s16_swap;
#else
        codec->encode = encode_s16;
#endif
        }
      table->sample_size = 16;
      atrack->sample_format = LQT_SAMPLE_INT16;
      break;
    case FORMAT_INT_24:
      formatSpecificFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
      if(!codec->little_endian)
        {
        formatSpecificFlags |= kAudioFormatFlagIsBigEndian;
        codec->encode = encode_s24_be;
        }
      else
        {
        codec->encode = encode_s24_le;
        }
      table->sample_size = 24;
      atrack->sample_format = LQT_SAMPLE_INT32;
      break;
    case FORMAT_INT_32:
      formatSpecificFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
      if(!codec->little_endian)
        {
        formatSpecificFlags |= kAudioFormatFlagIsBigEndian;
#ifdef WORDS_BIGENDIAN
        codec->encode = encode_s32;
#else
        codec->encode = encode_s32_swap;
#endif
        }
      else
        {
#ifdef WORDS_BIGENDIAN
        codec->encode = encode_s32_swap;
#else
        codec->encode = encode_s32;
#endif
        }
      table->sample_size = 32;
      atrack->sample_format = LQT_SAMPLE_INT32;
      break;
    case FORMAT_FLOAT_32:
      formatSpecificFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
      if(!codec->little_endian)
        {
        formatSpecificFlags |= kAudioFormatFlagIsBigEndian;
        codec->encode = encode_fl32_be;
        }
      else
        {
        codec->encode = encode_fl32_le;
        }
      table->sample_size = 32;
      atrack->sample_format = LQT_SAMPLE_FLOAT;
      break;
    case FORMAT_FLOAT_64:
      formatSpecificFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
      if(!codec->little_endian)
        {
        formatSpecificFlags |= kAudioFormatFlagIsBigEndian;
        codec->encode = encode_fl64_be;
        }
      else
        {
        codec->encode = encode_fl64_le;
        }
      table->sample_size = 64;
      atrack->sample_format = LQT_SAMPLE_DOUBLE;
      break;
    }
  
  /* Initialize version 2 stsd */

  atrack->block_align = atrack->channels * (table->sample_size/8);

  quicktime_set_stsd_audio_v2(table,
                              formatSpecificFlags,
                              atrack->block_align /* constBytesPerAudioPacket */,
                              1 /* constLPCMFramesPerAudioPacket */);

  /* Set correct stsz */
  atrack->track->mdia.minf.stbl.stsz.sample_size =
    (table->sample_size/8)*atrack->channels;
  
  }


void quicktime_init_codec_lpcm(quicktime_codec_t *codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_pcm_codec_t *codec;
  
  /* Init public items */
  codec_base->delete_codec = delete_pcm;
  codec_base->decode_audio = decode_pcm;
  codec_base->encode_audio = encode_pcm;
  codec_base->set_parameter = set_parameter_pcm;
  
  /* Init private items */
  codec = calloc(1, sizeof(*codec));
  codec_base->priv = codec;
    
  codec->init_encode = init_encode_lpcm;
  codec->init_decode = init_decode_lpcm;
  
  }

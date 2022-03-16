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
#include "ffmpeg.h"
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>
#include <stdlib.h>
#include <string.h>

#define LOG_DOMAIN "ffmpeg_audio"

#if LIBAVCODEC_BUILD >= ((53<<16)+(25<<8)+0)
#define DECODE_AUDIO4 1
#elif LIBAVCODEC_BUILD >= ((52<<16)+(23<<8)+0)
#define DECODE_AUDIO3 1
#else
#define DECODE_AUDIO2 1
#endif

#if LIBAVCODEC_BUILD >= ((53<<16)+(34<<8)+0)
#define ENCODE_AUDIO2 1
#else
#define ENCODE_AUDIO 1
#endif

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
# define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#endif

/* The following code was ported from gmerlin_avdecoder (http://gmerlin.sourceforge.net) */

/* MPEG Audio header parsing code */

static int mpeg_bitrates[5][16] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,    // I
       256000, 288000, 320000, 352000, 384000, 416000, 448000, 0},
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,    // II
       128000, 160000, 192000, 224000, 256000, 320000, 384000, 0 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,    // III
       112000, 128000, 160000, 192000, 224000, 256000, 320000, 0 },

  /* MPEG-2 LSF */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,    // I
       128000, 144000, 160000, 176000, 192000, 224000, 256000, 0 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,
        64000,  80000,  96000, 112000, 128000, 144000, 160000, 0 } // II & III
};

static int mpeg_samplerates[3][3] = {
  { 44100, 48000, 32000 }, // MPEG1
  { 22050, 24000, 16000 }, // MPEG2
  { 11025, 12000, 8000 }   // MPEG2.5
  };

#define MPEG_ID_MASK        0x00180000
#define MPEG_MPEG1          0x00180000
#define MPEG_MPEG2          0x00100000
#define MPEG_MPEG2_5        0x00000000

#define MPEG_LAYER_MASK     0x00060000
#define MPEG_LAYER_III      0x00020000
#define MPEG_LAYER_II       0x00040000
#define MPEG_LAYER_I        0x00060000
#define MPEG_PROTECTION     0x00010000
#define MPEG_BITRATE_MASK   0x0000F000
#define MPEG_FREQUENCY_MASK 0x00000C00
#define MPEG_PAD_MASK       0x00000200
#define MPEG_PRIVATE_MASK   0x00000100
#define MPEG_MODE_MASK      0x000000C0
#define MPEG_MODE_EXT_MASK  0x00000030
#define MPEG_COPYRIGHT_MASK 0x00000008
#define MPEG_HOME_MASK      0x00000004
#define MPEG_EMPHASIS_MASK  0x00000003
#define LAYER_I_SAMPLES       384
#define LAYER_II_III_SAMPLES 1152

/* Header detection stolen from the mpg123 plugin of xmms */

static int mpa_header_check(uint32_t head)
  {
  if ((head & 0xffe00000) != 0xffe00000)
    return 0;
  if (!((head >> 17) & 3))
    return 0;
  if (((head >> 12) & 0xf) == 0xf)
    return 0;
  if (!((head >> 12) & 0xf))
    return 0;
  if (((head >> 10) & 0x3) == 0x3)
    return 0;
  if (((head >> 19) & 1) == 1 &&
      ((head >> 17) & 3) == 3 &&
      ((head >> 16) & 1) == 1)
    return 0;
  if ((head & 0xffff0000) == 0xfffe0000)
    return 0;
  return 1;
  }

typedef enum
  {
    MPEG_VERSION_NONE = 0,
    MPEG_VERSION_1 = 1,
    MPEG_VERSION_2 = 2,
    MPEG_VERSION_2_5
  } mpeg_version_t;

#define CHANNEL_STEREO   0
#define CHANNEL_JSTEREO  1
#define CHANNEL_DUAL     2
#define CHANNEL_MONO     3

typedef struct
  {
  mpeg_version_t version;
  int layer;
  int bitrate;    /* -1: VBR */
  int samplerate;
  int frame_bytes;
  int channel_mode;
  int mode;
  int samples_per_frame;
  } mpa_header;

static int mpa_header_equal(const mpa_header * h1, const mpa_header * h2)
  {
  return ((h1->layer == h2->layer) && (h1->version == h2->version) &&
          (h1->samplerate == h2->samplerate));
  }

static int mpa_decode_header(mpa_header * h, uint8_t * ptr,
                             const mpa_header * ref)
  {
  uint32_t header;
  int index;
  /* For calculation of the byte length of a frame */
  int pad;
  int slots_per_frame;
  h->frame_bytes = 0;
  header =
    ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
  if(!mpa_header_check(header))
    return 0;
  index = (header & MPEG_MODE_MASK) >> 6;
  switch(index)
    {
    case 0:
      h->channel_mode = CHANNEL_STEREO;
      break;
    case 1:
      h->channel_mode = CHANNEL_JSTEREO;
      break;
    case 2:
      h->channel_mode = CHANNEL_DUAL;
      break;
    case 3:
      h->channel_mode = CHANNEL_MONO;
      break;
    }
  /* Get Version */
  switch(header & MPEG_ID_MASK)
    {
    case MPEG_MPEG1:
      h->version = MPEG_VERSION_1;
        break;
    case MPEG_MPEG2:
      h->version = MPEG_VERSION_2;
      break;
    case MPEG_MPEG2_5:
      h->version = MPEG_VERSION_2_5;
      break;
    default:
      return 0;
    }
  /* Get Layer */
  switch(header & MPEG_LAYER_MASK)
    {
    case MPEG_LAYER_I:
      h->layer = 1;
      break;
    case MPEG_LAYER_II:
      h->layer = 2;
      break;
    case MPEG_LAYER_III:
      h->layer = 3;
      break;
    }
  index = (header & MPEG_BITRATE_MASK) >> 12;
  switch(h->version)
    {
    case MPEG_VERSION_1:
      switch(h->layer)
        {
        case 1:
          h->bitrate = mpeg_bitrates[0][index];
          break;
        case 2:
          h->bitrate = mpeg_bitrates[1][index];
          break;
        case 3:
          h->bitrate = mpeg_bitrates[2][index];
          break;
        }
      break;
    case MPEG_VERSION_2:
    case MPEG_VERSION_2_5:
      switch(h->layer)
        {
        case 1:
          h->bitrate = mpeg_bitrates[3][index];
          break;
        case 2:
        case 3:
          h->bitrate = mpeg_bitrates[4][index];
          break;
        }
      break;
    default: // This won't happen, but keeps gcc quiet
      return 0;
    }
  index = (header & MPEG_FREQUENCY_MASK) >> 10;
  switch(h->version)
    {
    case MPEG_VERSION_1:
      h->samplerate = mpeg_samplerates[0][index];
      break;
    case MPEG_VERSION_2:
      h->samplerate = mpeg_samplerates[1][index];
      break;
    case MPEG_VERSION_2_5:
      h->samplerate = mpeg_samplerates[2][index];
      break;
    default: // This won't happen, but keeps gcc quiet
      return 0;
    }
  pad = (header & MPEG_PAD_MASK) ? 1 : 0;
  if(h->layer == 1)
    {
    h->frame_bytes = ((12 * h->bitrate / h->samplerate) + pad) * 4;
    }
  else
    {
    slots_per_frame = ((h->layer == 3) &&
      ((h->version == MPEG_VERSION_2) ||
       (h->version == MPEG_VERSION_2_5))) ? 72 : 144;
    h->frame_bytes = (slots_per_frame * h->bitrate) / h->samplerate + pad;
    }
  // h->mode = (ptr[3] >> 6) & 3;
  h->samples_per_frame =
    (h->layer == 1) ? LAYER_I_SAMPLES : LAYER_II_III_SAMPLES;
  if(h->version != MPEG_VERSION_1)
    h->samples_per_frame /= 2;
  //  dump_header(h);

  /* Check against reference header */

  if(ref && !mpa_header_equal(ref, h))
    return 0;
  return 1;
  }

/* AC3 header */

typedef struct
  {
  /* Primary */
  int fscod;
  int frmsizecod;
  int bsid;
  int bsmod;
  int acmod;
  int cmixlev;
  int surmixlev;
  int dsurmod;
  int lfeon;
  
  /* Secondary */
  int frame_bytes;
  int bitrate;
  } a52_header;

#define PTR_2_32BE(p) \
((*(p) << 24) | \
(*(p+1) << 16) | \
(*(p+2) << 8) | \
*(p+3))

static inline int get_bits(uint32_t * bits, int num)
  {
  int ret;
  
  ret = (*bits) >> (32 - num);
  (*bits) <<= num;
  
  return ret;
  }

#define A52_FRAME_SAMPLES 1536

/* Tables from ffmpeg (ac3tab.c) */
static const uint16_t ac3_frame_size_tab[38][3] =
  {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
  };

const uint16_t ac3_bitrate_tab[19] =
  {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
  };

static int a52_header_read(a52_header * ret, uint8_t * buf)
  {
  int shift;
  uint32_t bits;
  memset(ret, 0, sizeof(*ret));

  /* Check syncword */
  if((buf[0] != 0x0b) || (buf[1] != 0x77))
    return 0;

  /* Skip syncword & crc */
  buf += 4;

  bits = PTR_2_32BE(buf);
  
  ret->fscod      = get_bits(&bits, 2);
  ret->frmsizecod = get_bits(&bits, 6);

  if(ret->frmsizecod > 37)
    return 0;
  
  ret->bsid       = get_bits(&bits, 5);

  if(ret->bsid >= 12)
    return 0;
  
  ret->bsmod      = get_bits(&bits, 3);
  ret->acmod      = get_bits(&bits, 3);

  if((ret->acmod & 0x01) && (ret->acmod != 0x01))
    ret->cmixlev = get_bits(&bits, 2);
  if(ret->acmod & 0x04)
    ret->surmixlev = get_bits(&bits, 2);
  if(ret->acmod == 0x02)
    ret->dsurmod = get_bits(&bits, 2);
  ret->lfeon = get_bits(&bits, 1);

  /* Secondary variables */

  shift = ret->bsid - 8;
  if(shift < 0)
    shift = 0;

  ret->bitrate = (ac3_bitrate_tab[ret->frmsizecod>>1] * 1000) >> shift;
  ret->frame_bytes = ac3_frame_size_tab[ret->frmsizecod][ret->fscod] * 2;
  return 1;
  }

/* Codec */

typedef struct
  {
  AVCodecContext * avctx;
  AVCodec * encoder;
  AVCodec * decoder;

  int initialized;
  
  /* Interleaved samples as avcodec needs them */
    
  int16_t * sample_buffer;
  int sample_buffer_alloc; 
  int samples_in_buffer;
  
  /* Buffer for the entire chunk */

  uint8_t * chunk_buffer;
  int chunk_buffer_alloc;
  int bytes_in_chunk_buffer;

  /* Start and end positions of the sample buffer */
    
  int64_t sample_buffer_start;
  int64_t sample_buffer_end;

  mpa_header mph;
  int have_mpa_header;

  uint8_t * extradata;
#if LIBAVCODEC_BUILD >= ((52<<16)+(26<<8)+0)
  AVPacket pkt;
#endif

  int64_t pts; /* For reading compressed packets */

  int header_written;
#if LIBAVCODEC_VERSION_MAJOR >= 54
  AVDictionary * options;
#endif
  } quicktime_ffmpeg_audio_codec_t;

static int lqt_ffmpeg_delete_audio(quicktime_codec_t *codec_base)
  {
  quicktime_ffmpeg_audio_codec_t * codec = codec_base->priv;
  if(codec->avctx)
    {
    if(codec->initialized)
      avcodec_close(codec->avctx);
    av_free(codec->avctx);
    }
  if(codec->sample_buffer)
    free(codec->sample_buffer);
  if(codec->chunk_buffer)
    free(codec->chunk_buffer);
  if(codec->extradata)
    free(codec->extradata);
#if LIBAVCODEC_VERSION_MAJOR >= 54
  if(codec->options)
    av_dict_free(&codec->options);
#endif
  free(codec);
  return 0;
  }

static int set_parameter(quicktime_t *file, 
                  int track, 
                  const char *key, 
                  const void *value)
  {
  quicktime_ffmpeg_audio_codec_t *codec = file->atracks[track].codec->priv;
  lqt_ffmpeg_set_parameter(codec->avctx,
#if LIBAVCODEC_VERSION_MAJOR >= 54
                           &codec->options,
#endif
                           key, value);
  return 0;
  }


/* Decode VBR chunk into the sample buffer */

static int decode_chunk_vbr(quicktime_t * file, int track)
  {
  int chunk_packets, i, num_samples, bytes_decoded;
  int packet_size, packet_samples;
  int frame_bytes;
  int new_samples;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;

#if DECODE_AUDIO4
  AVFrame f;
  int got_frame;
#endif
  
  chunk_packets = lqt_audio_num_vbr_packets(file, track, track_map->cur_chunk, &num_samples);

  if(!chunk_packets)
    return 0;

  new_samples = num_samples + AVCODEC_MAX_AUDIO_FRAME_SIZE / (2 * track_map->channels);
  
  if(codec->sample_buffer_alloc <
     codec->sample_buffer_end - codec->sample_buffer_start + new_samples)
    {
    
    codec->sample_buffer_alloc = codec->sample_buffer_end - codec->sample_buffer_start + new_samples;
    codec->sample_buffer = realloc(codec->sample_buffer, 2 * codec->sample_buffer_alloc *
                                   track_map->channels);
    }

  for(i = 0; i < chunk_packets; i++)
    {
    packet_size = lqt_audio_read_vbr_packet(file, track, track_map->cur_chunk, i,
                                            &codec->chunk_buffer, &codec->chunk_buffer_alloc,
                                            &packet_samples);
    if(!packet_size)
      return 0;

    bytes_decoded = codec->sample_buffer_alloc -
      (codec->sample_buffer_end - codec->sample_buffer_start);
    bytes_decoded *= 2 * track_map->channels;

#if DECODE_AUDIO3 || DECODE_AUDIO4
    codec->pkt.data = codec->chunk_buffer;
    codec->pkt.size = packet_size + FF_INPUT_BUFFER_PADDING_SIZE;

#if DECODE_AUDIO4
    frame_bytes = avcodec_decode_audio4(codec->avctx, &f,
                                        &got_frame, &codec->pkt);
    if(frame_bytes < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_decode_audio4 error");
      break;
      }
    bytes_decoded = f.nb_samples * 2 * track_map->channels;
    memcpy(&codec->sample_buffer[track_map->channels *
                                 (codec->sample_buffer_end -
                                  codec->sample_buffer_start)],
           f.extended_data[0],
           bytes_decoded);
    
#else // DECODE_AUDIO3
    frame_bytes = avcodec_decode_audio3(codec->avctx,
                                        &codec->sample_buffer[track_map->channels *
                                                               (codec->sample_buffer_end -
                                                                codec->sample_buffer_start)],
                                        &bytes_decoded,
                                        &codec->pkt);
    if(frame_bytes < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_decode_audio3 error");
      break;
      }
#endif // DECODE_AUDIO3
    
#else // DECODE_AUDIO2
    frame_bytes =
      avcodec_decode_audio2(codec->avctx,
                  &codec->sample_buffer[track_map->channels *
                                         (codec->sample_buffer_end - codec->sample_buffer_start)],
                  &bytes_decoded,
                  codec->chunk_buffer,
                  packet_size + FF_INPUT_BUFFER_PADDING_SIZE);
    if(frame_bytes < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_decode_audio2 error");
      break;
      }
#endif
    codec->sample_buffer_end += (bytes_decoded / (track_map->channels * 2));
    }
  track_map->cur_chunk++;
  return num_samples;
  }


/* Decode the current chunk into the sample buffer */

static int decode_chunk(quicktime_t * file, int track)
  {
  mpa_header mph;
    
  int frame_bytes;
  int num_samples;
  int new_samples;
  int samples_decoded = 0;
  int bytes_decoded;
  int bytes_used, bytes_skipped;
  int64_t chunk_size;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;

#if DECODE_AUDIO4
  AVFrame f;
  int got_frame;
#endif
  
  /* Read chunk */
  
  chunk_size = lqt_append_audio_chunk(file,
                                      track, track_map->cur_chunk,
                                      &codec->chunk_buffer,
                                      &codec->chunk_buffer_alloc,
                                      codec->bytes_in_chunk_buffer);

  
  if(!chunk_size)
    {
    /* If the codec is mp3, make sure to decode the very last frame */

    if((codec->avctx->codec_id == AV_CODEC_ID_MP3) &&
       (codec->bytes_in_chunk_buffer >= 4))
      {
      if(!mpa_decode_header(&mph, codec->chunk_buffer, (const mpa_header*)0))
        {
        lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Decode header failed");
        return 0;
        }
      if(mph.frame_bytes <= codec->bytes_in_chunk_buffer)
        {
        lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Huh, frame not decoded?");
        return 0;
        }

      if(codec->chunk_buffer_alloc < mph.frame_bytes + FF_INPUT_BUFFER_PADDING_SIZE)
        {
        codec->chunk_buffer_alloc = mph.frame_bytes + FF_INPUT_BUFFER_PADDING_SIZE;
        codec->chunk_buffer = realloc(codec->chunk_buffer, codec->chunk_buffer_alloc);
        }
      memset(codec->chunk_buffer + codec->bytes_in_chunk_buffer, 0,
             mph.frame_bytes - codec->bytes_in_chunk_buffer + FF_INPUT_BUFFER_PADDING_SIZE);
      num_samples = mph.samples_per_frame;
      codec->bytes_in_chunk_buffer = mph.frame_bytes;
      }
    else
      return 0;
    }
  else
    {
    num_samples = quicktime_chunk_samples(track_map->track, track_map->cur_chunk);
    track_map->cur_chunk++;
    codec->bytes_in_chunk_buffer += chunk_size;
    }
  

  if(!num_samples)
    {
    return 0;
    }
  /*
   *  For AVIs, chunk samples are not always 100% correct.
   *  Furthermore, there can be a complete mp3 frame from the last chunk!
   */

  num_samples += 8192;
  new_samples = num_samples + AVCODEC_MAX_AUDIO_FRAME_SIZE / (2 * track_map->channels);
  
  /* Reallocate sample buffer */
  
  if(codec->sample_buffer_alloc < codec->sample_buffer_end - codec->sample_buffer_start + new_samples)
    {
    
    codec->sample_buffer_alloc = codec->sample_buffer_end - codec->sample_buffer_start + new_samples;
    codec->sample_buffer = realloc(codec->sample_buffer, 2 * codec->sample_buffer_alloc *
                                   track_map->channels);
    }
  
  /* Decode this */

  bytes_used = 0;
  while(1)
    {

        
    /* BIG NOTE: We pass extra FF_INPUT_BUFFER_PADDING_SIZE for the buffer size
       because we know, that lqt_read_audio_chunk allocates 16 extra bytes for us */
    
    /* Some really broken mp3 files have the header bytes split across 2 chunks */

    if(codec->avctx->codec_id == AV_CODEC_ID_MP3)
      {
      if(codec->bytes_in_chunk_buffer < 4)
        {
        
        if(codec->bytes_in_chunk_buffer > 0)
          memmove(codec->chunk_buffer,
                  codec->chunk_buffer + bytes_used, codec->bytes_in_chunk_buffer);
        return 1;
        }

      bytes_skipped = 0;
      while(1)
        {
        if(!codec->have_mpa_header)
          {
          if(mpa_decode_header(&mph, &codec->chunk_buffer[bytes_used], NULL))
            {
            memcpy(&codec->mph, &mph, sizeof(mph));
            codec->have_mpa_header = 1;
            break;
            }
          }
        else if(mpa_decode_header(&mph, &codec->chunk_buffer[bytes_used], &codec->mph))
          break;
        
        bytes_used++;
        codec->bytes_in_chunk_buffer--;
        bytes_skipped++;
        if(codec->bytes_in_chunk_buffer <= 4)
          {

          if(codec->bytes_in_chunk_buffer > 0)
            memmove(codec->chunk_buffer,
                    codec->chunk_buffer + bytes_used, codec->bytes_in_chunk_buffer);
          return 1;
          }
        }
      if(codec->bytes_in_chunk_buffer < mph.frame_bytes)
        {
        
        if(codec->bytes_in_chunk_buffer > 0)
          memmove(codec->chunk_buffer,
                  codec->chunk_buffer + bytes_used, codec->bytes_in_chunk_buffer);
        return 1;
        }
      }

    /*
     *  decode an audio frame. return -1 if error, otherwise return the
     *  number of bytes used. If no frame could be decompressed,
     *  frame_size_ptr is zero. Otherwise, it is the decompressed frame
     *  size in BYTES.
     */

    bytes_decoded = codec->sample_buffer_alloc -
      (codec->sample_buffer_end - codec->sample_buffer_start);
    bytes_decoded *= 2 * track_map->channels;

#if DECODE_AUDIO3 || DECODE_AUDIO4
    codec->pkt.data = &codec->chunk_buffer[bytes_used];
    codec->pkt.size = codec->bytes_in_chunk_buffer + FF_INPUT_BUFFER_PADDING_SIZE;

#if DECODE_AUDIO4
    
    frame_bytes = avcodec_decode_audio4(codec->avctx, &f,
                                        &got_frame, &codec->pkt);
    if(frame_bytes < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_decode_audio4 error");
      break;
      }
    bytes_decoded = f.nb_samples * 2 * track_map->channels;
    memcpy(&codec->sample_buffer[track_map->channels *
                                 (codec->sample_buffer_end -
                                  codec->sample_buffer_start)],
           f.extended_data[0],
           bytes_decoded);
    
#else // DECODE_AUDIO3
    frame_bytes =
      avcodec_decode_audio3(codec->avctx,
                            &codec->sample_buffer[track_map->channels *
                                                   (codec->sample_buffer_end - codec->sample_buffer_start)],
                            &bytes_decoded,
                            &codec->pkt);

#endif

    
    
#else // DECODE_AUDIO2
    frame_bytes =
      avcodec_decode_audio2(codec->avctx,
                            &codec->sample_buffer[track_map->channels *
                                                  (codec->sample_buffer_end - codec->sample_buffer_start)],
                            &bytes_decoded,
                            &codec->chunk_buffer[bytes_used],
                            codec->bytes_in_chunk_buffer + FF_INPUT_BUFFER_PADDING_SIZE);
#endif
    if(frame_bytes < 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_decode_audio error");
      break;
      }

    bytes_used                   += frame_bytes;
    codec->bytes_in_chunk_buffer -= frame_bytes;
    
    if(bytes_decoded < 0)
      {
      if(codec->avctx->codec_id == AV_CODEC_ID_MP3)
        {
        /* For mp3, bytes_decoded < 0 means, that the frame should be muted */
        memset(&codec->sample_buffer[track_map->channels * (codec->sample_buffer_end -
                                                            codec->sample_buffer_start)],
               0, 2 * mph.samples_per_frame * track_map->channels);
        
        codec->sample_buffer_end += mph.samples_per_frame * track_map->channels;

        if(codec->bytes_in_chunk_buffer < 0)
          codec->bytes_in_chunk_buffer = 0;

        if(!codec->bytes_in_chunk_buffer)
          break;

        continue;
        }
      else
        {
        /* Incomplete frame, save the data for later use and exit here */
        if(codec->bytes_in_chunk_buffer > 0)
          memmove(codec->chunk_buffer,
                  codec->chunk_buffer + bytes_used, codec->bytes_in_chunk_buffer);
        return 1;
        }
      }
    
    /* This happens because ffmpeg adds FF_INPUT_BUFFER_PADDING_SIZE to the bytes returned */
    
    if(codec->bytes_in_chunk_buffer < 0)
      codec->bytes_in_chunk_buffer = 0;

    if(bytes_decoded < 0)
      {
      if(codec->bytes_in_chunk_buffer > 0)
        codec->bytes_in_chunk_buffer = 0;
      break;
      }
    
    samples_decoded += (bytes_decoded / (track_map->channels * 2));
    codec->sample_buffer_end += (bytes_decoded / (track_map->channels * 2));

    if((int)(codec->sample_buffer_end - codec->sample_buffer_start) > codec->sample_buffer_alloc)
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "BUUUUG, buffer overflow, %d %d",
              (int)(codec->sample_buffer_end - codec->sample_buffer_start),
              codec->sample_buffer_alloc);
    
    if(!codec->bytes_in_chunk_buffer)
      break;

    }
  //  track_map->current_chunk++;
  return samples_decoded;
  }

static void init_compression_info(quicktime_t *file, int track)
  {
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;

  if((codec->decoder->id == AV_CODEC_ID_MP2) ||
     (codec->decoder->id == AV_CODEC_ID_MP3))
    {
    mpa_header h;
    uint32_t header;
    uint8_t * ptr;
    int chunk_size;
    
    chunk_size = lqt_append_audio_chunk(file,
                                        track, track_map->cur_chunk,
                                        &codec->chunk_buffer,
                                        &codec->chunk_buffer_alloc,
                                        codec->bytes_in_chunk_buffer);

    if(chunk_size + codec->bytes_in_chunk_buffer < 4)
      return;

    ptr = codec->chunk_buffer;
    while(1)
      {
      header =
        ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
      if(mpa_header_check(header))
        break;

      ptr++;
      if(ptr - codec->chunk_buffer > codec->bytes_in_chunk_buffer - 4)
        return;
      }
    
    if(!mpa_decode_header(&h, ptr, NULL))
      return;

    if(h.layer == 2)
      track_map->ci.id = LQT_COMPRESSION_MP2;
    else if(h.layer == 3)
      track_map->ci.id = LQT_COMPRESSION_MP3;
    
    if(lqt_audio_is_vbr(file, track))
      track_map->ci.bitrate = -1;
    else
      track_map->ci.bitrate = h.bitrate;
    }
  else if(codec->decoder->id == AV_CODEC_ID_AC3)
    {
    a52_header h;
    uint8_t * ptr;
    int chunk_size;

    chunk_size = lqt_append_audio_chunk(file,
                                        track, track_map->cur_chunk,
                                        &codec->chunk_buffer,
                                        &codec->chunk_buffer_alloc,
                                        codec->bytes_in_chunk_buffer);

    if(chunk_size + codec->bytes_in_chunk_buffer < 8)
      return;
    
    ptr = codec->chunk_buffer;
    while(1)
      {
      if(a52_header_read(&h, ptr))
        break;
      
      ptr++;
      if(ptr - codec->chunk_buffer > codec->bytes_in_chunk_buffer - 8)
        return;
      }
    
    track_map->ci.bitrate = h.bitrate;
    track_map->ci.id = LQT_COMPRESSION_AC3;
    }
  }

static int lqt_ffmpeg_decode_audio(quicktime_t *file, void * output, long samples, int track)
  {
  uint8_t * header;
  uint32_t header_len;
  
  int samples_decoded;
  //  int result = 0;
  int64_t chunk_sample; /* For seeking only */
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;
  int channels = file->atracks[track].channels;
  //  int64_t total_samples;

  int samples_to_skip;
  int samples_to_move;


  if(!output) /* Global initialization */
    {
    init_compression_info(file, track);
    return 0;
    }
  
  /* Initialize codec */
  if(!codec->initialized)
    {
    /* Set some mandatory variables */
    codec->avctx->channels        = quicktime_track_channels(file, track);
    codec->avctx->sample_rate     = quicktime_sample_rate(file, track);

    if(track_map->track->mdia.minf.stbl.stsd.table[0].version == 1)
      {
      if(track_map->track->mdia.minf.stbl.stsd.table[0].audio_bytes_per_frame)
        codec->avctx->block_align =
          track_map->track->mdia.minf.stbl.stsd.table[0].audio_bytes_per_frame;
      }
    
    //  priv->ctx->block_align     = s->data.audio.block_align;
    //  priv->ctx->bit_rate        = s->codec_bitrate;
#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
    codec->avctx->bits_per_sample = quicktime_audio_bits(file, track);
#else
    codec->avctx->bits_per_coded_sample = quicktime_audio_bits(file, track);
#endif
    /* Some codecs need extra stuff */

    if(codec->decoder->id == AV_CODEC_ID_ALAC)
      {
      header = quicktime_wave_get_user_atom(track_map->track, "alac", &header_len);
      if(header)
        {
        codec->avctx->extradata = header;
        codec->avctx->extradata_size = header_len;
        }
      }
    if(codec->decoder->id == AV_CODEC_ID_QDM2)
      {
      header = quicktime_wave_get_user_atom(track_map->track, "QDCA", &header_len);
      if(header)
        {
        codec->extradata = malloc(header_len + 12);
        /* frma atom */
        codec->extradata[0] = 0x00;
        codec->extradata[1] = 0x00;
        codec->extradata[2] = 0x00;
        codec->extradata[3] = 0x0C;
        memcpy(codec->extradata + 4, "frmaQDM2", 8);
        /* QDCA atom */
        memcpy(codec->extradata + 12, header, header_len);
        codec->avctx->extradata = codec->extradata;
        codec->avctx->extradata_size = header_len + 12;
        }

      }

    
    //    memcpy(&codec->com.ffcodec_enc, &codec->com.params, sizeof(AVCodecContext));
   
    codec->avctx->codec_id = codec->decoder->id;
    codec->avctx->codec_type = codec->decoder->type;

#if LIBAVCODEC_VERSION_MAJOR < 54
    if(avcodec_open(codec->avctx, codec->decoder) != 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_open failed");
      return 0;
      }
#else
    if(avcodec_open2(codec->avctx, codec->decoder, NULL) != 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_open2 failed");
      return 0;
      }
#endif
    
    //    codec->sample_buffer_offset = 0;
    codec->initialized = 1;
    }

  /* Check if we have to reposition the stream */
  
  if(track_map->last_position != track_map->current_position)
    {

    if((track_map->current_position < codec->sample_buffer_start) || 
       (track_map->current_position + samples >= codec->sample_buffer_end))
      {
      if(lqt_audio_is_vbr(file, track))
        lqt_chunk_of_sample_vbr(&chunk_sample,
                                &track_map->cur_chunk,
                                track_map->track,
                                track_map->current_position);
      else
        quicktime_chunk_of_sample(&chunk_sample,
                                  &track_map->cur_chunk,
                                  track_map->track,
                                  track_map->current_position);
      codec->sample_buffer_start = chunk_sample;
      codec->sample_buffer_end   = chunk_sample;
      codec->bytes_in_chunk_buffer = 0;

      if(lqt_audio_is_vbr(file, track))
        decode_chunk_vbr(file, track);
      else
        decode_chunk(file, track);
      }
    }
  
  /* Flush unneeded samples */
  samples_to_skip = 0;
  if(track_map->current_position > codec->sample_buffer_start)
    {
    samples_to_skip = track_map->current_position - codec->sample_buffer_start;
    if(samples_to_skip > (int)(codec->sample_buffer_end - codec->sample_buffer_start))
      samples_to_skip = (int)(codec->sample_buffer_end - codec->sample_buffer_start);
    
    if(codec->sample_buffer_end > track_map->current_position)
      {
      samples_to_move = codec->sample_buffer_end - track_map->current_position;

      memmove(codec->sample_buffer,
              &codec->sample_buffer[samples_to_skip * channels],
              samples_to_move * channels * sizeof(int16_t));
      }
    codec->sample_buffer_start += samples_to_skip;
    

    }
  samples_to_skip = track_map->current_position - codec->sample_buffer_start;
  
  /* Read new chunks until we have enough samples */
  while(codec->sample_buffer_end - codec->sample_buffer_start < samples + samples_to_skip)
    {
    
    //    if(track_map->current_chunk >= track_map->track->mdia.minf.stbl.stco.total_entries)
    //      return 0;

    if(lqt_audio_is_vbr(file, track))
      {
      if(!decode_chunk_vbr(file, track))
        break;
      }
    else
      {
      if(!decode_chunk(file, track))
        break;
      }
    }
  samples_decoded = codec->sample_buffer_end - codec->sample_buffer_start - samples_to_skip;

  
  if(samples_decoded <= 0)
    {
    track_map->last_position = track_map->current_position;
    return 0;
    }
  if(samples_decoded > samples)
    samples_decoded = samples;
  
  /* Deinterleave into the buffer */
  
  
  //  deinterleave(output_i, output_f, codec->sample_buffer + (track_map->channels * samples_to_skip),
  //               channels, samples_decoded);

  memcpy(output, codec->sample_buffer + (channels * samples_to_skip),
         channels * samples_decoded * 2);
  
  track_map->last_position = track_map->current_position + samples_decoded;

  return samples_decoded;
  // #endif
  }

/*
 *   Encoding part
 */

static void create_dac3_atom(quicktime_t * file, int track, uint8_t * buf)
  {
  uint32_t tmp;
  uint8_t dac3_data[3];
  a52_header header;
  quicktime_trak_t *trak = file->atracks[track].track;
  
  if(a52_header_read(&header, buf))
    {
    tmp = header.fscod;

    tmp <<= 5;
    tmp |= header.bsid;
    
    tmp <<= 3;
    tmp |= header.bsmod;

    tmp <<= 3;
    tmp |= header.acmod;

    tmp <<= 1;
    tmp |= header.lfeon;

    tmp <<= 5;
    tmp |= header.frmsizecod >> 1;

    tmp <<= 5;

    dac3_data[0] = tmp >> 16;
    dac3_data[1] = (tmp >>  8) & 0xff;
    dac3_data[2] = tmp & 0xff;
    
    quicktime_user_atoms_add_atom(&trak->mdia.minf.stbl.stsd.table[0].user_atoms,
                                  "dac3", dac3_data,
                                  sizeof(dac3_data));
    }
  
  }


static int lqt_ffmpeg_encode_audio(quicktime_t *file, void * input,
                                   long samples, int track)
  {
  int result = -1;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;
  quicktime_trak_t *trak = track_map->track;
  int channels = file->atracks[track].channels;
  int frame_bytes;
  int samples_done = 0;
  int samples_encoded;
  /* Initialize encoder */
#if ENCODE_AUDIO2
  AVFrame f;
  AVPacket pkt;
  int got_packet;
#endif
  
  if(!codec->initialized)
    {
    codec->avctx->sample_rate = track_map->samplerate;
    codec->avctx->channels = channels;

    codec->avctx->codec_id = codec->encoder->id;
    codec->avctx->codec_type = codec->encoder->type;

    codec->avctx->sample_fmt = codec->encoder->sample_fmts[0];

    
#if LIBAVCODEC_VERSION_MAJOR < 54
    if(avcodec_open(codec->avctx, codec->encoder) != 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_open failed");
      return 0;
      }
#else
    if(avcodec_open2(codec->avctx, codec->encoder, NULL) != 0)
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "avcodec_open2 failed");
      return 0;
      }
#endif
    
    codec->initialized = 1;

    /* One frame is: bitrate * frame_samples / (samplerate * 8) + 1024 */
    codec->chunk_buffer_alloc = ( codec->avctx->frame_size
                                  * sizeof( int16_t )
                                  * codec->avctx->channels);
    codec->chunk_buffer = malloc(codec->chunk_buffer_alloc);
    
    if(trak->strl)
      lqt_set_audio_bitrate(file, track, codec->avctx->bit_rate);
    }

  /* Allocate sample buffer if necessary */

  if(codec->sample_buffer_alloc < (codec->samples_in_buffer + samples))
    {
    codec->sample_buffer_alloc = codec->samples_in_buffer + samples + 16;
    codec->sample_buffer = realloc(codec->sample_buffer,
                                   codec->sample_buffer_alloc * channels * sizeof(int16_t));
    }

  /* Interleave */

  //  interleave(&codec->sample_buffer[codec->samples_in_buffer * channels],
  //             input_i, input_f, samples, channels);

  memcpy(codec->sample_buffer + codec->samples_in_buffer * channels,
         input, samples * channels * 2);
  
  codec->samples_in_buffer += samples;
  
  /* Encode */
  
  while(codec->samples_in_buffer >= codec->avctx->frame_size)
    {
#if ENCODE_AUDIO2
    av_init_packet(&pkt);
    pkt.data = codec->chunk_buffer;
    pkt.size = codec->chunk_buffer_alloc;

    av_frame_unref(&f);
    f.nb_samples = codec->avctx->frame_size;
    
    avcodec_fill_audio_frame(&f, channels, codec->avctx->sample_fmt,
                             (uint8_t*)&codec->sample_buffer[samples_done*channels],
                             codec->avctx->frame_size * channels * 2, 
                             1);

    if(avcodec_encode_audio2(codec->avctx, &pkt,
                             &f, &got_packet) < 0)
      return 0;

    if(got_packet && pkt.size)
      frame_bytes = pkt.size;
    else
      frame_bytes = 0;

#else
    frame_bytes = avcodec_encode_audio(codec->avctx, codec->chunk_buffer,
                                       codec->chunk_buffer_alloc,
                                       &codec->sample_buffer[samples_done*channels]);
#endif
    
    if(frame_bytes > 0)
      {
      quicktime_write_chunk_header(file, trak);
      samples_encoded = codec->avctx->frame_size;
      samples_done              += samples_encoded;
      codec->samples_in_buffer  -= samples_encoded;
      
      result = !quicktime_write_data(file, codec->chunk_buffer, frame_bytes);
      trak->chunk_samples = samples_encoded;
      quicktime_write_chunk_footer(file, trak);
      file->atracks[track].cur_chunk++;
      }
    }
  if(codec->samples_in_buffer && samples_done)
    memmove(codec->sample_buffer, &codec->sample_buffer[samples_done*channels],
            codec->samples_in_buffer * sizeof(int16_t) * channels);
  return result;
  }

static int read_packet_mpa(quicktime_t * file, lqt_packet_t * p, int track)
  {
  mpa_header h;
  uint8_t * ptr;
  uint32_t header;
  int chunk_size;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;
  
  if(codec->bytes_in_chunk_buffer < 4)
    {
    chunk_size = lqt_append_audio_chunk(file,
                                        track, track_map->cur_chunk,
                                        &codec->chunk_buffer,
                                        &codec->chunk_buffer_alloc,
                                        codec->bytes_in_chunk_buffer);

    //fprintf(stderr, "Got chunk %ld: %d bytes\n", track_map->cur_chunk, chunk_size);
    
    if(chunk_size + codec->bytes_in_chunk_buffer < 4)
      return 0;

    codec->bytes_in_chunk_buffer += chunk_size;
    track_map->cur_chunk++;
    }
  
  /* Check for mpa header */

  ptr = codec->chunk_buffer;
  while(1)
    {
    header =
      ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
    if(mpa_header_check(header))
      break;
    
    ptr++;

    if(ptr - codec->chunk_buffer > codec->bytes_in_chunk_buffer - 4)
      return 0;
    }

  if(!mpa_decode_header(&h, ptr, NULL))
    return 0;
  
  lqt_packet_alloc(p, h.frame_bytes);
  memcpy(p->data, ptr, h.frame_bytes);
  ptr += h.frame_bytes;
  
  codec->bytes_in_chunk_buffer -= (ptr - codec->chunk_buffer);

  if(codec->bytes_in_chunk_buffer)
    memmove(codec->chunk_buffer, ptr, codec->bytes_in_chunk_buffer);

  p->duration = h.samples_per_frame;
  p->timestamp = codec->pts;
  codec->pts += p->duration;
  p->flags = LQT_PACKET_KEYFRAME;
  p->data_len = h.frame_bytes;
  
  return 1;
  }

#if 0
static int writes_compressed_mp2(lqt_file_type_t type,
                                 const lqt_compression_info_t * ci)
  {
  return 1;
  }

static int init_compressed_mp2(quicktime_t * file, int track)
  {
  
  return 0;
  }

static int init_compressed_ac3(quicktime_t * file, int track)
  {
  
  return 0;
  }
#endif

static int write_packet_ac3(quicktime_t * file, lqt_packet_t * p, int track)
  {
  int result;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = atrack->codec->priv;
  
  if(!codec->header_written && (p->data_len >= 8))
    {
    if(file->file_type & (LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4))
      create_dac3_atom(file, track, p->data);
    else if(file->file_type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML))
      lqt_set_audio_bitrate(file, track, atrack->ci.bitrate);
    codec->header_written = 1;
    }
  
  quicktime_write_chunk_header(file, atrack->track);
  result = !quicktime_write_data(file, p->data, p->data_len);
  
  atrack->track->chunk_samples = p->duration;
  quicktime_write_chunk_footer(file, atrack->track);
  atrack->cur_chunk++;
  if(result)
    return 0;
  else
    return 1;
  
  }

static int read_packet_ac3(quicktime_t * file, lqt_packet_t * p, int track)
  {
  a52_header h;
  uint8_t * ptr;
  int chunk_size;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_ffmpeg_audio_codec_t *codec = track_map->codec->priv;
  
  if(codec->bytes_in_chunk_buffer < 8)
    {
    chunk_size = lqt_append_audio_chunk(file,
                                        track, track_map->cur_chunk,
                                        &codec->chunk_buffer,
                                        &codec->chunk_buffer_alloc,
                                        codec->bytes_in_chunk_buffer);

    if(chunk_size + codec->bytes_in_chunk_buffer < 8)
      return 0;

    codec->bytes_in_chunk_buffer += chunk_size;
    track_map->cur_chunk++;
    }

  /* Check for mpa header */

  ptr = codec->chunk_buffer;
  while(1)
    {
    if(a52_header_read(&h, ptr))
      break;
      
    ptr++;

    if(ptr - codec->chunk_buffer > codec->bytes_in_chunk_buffer - 8)
      return 0;
    }
  
  lqt_packet_alloc(p, h.frame_bytes);
  memcpy(p->data, ptr, h.frame_bytes);
  ptr += h.frame_bytes;

  codec->bytes_in_chunk_buffer -= (ptr - codec->chunk_buffer);

  if(codec->bytes_in_chunk_buffer)
    memmove(codec->chunk_buffer, ptr, codec->bytes_in_chunk_buffer);

  p->data_len = h.frame_bytes;
  p->duration = A52_FRAME_SAMPLES;
  p->timestamp = codec->pts;
  codec->pts += p->duration;
  p->flags = LQT_PACKET_KEYFRAME;
  return 1;
  }

void quicktime_init_audio_codec_ffmpeg(quicktime_codec_t * codec_base,
                                       quicktime_audio_map_t *atrack, AVCodec *encoder,
                                       AVCodec *decoder)
  {
  quicktime_ffmpeg_audio_codec_t *codec;
  
  //  avcodec_init();
  codec = calloc(1, sizeof(*codec));
  if(!codec)
    return;
  
  codec->encoder = encoder;
  codec->decoder = decoder;
#if LIBAVCODEC_VERSION_INT < ((53<<16)|(8<<8)|0)
  codec->avctx = avcodec_alloc_context();
#else
  codec->avctx = avcodec_alloc_context3(NULL);
#endif
  codec_base->priv = (void *)codec;

  codec_base->delete_codec = lqt_ffmpeg_delete_audio;
  if(encoder)
    codec_base->encode_audio = lqt_ffmpeg_encode_audio;
  if(decoder)
    codec_base->decode_audio = lqt_ffmpeg_decode_audio;
  codec_base->set_parameter = set_parameter;

  if((decoder->id == AV_CODEC_ID_MP3) || (decoder->id == AV_CODEC_ID_MP2))
    codec_base->read_packet = read_packet_mpa;
  else if(decoder->id == AV_CODEC_ID_AC3)
    {
    codec_base->write_packet = write_packet_ac3;
    codec_base->read_packet = read_packet_ac3;
    }
  if(!atrack)
    return;
  
  atrack->sample_format = LQT_SAMPLE_INT16;
  }

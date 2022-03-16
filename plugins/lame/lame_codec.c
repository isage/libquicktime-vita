/*******************************************************************************
 lame_codec.c

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
#include <lame/lame.h>
#include <stdlib.h>
#include <string.h>
#include <lame_codec.h>

#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>


#define LOG_DOMAIN "lame"

/*
 *  We decode all headers to figure out, how many COMPLETE frames we have encoded.
 */

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

static int header_check(uint32_t head)
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

  int bitrate_mode;
  int bitrate;
  
  int min_bitrate; /* ABR only */
  int max_bitrate; /* ABR only  */
  int samplerate;
  int frame_bytes;
  int channel_mode;
  int mode;
  int samples_per_frame;
  } mpeg_header;

static int decode_header(mpeg_header * h, uint8_t * ptr)
  {
  uint32_t header;
  int index;
  /* For calculation of the byte length of a frame */
  int pad;
  int slots_per_frame;
  h->frame_bytes = 0;
  header =
    ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
  if(!header_check(header))
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
  return 1;
  }

typedef struct
  {
  // mp3 encoder
  lame_global_flags *lame_global;
  //        mpeg3_layer_t *encoded_header;
  int encode_initialized;
  int input_size;
  int input_allocated;
  
  unsigned char *encoder_output;
  int encoder_output_alloc;
  int encoder_output_size;
  
  int samples_per_frame;
  int stereo;

  int16_t * input_buffer[2];
  int input_buffer_alloc;

  int64_t samples_read;    /* samples passed to lame_encode_buffer */
  int64_t samples_written; /* samples written to the file          */

  /* Configuration stuff */
  int bitrate_mode;
  int bitrate;
  int bitrate_min;
  int bitrate_max;
  int quality;
  int quality_vbr;

  int header_set;
  
  } quicktime_mp3_codec_t;

static int delete_codec(quicktime_codec_t *codec_base)
  {
  quicktime_mp3_codec_t *codec = codec_base->priv;
  
  if(codec->lame_global)
    lame_close(codec->lame_global);
  
  if(codec->input_buffer[0])
    free(codec->input_buffer[0]);
  if(codec->input_buffer[1])
    free(codec->input_buffer[1]);
  
  
  if(codec->encoder_output)
    free(codec->encoder_output);
  free(codec);
  return 0;
  }

#define PUT_16_LE(num, ptr) \
ptr[0] = num & 0xff; \
ptr[1] = (num >> 8) & 0xff;\
ptr+=2;

#define PUT_32_LE(num, ptr) \
ptr[0] = num & 0xff; \
ptr[1] = (num >> 8) & 0xff;\
ptr[2] = (num >> 16) & 0xff;\
ptr[3] = (num >> 24) & 0xff;\
ptr+=4;

/* nanosoft style mp3 definitions */

#define MPEGLAYER3_ID_UNKNOWN            0
#define MPEGLAYER3_ID_MPEG               1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

#define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002

static void set_avi_mp3_header(quicktime_t * file, int track, mpeg_header * h, int vbr)
  {
  uint8_t extradata[12];
  uint8_t * extradata_ptr;
  uint32_t tmp;
  
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_trak_t *trak = atrack->track;
  
  //  quicktime_mp3_codec_t *codec = atrack->codec->priv;
  
  /* Initialize AVI header */
  
  if(!vbr)
    lqt_set_audio_bitrate(file, track, h->bitrate);
  
  /* Extradata (completely unneccesary for decoding) */
  
  extradata_ptr = extradata;

  tmp = MPEGLAYER3_ID_MPEG; // WORD          wID
  PUT_16_LE(tmp, extradata_ptr);

  tmp = MPEGLAYER3_FLAG_PADDING_ISO; // DWORD         fdwFlags;
  PUT_32_LE(tmp, extradata_ptr);
      
  switch(h->version)
    {
    case MPEG_VERSION_1:
      tmp = ((144000 / 1) * (h->bitrate/1000)) / atrack->samplerate; // WORD nBlockSize;
      break;
    case MPEG_VERSION_2:
      tmp = ((144000 / 2) * (h->bitrate/1000)) / atrack->samplerate; // WORD nBlockSize;
      break;
    case MPEG_VERSION_2_5:
      tmp = ((144000 / 4) * (h->bitrate/1000)) / atrack->samplerate; // WORD nBlockSize;
      break;
    default: // This won't happen, but keeps gcc quiet
      return;
    }
  
  PUT_16_LE(tmp, extradata_ptr);

  tmp = 1; // WORD nFramesPerBlock;
  PUT_16_LE(tmp, extradata_ptr);

  tmp = 1393; //  WORD nCodecDelay; WTF???
  PUT_16_LE(tmp, extradata_ptr);
      
  quicktime_strf_set_audio_extradata(&trak->strl->strf, extradata, 12);
  }

static int write_data(quicktime_t *file, int track,
                      quicktime_mp3_codec_t *codec, int samples)
  {
  int frame_samples;
  mpeg_header h;
  int result = 0;
  quicktime_audio_map_t *atrack;

  int one_packet_per_chunk;

  int vbr = lqt_audio_is_vbr(file, track);
  
  atrack = &file->atracks[track];

  if(vbr && atrack->track->strl)
    one_packet_per_chunk = 1;
  else
    one_packet_per_chunk = 0;
  
  memset(&h, 0, sizeof(h));

  if(!one_packet_per_chunk)
    quicktime_write_chunk_header(file, atrack->track);
  
  while(codec->encoder_output_size > 4)
    {
    if(!decode_header(&h, codec->encoder_output))
      {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "Ouch: lame created non mp3 data\n");
      break;
      }

    if(!codec->header_set && atrack->track->strl)
      {
      set_avi_mp3_header(file, track, &h, vbr);
      codec->header_set = 1;
      }
      
    if((codec->encoder_output_size >= h.frame_bytes) || (samples > 0))
      {
      frame_samples = samples > 0 ? samples : h.samples_per_frame;

      if(one_packet_per_chunk)
        quicktime_write_chunk_header(file, atrack->track);
      
      if(vbr)
        lqt_start_audio_vbr_frame(file, track);

      result = !quicktime_write_data(file, codec->encoder_output, h.frame_bytes);

      if(vbr)
        lqt_finish_audio_vbr_frame(file, track, frame_samples);

      if(one_packet_per_chunk)
        {
        quicktime_write_chunk_footer(file, atrack->track);
        atrack->cur_chunk++;
        }
      else
        atrack->track->chunk_samples += frame_samples;
      
      codec->samples_written += frame_samples;
      codec->encoder_output_size -= h.frame_bytes;
      
      if(codec->encoder_output_size)
        memmove(codec->encoder_output,
                codec->encoder_output + h.frame_bytes, codec->encoder_output_size);
      }
    else
      break;
    }

  if(!one_packet_per_chunk)
    {
    quicktime_write_chunk_footer(file, atrack->track);
    atrack->cur_chunk++;
    }
  return result;
  }


static int encode(quicktime_t *file, 
                  void * _input, 
                  long samples,
                  int track)
  {
  int16_t * input;
  int result = 0;
  int encoded_size;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_trak_t *trak = atrack->track;
  quicktime_mp3_codec_t *codec = atrack->codec->priv;
  int i;
  
  if(!codec->encode_initialized)
    {
    /* CBR audio is written only for AVIs and when VBR is off */
    if(!(trak->strl && (codec->bitrate_mode == vbr_off)))
      lqt_init_vbr_audio(file, track);
    
    codec->encode_initialized = 1;
    codec->lame_global = lame_init();

    if(codec->bitrate_mode == vbr_off)
      {
      lame_set_VBR(codec->lame_global, vbr_off);
      lame_set_brate(codec->lame_global, codec->bitrate / 1000);
      }
    else if(codec->bitrate_mode == vbr_default)
      {
      lame_set_VBR(codec->lame_global, vbr_default);
      lame_set_VBR_q(codec->lame_global, codec->quality_vbr);
      }
    else if(codec->bitrate_mode == vbr_abr)
      {
      lame_set_VBR(codec->lame_global, vbr_abr);
      lame_set_VBR_min_bitrate_kbps(codec->lame_global, codec->bitrate_min/1000);
      lame_set_VBR_max_bitrate_kbps(codec->lame_global, codec->bitrate_max/1000);
      }
    
    lame_set_quality(codec->lame_global, codec->quality);
    lame_set_in_samplerate(codec->lame_global, atrack->samplerate);
    lame_set_out_samplerate(codec->lame_global, atrack->samplerate);
    
    lame_set_bWriteVbrTag(codec->lame_global, 0);
    
    /* Also support Mono streams */
    
    codec->stereo = (trak->mdia.minf.stbl.stsd.table[0].channels == 1) ? 0 : 1;
    lame_set_num_channels(codec->lame_global, (codec->stereo ? 2 : 1));
    
    //    lame_set_padding_type(codec->lame_global, PAD_ALL); // PAD_NO | PAD_ALL | PAD_ADJUST
    
    if((result = lame_init_params(codec->lame_global)) < 0)
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "lame_init_params returned %d\n", result);
    //    codec->encoded_header = mpeg3_new_layer();
    codec->samples_per_frame = lame_get_framesize(codec->lame_global);

    }

  /* Reallocate output if necessary */

  encoded_size = (5*samples)/4 + 7200;
  
  if(codec->encoder_output_alloc < encoded_size + codec->encoder_output_size)
    {
    codec->encoder_output_alloc = encoded_size + codec->encoder_output_size + 16;
    codec->encoder_output       = realloc(codec->encoder_output, codec->encoder_output_alloc);
    }

  if(codec->input_buffer_alloc < samples)
    {
    codec->input_buffer_alloc = samples + 16;
    codec->input_buffer[0] =
      realloc(codec->input_buffer[0], codec->input_buffer_alloc * sizeof(*codec->input_buffer[0]));
    
    if(codec->stereo)
      codec->input_buffer[1] =
        realloc(codec->input_buffer[1], codec->input_buffer_alloc * sizeof(*codec->input_buffer[1]));
    }

  input = (int16_t*)_input;
    
  if(codec->stereo)
    {
    for(i = 0; i < samples; i++)
      {
      codec->input_buffer[0][i] = *(input++);
      codec->input_buffer[1][i] = *(input++);
      }
    }
  else
    {
    for(i = 0; i < samples; i++)
      codec->input_buffer[0][i] = *(input++);
    }
  
  result = lame_encode_buffer(codec->lame_global,
                              codec->input_buffer[0],
                              codec->stereo ? codec->input_buffer[1] : codec->input_buffer[0],
                              samples,
                              codec->encoder_output + codec->encoder_output_size,
                              codec->encoder_output_alloc - codec->encoder_output_size);
  codec->samples_read += samples;
    
  if(result > 0)
    {
    codec->encoder_output_size += result;
    result = write_data(file, track, codec, -1);
    }
  
  return result;
  }



static int set_parameter(quicktime_t *file, int track, 
                         const char *key, const void *value)
  {
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_mp3_codec_t *codec =
    atrack->codec->priv;
  
  if(!strcasecmp(key, "mp3_bitrate_mode"))
    {
    if(!strcmp((char*)value, "CBR"))
      codec->bitrate_mode = vbr_off;
    else if(strcmp((char*)value, "ABR"))
      codec->bitrate_mode = vbr_abr;
    else if(strcmp((char*)value, "VBR"))
      codec->bitrate_mode = vbr_default;
    }
  else if(!strcasecmp(key, "mp3_bitrate"))
    codec->bitrate = *(int*)value;
  else if(!strcasecmp(key, "mp3_bitrate_min"))
    codec->bitrate_min = *(int*)value;
  else if(!strcasecmp(key, "mp3_bitrate_max"))
    codec->bitrate_max = *(int*)value;
  else if(!strcasecmp(key, "mp3_quality"))
    codec->quality = *(int*)value;
  else if(!strcasecmp(key, "mp3_quality_vbr"))
    codec->quality_vbr = *(int*)value;
  
  return 0;
  }

static int flush(quicktime_t *file, int track)
  {
  int result = 0;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_mp3_codec_t *codec = atrack->codec->priv;
  
  if(codec->encode_initialized)
    {
    //    samples_left = lame_get_mf_samples_to_encode(codec->lame_global);
    
    result = lame_encode_flush(codec->lame_global,
                               codec->encoder_output + codec->encoder_output_size, 
                               codec->encoder_output_alloc);
    /* Check if more frames arrived */

    if(result > 0)
      {
      codec->encoder_output_size += result;
      write_data(file, track, codec, codec->samples_read - codec->samples_written);
      return 1;
      }
    }
  return 0;
  }

static int writes_compressed_lame(lqt_file_type_t type, const lqt_compression_info_t * ci)
  {
  if(type & (LQT_FILE_QT_OLD | LQT_FILE_QT | LQT_FILE_MP4))
    return 1;
  else if(type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML))
    return 1;
  return 0;
  }

static int write_packet_lame(quicktime_t * file, lqt_packet_t * p, int track)
  {
  int result;
  int one_packet_per_chunk;
  quicktime_audio_map_t *atrack = &file->atracks[track];
  quicktime_mp3_codec_t *codec = atrack->codec->priv;
  
  if(p->data_len < 4)
    return 0;
  
  if((atrack->ci.bitrate < 0) && atrack->track->strl)
    one_packet_per_chunk = 1;
  else
    one_packet_per_chunk = 0;
  
  if(!codec->header_set)
    {
    if(!(file->file_type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML)) ||
       (file->atracks[track].ci.bitrate < 0))
      lqt_init_vbr_audio(file, track);
      
    if(atrack->track->strl)
      {
      mpeg_header h;
      if(!decode_header(&h, p->data))
        return 0;
      set_avi_mp3_header(file, track, &h, (atrack->ci.bitrate < 0));
      }
    codec->header_set = 1;
    }

  if((file->write_trak != atrack->track) && !one_packet_per_chunk)
    quicktime_write_chunk_header(file, atrack->track);
  
  if(lqt_audio_is_vbr(file, track))
    {
    if(one_packet_per_chunk)
      quicktime_write_chunk_header(file, atrack->track);
    lqt_start_audio_vbr_frame(file, track);
    result = !quicktime_write_data(file, p->data, p->data_len);

    lqt_finish_audio_vbr_frame(file, track, p->duration);
    
    /* Close this audio chunk. For non-AVIs, lqt will do that for us */
    
    if(one_packet_per_chunk)
      {
      quicktime_write_chunk_footer(file, atrack->track);
      atrack->cur_chunk++;
      }
    }
  else
    {
    result = !quicktime_write_data(file, p->data, p->data_len);
    atrack->track->chunk_samples += p->duration;
    }
  
  if(result)
    return 0;
  else
    return 1;
  
  }

void quicktime_init_codec_lame(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack)
  {
  quicktime_mp3_codec_t *codec;

  codec = calloc(1, sizeof(*codec));
  
  /* Init public items */
  codec_base->priv = codec;
  codec_base->delete_codec = delete_codec;
  //  codec_base->decode_audio = decode;
  codec_base->encode_audio = encode;
  codec_base->set_parameter = set_parameter;
  codec_base->flush = flush;
  
  codec_base->writes_compressed = writes_compressed_lame;
  codec_base->write_packet = write_packet_lame;
  
  codec->bitrate = 256000;
  codec->quality = 0;

  if(!atrack)
    return;
  
  atrack->sample_format = LQT_SAMPLE_INT16;
  }

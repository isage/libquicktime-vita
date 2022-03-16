/*******************************************************************************
 lqt_codecs.c

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
#include "quicktime/colormodels.h"
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "codecs.h"

#define LOG_DOMAIN "codecs"

static int quicktime_delete_codec_stub(quicktime_codec_t *codec)
  {
  lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
          "quicktime_delete_stub called");
  return 0;
  }

static int quicktime_decode_video_stub(quicktime_t *file, 
                                       unsigned char **row_pointers, 
                                       int track)
  {
  lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
          "quicktime_decode_video_stub called");
  return 1;
  }

static int quicktime_encode_video_stub(quicktime_t *file, 
                                       unsigned char **row_pointers, 
                                       int track)
  {
  lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
          "quicktime_encode_video_stub called");
  return 1;
  }

static int quicktime_decode_audio_stub(quicktime_t *file, 
                                       void * output,
                                       long samples,
                                       int track)
  {
  lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
          "quicktime_decode_audio_stub called");
  return 0;
  }

static int quicktime_encode_audio_stub(quicktime_t *file, 
                                       void * input,
                                       long samples,
                                       int track)
  {
  lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
          "quicktime_encode_audio_stub called");
  return 1;
  }

static int quicktime_flush_codec_stub(quicktime_t *file, int track)
  {
  return 0;
  }

/*
 *  Original quicktime4linux function changed for dynamic loading
 */
quicktime_codec_t * quicktime_load_codec(lqt_codec_info_t * info,
                                         quicktime_audio_map_t * amap,
                                         quicktime_video_map_t * vmap)
  {
  quicktime_codec_t * codec;
  lqt_init_codec_func_t init_codec;
  lqt_init_codec_func_t (*get_codec)(int);

  codec = calloc(1, sizeof(*codec));

  /* Set stubs */
  codec->delete_codec = quicktime_delete_codec_stub;
  codec->decode_video = quicktime_decode_video_stub;
  codec->encode_video = quicktime_encode_video_stub;
  codec->decode_audio = quicktime_decode_audio_stub;
  codec->encode_audio = quicktime_encode_audio_stub;
  codec->flush = quicktime_flush_codec_stub;
  
  if(!info)
    return codec;
  
  codec->info = lqt_codec_info_copy_single(info);
  
  lqt_log(NULL, LQT_LOG_DEBUG, LOG_DOMAIN,
          "Loading module %s", info->module_filename);


  if (strcmp(info->module_filename, "audio") == 0)
      get_codec = audiocodec_get_codec;
  else if (strcmp(info->module_filename, "video") == 0)
      get_codec = videocodec_get_codec;
  else if (strcmp(info->module_filename, "ffmpeg") == 0)
      get_codec = ffmpegcodec_get_codec;
  
  if(!get_codec)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
            "Module %s contains no function get_codec",
            info->module_filename);
    goto fail;
    }

  lqt_log(NULL, LQT_LOG_DEBUG, LOG_DOMAIN,
          "Init codec %d", info->module_index);

  init_codec = get_codec(info->module_index);
  init_codec(codec, amap, vmap);
  
  return codec;

  fail:
  free(codec);
  return NULL;
  }

int quicktime_init_vcodec(quicktime_video_map_t *vtrack, int encode,
                          lqt_codec_info_t * codec_info)
  {
  lqt_codec_info_t ** codec_array = (lqt_codec_info_t**)0;
  
  char *compressor = vtrack->track->mdia.minf.stbl.stsd.table[0].format;
  
  /* Try to find the codec */

  if(!codec_info)
    {
    codec_array = lqt_find_video_codec(compressor, encode);
  
    if(!codec_array)
      {
      lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
              "Could not find video %s for fourcc %4s",
              (encode ? "Encoder" : "Decoder"), compressor);
      }
    else
      codec_info = *codec_array;
    }
  
  vtrack->codec = quicktime_load_codec(codec_info, NULL, vtrack);

  if(!vtrack->codec)
    {
    lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN, "Loading codec %s failed",
            codec_info->name);
    
    if(codec_array)
      lqt_destroy_codec_info(codec_array);

    return -1;
    
    }

  if(codec_array)
    lqt_destroy_codec_info(codec_array);

  //  vtrack->stream_cmodel = lqt_get_decoder_colormodel(quicktime_t * file, int track);
  
  return 0;
  
  }

int quicktime_init_acodec(quicktime_audio_map_t *atrack, int encode,
                          lqt_codec_info_t * codec_info)
  {
  lqt_codec_info_t ** codec_array = (lqt_codec_info_t**)0;
  
  char *compressor = atrack->track->mdia.minf.stbl.stsd.table[0].format;
  int wav_id       = atrack->track->mdia.minf.stbl.stsd.table[0].compression_id;
  
  /* Try to find the codec */

  if(!codec_info)
    {
    if(compressor && (*compressor != '\0'))
      {
      codec_array = lqt_find_audio_codec(compressor, encode);
      }
    else if(wav_id)
      {
      codec_array = lqt_find_audio_codec_by_wav_id(wav_id, encode);
      }
    if(!codec_array)
      {
      lqt_log(NULL, LQT_LOG_WARNING, LOG_DOMAIN,
              "Could not find audio %s for fourcc %4s",
              (encode ? "Encoder" : "Decoder"), compressor);
      }
    else
      codec_info = *codec_array;
    }

  atrack->codec = quicktime_load_codec(codec_info, atrack, NULL);
  
  /* We set the wav ids from our info structure, so we don't have to do this
     in the plugin sources */
  
  if(codec_info && codec_info->num_wav_ids)
    atrack->wav_id = codec_info->wav_ids[0];
  
  if(codec_array)
    lqt_destroy_codec_info(codec_array);
  return 0;
  }


int quicktime_delete_codec(quicktime_codec_t *codec)
  {
  codec->delete_codec(codec);
  /* Close the module */
  

  if(codec->info)
    lqt_codec_info_destroy_single(codec->info);

  free(codec);
  return 0;
  }

int quicktime_supported_video(quicktime_t *file, int track)
  {
  char *compressor = quicktime_video_compressor(file, track);
  lqt_codec_info_t ** test_codec =
    lqt_find_video_codec(compressor, file->wr);
  if(!test_codec)
    return 0;
        
  lqt_destroy_codec_info(test_codec);
  return 1;
  }

int quicktime_supported_audio(quicktime_t *file, int track)
  {
  lqt_codec_info_t ** test_codec;
  char *compressor = quicktime_audio_compressor(file, track);

  test_codec = (lqt_codec_info_t**)0;
         
  if(compressor && (*compressor != '\0'))
    test_codec = lqt_find_audio_codec(compressor, file->wr);
  else if(lqt_is_avi(file))
    test_codec = lqt_find_audio_codec_by_wav_id(lqt_get_wav_id(file, track), file->wr);
         
  if(!test_codec)
    return 0;
        
  lqt_destroy_codec_info(test_codec);
  return 1;
  }

void lqt_update_frame_position(quicktime_video_map_t * track)
  {
  track->timestamp +=
    track->track->mdia.minf.stbl.stts.table[track->stts_index].sample_duration;

  track->stts_count++;

  if(track->stts_count >=
     track->track->mdia.minf.stbl.stts.table[track->stts_index].sample_count)
    {
    track->stts_index++;
    track->stts_count = 0;
    }

  if(track->track->mdia.minf.stbl.has_ctts)
    {
    track->ctts_count++;

    if(track->ctts_count >=
       track->track->mdia.minf.stbl.ctts.table[track->ctts_index].sample_count)
      {
      track->ctts_index++;
      track->ctts_count = 0;
      }
    
    }
  
  track->current_position++;
  }

/* Set the io_rowspan for the case the user didn't. */

static void set_default_rowspan(quicktime_t *file, int track)
  {
  if(file->vtracks[track].io_row_span)
    return;

  lqt_get_default_rowspan(file->vtracks[track].io_cmodel,
                          quicktime_video_width(file, track),
                          &file->vtracks[track].io_row_span,
                          &file->vtracks[track].io_row_span_uv);
  }

/*
 *  Same as quicktime_decode_video but doesn't force BC_RGB888
 */

int lqt_decode_video(quicktime_t *file,
                     unsigned char **row_pointers, int track)
  {
  int result;
  int height;
  int width;
  set_default_rowspan(file, track);
        
  height = quicktime_video_height(file, track);
  width =  quicktime_video_width(file, track);
        
  if(file->vtracks[track].io_cmodel != file->vtracks[track].stream_cmodel)
    {
    if(!file->vtracks[track].temp_frame)
      {
      file->vtracks[track].temp_frame =
        lqt_rows_alloc(width, height, file->vtracks[track].stream_cmodel,
                       &file->vtracks[track].stream_row_span,
                       &file->vtracks[track].stream_row_span_uv);
      }
    result =
      file->vtracks[track].codec->decode_video(file,
                                               file->vtracks[track].temp_frame,
                                               track);
    cmodel_transfer(row_pointers,                    //    unsigned char **output_rows, /* Leave NULL if non existent */
                    file->vtracks[track].temp_frame, //    unsigned char **input_rows,
                    0, //                                  int in_x,        /* Dimensions to capture from input frame */
                    0, //                                  int in_y, 
                    width, //                              int in_w, 
                    height + file->vtracks[track].height_extension, // int in_h,
                    width, //                              int out_w,
                    height + file->vtracks[track].height_extension, // int out_h,
                    file->vtracks[track].stream_cmodel, // int in_colormodel,
                    file->vtracks[track].io_cmodel,     // int out_colormodel,
                    file->vtracks[track].stream_row_span,   /* For planar use the luma rowspan */
                    file->vtracks[track].io_row_span,       /* For planar use the luma rowspan */
                    file->vtracks[track].stream_row_span_uv, /* Chroma rowspan */
                    file->vtracks[track].io_row_span_uv      /* Chroma rowspan */);
         
    }
  else
    {
    file->vtracks[track].stream_row_span    = file->vtracks[track].io_row_span;
    file->vtracks[track].stream_row_span_uv = file->vtracks[track].io_row_span_uv;
          
    result = file->vtracks[track].codec->decode_video(file, row_pointers, track);
          
    }
  
  lqt_update_frame_position(&file->vtracks[track]);
  return result;
  }

/* The original function, which forces BG_RGB888 */
int quicktime_decode_video(quicktime_t *file,
                           unsigned char **row_pointers, int track)
  {
  
  file->vtracks[track].io_cmodel = BC_RGB888;
  return lqt_decode_video(file, row_pointers, track);
  }


long quicktime_decode_scaled(quicktime_t *file, 
                             int in_x,                    /* Location of input frame to take picture */
                             int in_y,
                             int in_w,
                             int in_h,
                             int out_w,                   /* Dimensions of output frame */
                             int out_h,
                             int color_model,             /* One of the color models defined above */
                             unsigned char **row_pointers, 
                             int track)
  {
  int result;

  int height;
  int width;
        
  set_default_rowspan(file, track);
  height = quicktime_video_height(file, track);
  width =  quicktime_video_width(file, track);

        
  file->vtracks[track].io_cmodel = color_model;

  if(!file->vtracks[track].temp_frame)
    {
    file->vtracks[track].temp_frame =
      lqt_rows_alloc(width, height, file->vtracks[track].stream_cmodel,
                     &file->vtracks[track].stream_row_span,
                     &file->vtracks[track].stream_row_span_uv);
    }
  result =
    file->vtracks[track].codec->decode_video(file,
                                             file->vtracks[track].temp_frame,
                                             track);
  cmodel_transfer(row_pointers,                    //    unsigned char **output_rows, /* Leave NULL if non existent */
                  file->vtracks[track].temp_frame, //    unsigned char **input_rows,
                  in_x, //                               int in_x,        /* Dimensions to capture from input frame */
                  in_y, //                               int in_y, 
                  in_w, //                               int in_w, 
                  in_h, //                               int in_h,
                  out_w, //                              int out_w, 
                  out_h, //                              int out_h,
                  file->vtracks[track].stream_cmodel, // int in_colormodel, 
                  file->vtracks[track].io_cmodel,     // int out_colormodel,
                  file->vtracks[track].stream_row_span,   /* For planar use the luma rowspan */
                  file->vtracks[track].io_row_span,       /* For planar use the luma rowspan */
                  file->vtracks[track].stream_row_span_uv, /* Chroma rowspan */
                  file->vtracks[track].io_row_span_uv      /* Chroma rowspan */);
        
  lqt_update_frame_position(&file->vtracks[track]);
  return result;
  }

int lqt_set_video_pass(quicktime_t *file,
                       int pass, int total_passes, 
                       const char * stats_file, int track)
  {
  if(file->vtracks[track].codec->set_pass)
    return file->vtracks[track].codec->set_pass(file,
                                                track,
                                                pass,
                                                total_passes,
                                                stats_file);
  else
    return 0;
  }

void lqt_start_encoding(quicktime_t *file)
  {
  int i;
  if(file->encoding_started)
    return;

  file->encoding_started = 1;

  if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
    {
    quicktime_set_position(file, 0);
    // Write RIFF chunk
    quicktime_init_riff(file);
    }
  
  /* Trigger warnings if codecs are in the wrong container */
  for(i = 0; i < file->total_atracks; i++)
    {
    if(!(file->atracks[i].codec->info->compatibility_flags & file->file_type))
      lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
              "Audio codec and container are not known to be compatible. File might be playable by libquicktime only.");
    }
  for(i = 0; i < file->total_vtracks; i++)
    {
    if(!(file->vtracks[i].codec->info->compatibility_flags & file->file_type))
      lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
              "Video codec and container are not known to be compatible. File might be playable by libquicktime only.");
    
    }
  }


static int do_encode_video(quicktime_t *file, 
                           unsigned char **row_pointers, 
                           int track)
  {
  int result;

  int height;
  int width;

  lqt_start_encoding(file);
  
  set_default_rowspan(file, track);
  height = quicktime_video_height(file, track);
  width =  quicktime_video_width(file, track);
  
  if(file->vtracks[track].io_cmodel != file->vtracks[track].stream_cmodel)
    {
    if(!file->vtracks[track].temp_frame)
      {
      file->vtracks[track].temp_frame =
        lqt_rows_alloc(width, height + file->vtracks[track].height_extension,
                       file->vtracks[track].stream_cmodel,
                       &file->vtracks[track].stream_row_span,
                       &file->vtracks[track].stream_row_span_uv);
      }
    cmodel_transfer(file->vtracks[track].temp_frame, //    unsigned char **output_rows, /* Leave NULL if non existent */
                    row_pointers,                    //    unsigned char **input_rows,
                    0, //                                  int in_x,        /* Dimensions to capture from input frame */
                    0, //                                  int in_y, 
                    width, //                              int in_w, 
                    height + file->vtracks[track].height_extension, // int in_h,
                    width, //                              int out_w, 
                    height + file->vtracks[track].height_extension, // int out_h,
                    file->vtracks[track].io_cmodel, // int in_colormodel, 
                    file->vtracks[track].stream_cmodel,     // int out_colormodel,
                    file->vtracks[track].io_row_span,   /* For planar use the luma rowspan */
                    file->vtracks[track].stream_row_span,       /* For planar use the luma rowspan */
                    file->vtracks[track].io_row_span_uv, /* Chroma rowspan */
                    file->vtracks[track].stream_row_span_uv      /* Chroma rowspan */);
    result = file->vtracks[track].codec->encode_video(file, file->vtracks[track].temp_frame, track);
    }
  else
    {
    file->vtracks[track].stream_row_span    = file->vtracks[track].io_row_span;
    file->vtracks[track].stream_row_span_uv = file->vtracks[track].io_row_span_uv;
    result = file->vtracks[track].codec->encode_video(file, row_pointers, track);
    }
  return result;
  }

void lqt_write_frame_header(quicktime_t * file, int track,
                            int pic_num1,
                            int64_t pic_pts, int keyframe)
  {
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  quicktime_trak_t * trak = vtrack->track;
  int pic_num = -1;
  int i;
  
  // fprintf(stderr, "Write frame header %d %ld\n", pic_num1, pic_pts);
  
  if(pic_num1 >= 0)
    pic_num = pic_num1;
  else
    {
    /* We start at current_position because this isn't incremented by now */
    for(i = vtrack->current_position; i >= 0; i--)
      {
      if(vtrack->timestamps[i] == pic_pts)
        {
        pic_num = i;
        break;
        }
      }
    }

  //  if(pic_num < 0)
  //    fprintf(stderr, "Picture number not found\n");
  
  if(vtrack->cur_chunk >= vtrack->picture_numbers_alloc)
    {
    vtrack->picture_numbers_alloc += 1024;
    vtrack->picture_numbers = realloc(vtrack->picture_numbers,
                                      sizeof(*vtrack->picture_numbers) *
                                      vtrack->picture_numbers_alloc);
    }
  vtrack->picture_numbers[vtrack->cur_chunk] = pic_num;
  vtrack->keyframe = keyframe;
  
  quicktime_write_chunk_header(file, trak);
  }


void lqt_write_frame_footer(quicktime_t * file, int track)
  {
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  quicktime_trak_t * trak = vtrack->track;

  trak->chunk_samples = 1;
  quicktime_write_chunk_footer(file, trak);
  
  if(vtrack->keyframe)
    quicktime_insert_keyframe(file, vtrack->cur_chunk, track);
  vtrack->cur_chunk++;
  }

void lqt_video_build_timestamp_tables(quicktime_t * file, int track)
  {
  int i;
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  quicktime_trak_t * trak = vtrack->track;
  quicktime_stts_t * stts;
  quicktime_ctts_t * ctts;
  quicktime_stts_table_t * stts_tab;
  int64_t dts;
  int has_b_frames = 0;
  
  /* If all frames are keyframes, disable stss */
  if(trak->mdia.minf.stbl.stss.total_entries == vtrack->cur_chunk)
    trak->mdia.minf.stbl.stss.total_entries = 0;
  
  /* If we have no timestamp arrays (e.g. if stream was written compressed)
     return here */

  if(!vtrack->picture_numbers)
    return;
  
  stts = &trak->mdia.minf.stbl.stts;
  ctts = &trak->mdia.minf.stbl.ctts;
  
#if 0
  fprintf (stderr, "Build timestamp tables %ld frames %d\n",
           vtrack->cur_chunk, stts->default_duration);
  for(i = 0; i < vtrack->cur_chunk; i++)
    {
    fprintf(stderr, "PTS[%d]: %ld\n", i, vtrack->timestamps[i]);
    }
  for(i = 0; i < vtrack->cur_chunk; i++)
    {
    fprintf(stderr, "pic_num[%d]: %d\n", i, vtrack->picture_numbers[i]);
    }
#endif
  /* Check if we have B-frames */
  for(i = 0; i < vtrack->cur_chunk-1; i++)
    {
    if(vtrack->picture_numbers[i] + 1 !=
       vtrack->picture_numbers[i+1])
      {
      has_b_frames = 1;
      break;
      }
    }
  
  /* Build preliminary stts */

  if(stts->table)
    {
    free(stts->table);
    stts->table = NULL;
    }
  stts->total_entries = vtrack->cur_chunk;

  if(!stts->total_entries)
    return;
  
  stts->table = malloc(vtrack->cur_chunk * sizeof(*stts->table));
  
  for(i = 0; i < vtrack->cur_chunk-1; i++)
    {
    stts->table[i].sample_count = 1;
    stts->table[i].sample_duration =
      vtrack->timestamps[i+1] - vtrack->timestamps[i];
    }

  /* Last entry */
  stts->table[vtrack->cur_chunk-1].sample_count = 1;
  stts->table[vtrack->cur_chunk-1].sample_duration =
    vtrack->duration - vtrack->timestamps[vtrack->cur_chunk-1];

  if(stts->table[vtrack->cur_chunk-1].sample_duration <= 0)
    stts->table[vtrack->cur_chunk-1].sample_duration = stts->default_duration;
  
  /* If we have no B-frames, exit here */
  if(!has_b_frames)
    return;

  /* If we have B-frames, reorder stts and build ctts */
  
  stts_tab = malloc(vtrack->cur_chunk * sizeof(*stts_tab));
  ctts->table = malloc(vtrack->cur_chunk * sizeof(*ctts->table));
  ctts->total_entries = vtrack->cur_chunk;
  trak->mdia.minf.stbl.has_ctts = 1;

  dts = 0;
  /* Loop over *coded* pictures */
  for(i = 0; i < vtrack->cur_chunk; i++)
    {
    stts_tab[i].sample_duration = stts->table[vtrack->picture_numbers[i]].sample_duration;
    stts_tab[i].sample_count = 1;

    /* CTS = PTS - DTS */
    ctts->table[i].sample_count = 1;
    ctts->table[i].sample_duration     = vtrack->timestamps[vtrack->picture_numbers[i]] - dts;
    dts += stts_tab[i].sample_duration;
    }
  
  free(stts->table);
  stts->table = stts_tab;
  }

void lqt_video_append_timestamp(quicktime_t * file, int track,
                                int64_t time, int duration)
  {
  quicktime_video_map_t * vtrack = &file->vtracks[track];
  /* Update timestamp table */

  //  fprintf(stderr, "lqt_video_append_timestamp: %ld %d\n",
  //          time, duration);

  if(vtrack->current_position >= vtrack->timestamps_alloc)
    {
    vtrack->timestamps_alloc += 1024;
    vtrack->timestamps = realloc(vtrack->timestamps,
                                 vtrack->timestamps_alloc *
                                 sizeof(*vtrack->timestamps));
    }
  vtrack->timestamps[vtrack->current_position] = time;
  vtrack->duration = time + duration;
  }


int lqt_encode_video(quicktime_t *file, 
                     unsigned char **row_pointers, 
                     int track, int64_t time)
  {
  return lqt_encode_video_d(file, row_pointers, track, time, -1);
  }

int lqt_encode_video_d(quicktime_t *file, 
                       unsigned char **row_pointers, 
                       int track, int64_t time, int duration)
  {
  int result;

  quicktime_video_map_t * vtrack = &file->vtracks[track];

  /* Must set valid timestamp for encoders */
  vtrack->timestamp = time;
  lqt_video_append_timestamp(file, track, time, duration);
  
  result = do_encode_video(file, row_pointers, track);
  if (result)
    return(result);

  if(file->io_error)
    return 1;

  if(file->vtracks[track].timecode_track)
    lqt_flush_timecode(file, track, time, 0);
  
  file->vtracks[track].current_position++;
  return 0;
  }

int quicktime_encode_video(quicktime_t *file, 
                           unsigned char **row_pointers, 
                           int track)
  {
  int result;
  
  result = lqt_encode_video_d(file, 
                              row_pointers, 
                              track, file->vtracks[track].timestamp,
                              file->vtracks[track].track->mdia.minf.stbl.stts.default_duration);
  file->vtracks[track].timestamp +=
    file->vtracks[track].track->mdia.minf.stbl.stts.default_duration;
  return result;
  }

static int bytes_per_sample(lqt_sample_format_t format)
  {
  switch(format)
    {
    case LQT_SAMPLE_INT8:
    case LQT_SAMPLE_UINT8:
      return 1;
      break;
    case LQT_SAMPLE_INT16:
      return 2;
      break;
    case LQT_SAMPLE_INT32:
      return 4;
      break;
    case LQT_SAMPLE_FLOAT: /* Float is ALWAYS machine native */
      return sizeof(float);
      break;
    case LQT_SAMPLE_DOUBLE: /* Double is ALWAYS machine native */
      return sizeof(double);
      break;
    case LQT_SAMPLE_UNDEFINED:
      return 0;
    }
  return 0;
  }

/* Decode raw samples */

int lqt_decode_audio_raw(quicktime_t *file,  void * output, long samples, int track)
  {
  int result;
  quicktime_audio_map_t * atrack;
  atrack = &file->atracks[track];
  result = atrack->codec->decode_audio(file, output, 
                                       samples,
                                       track);

  file->atracks[track].current_position += samples;
  return result;
  }

int lqt_encode_audio_raw(quicktime_t *file,  void * input, long samples, int track)
  {
  int result;
  quicktime_audio_map_t * atrack;
  if(!samples)
    return 0;
  atrack = &file->atracks[track];
  lqt_start_encoding(file);
  file->atracks[track].current_position += samples;
  result = atrack->codec->encode_audio(file, input, 
                                                               samples,
                                                               track);
  if(file->io_error)
    return 0;
  else
    return samples;
  }

/* Compatibility function for old decoding API */

static int decode_audio_old(quicktime_t *file, 
                            int16_t ** output_i, 
                            float ** output_f, 
                            long samples, 
                            int track)
  {
  int result;
  quicktime_audio_map_t * atrack;
  atrack = &file->atracks[track];

  if(atrack->sample_format == LQT_SAMPLE_UNDEFINED)
    atrack->codec->decode_audio(file, (void*)0, 
                                                        0, track);
  
  
  /* (Re)allocate sample buffer */

  if(atrack->sample_buffer_alloc < samples)
    {
    atrack->sample_buffer_alloc = samples + 1024;
    atrack->sample_buffer = realloc(atrack->sample_buffer,
                                    atrack->sample_buffer_alloc *
                                    atrack->channels *
                                    bytes_per_sample(atrack->sample_format));
    }
  
  /* Decode */

  result = atrack->codec->decode_audio(file, atrack->sample_buffer, 
                                                               samples,
                                                               track);
  
  /* Convert */
  lqt_convert_audio_decode(file, atrack->sample_buffer, output_i, output_f,
                           atrack->channels, samples,
                           atrack->sample_format);
  return result;
  }

int quicktime_decode_audio(quicktime_t *file, 
                           int16_t *output_i, 
                           float *output_f, 
                           long samples, 
                           int channel)
  {
  float   ** channels_f;
  int16_t ** channels_i;

  int quicktime_track, quicktime_channel;
  int result = 1;

  quicktime_channel_location(file, &quicktime_track,
                             &quicktime_channel, channel);

  if(file->atracks[quicktime_track].eof)
    return 1;
        
  if(output_i)
    {
    channels_i = calloc(quicktime_track_channels(file, quicktime_track), sizeof(*channels_i));
    channels_i[quicktime_channel] = output_i;
    }
  else
    channels_i = (int16_t**)0;
        
  if(output_f)
    {
    channels_f = calloc(quicktime_track_channels(file, quicktime_track), sizeof(*channels_f));
    channels_f[quicktime_channel] = output_f;
    }
  else
    channels_f = (float**)0;
        
  result = decode_audio_old(file, channels_i, channels_f, samples, quicktime_track);
  file->atracks[quicktime_track].current_position += result;

  if(channels_i)
    free(channels_i);
  else if(channels_f)
    free(channels_f);
  return ((result < 0) ? 1 : 0);
  }

int lqt_total_channels(quicktime_t *file)
  {
  int i = 0, total_channels = 0;
  for( i=0; i < file->total_atracks; i++ )
    {
    total_channels += file->atracks[i].channels;
    }

  return total_channels;
  }

/*
 * Same as quicktime_decode_audio, but it grabs all channels at
 * once. Or if you want only some channels you can leave the channels
 * you don't want = NULL in the poutput array. The poutput arrays
 * must contain at least lqt_total_channels(file) elements.
 */

int lqt_decode_audio(quicktime_t *file, 
                     int16_t **poutput_i, 
                     float **poutput_f, 
                     long samples)
  {
  int result = 1;
  int i = 0;
  int16_t **output_i;
  float   **output_f;

  int total_tracks = quicktime_audio_tracks(file);
  int track_channels;

  if(poutput_i)
    output_i = poutput_i;
  else
    output_i = (int16_t**)0;

  if(poutput_f)
    output_f = poutput_f;
  else
    output_f = (float**)0;
        
  for( i=0; i < total_tracks; i++ )
    {
    track_channels = quicktime_track_channels(file, i);

    if(file->atracks[i].eof)
      return 1;
                    
    result = decode_audio_old(file, output_i, output_f, samples, i);
    if(output_f)
      output_f += track_channels;
    if(output_i)
      output_i += track_channels;

    file->atracks[i].current_position += samples;
    }


  return result;
  }

int lqt_decode_audio_track(quicktime_t *file, 
                           int16_t **poutput_i, 
                           float **poutput_f, 
                           long samples,
                           int track)
  {
  int result = 1;

  if(file->atracks[track].eof)
    return 1;
          
  
  result = !decode_audio_old(file, poutput_i, poutput_f, samples, track);
  
  file->atracks[track].current_position += samples;
  
  return result;
  }

static int encode_audio_old(quicktime_t *file, 
                            int16_t **input_i, 
                            float **input_f, 
                            long samples,
                            int track)
  {
  quicktime_audio_map_t * atrack;
  atrack = &file->atracks[track];
  lqt_start_encoding(file);
  
  if(!samples)
    return 0;
  
  if(atrack->sample_format == LQT_SAMPLE_UNDEFINED)
    atrack->codec->encode_audio(file, (void*)0, 
                                                        0, track);

  
  /* (Re)allocate sample buffer */

  if(atrack->sample_buffer_alloc < samples)
    {
    atrack->sample_buffer_alloc = samples + 1024;
    atrack->sample_buffer = realloc(atrack->sample_buffer,
                                    atrack->sample_buffer_alloc *
                                    atrack->channels *
                                    bytes_per_sample(atrack->sample_format));
    }

  /* Convert */

  lqt_convert_audio_encode(file, input_i, input_f, atrack->sample_buffer, atrack->channels, samples,
                           atrack->sample_format);

  /* Encode */

  file->atracks[track].current_position += samples;
  
  return atrack->codec->encode_audio(file, atrack->sample_buffer,
                                                             samples, track);
  
  }

int lqt_encode_audio_track(quicktime_t *file, 
                           int16_t **input_i, 
                           float **input_f, 
                           long samples,
                           int track)
  {
  int result = 1;

  result = encode_audio_old(file, input_i, input_f, samples, track);
  return result;
  
  }

int quicktime_encode_audio(quicktime_t *file, int16_t **input_i, float **input_f, long samples)
  {
  return lqt_encode_audio_track(file, input_i, input_f, samples, 0);
  }

int quicktime_reads_cmodel(quicktime_t *file, 
                           int colormodel, 
                           int track)
  {
  return lqt_colormodel_has_conversion(file->vtracks[track].stream_cmodel, colormodel);
  }

int quicktime_writes_cmodel(quicktime_t *file, 
                            int colormodel, 
                            int track)
  {
  return lqt_colormodel_has_conversion(colormodel, file->vtracks[track].stream_cmodel);
  }

/* Compressors that can only encode a window at a time */
/* need to flush extra data here. */

static int quicktime_flush_acodec(quicktime_t *file, int track)
  {
  file->atracks[track].codec->flush(file, track);
  return 0;
  }

static void quicktime_flush_vcodec(quicktime_t *file, int track)
  {
  
  while(file->vtracks[track].codec->flush(file, track))
    ;
  }

int quicktime_codecs_flush(quicktime_t *file)
  {
  int result = 0;
  int i;
  if(!file->wr) return result;

  if(file->total_atracks)
    {
    for(i = 0; i < file->total_atracks && !result; i++)
      {
      quicktime_flush_acodec(file, i);
      }
    }

  if(file->total_vtracks)
    {
    for(i = 0; i < file->total_vtracks && !result; i++)
      {
      quicktime_flush_vcodec(file, i);
      }
    }
  return result;
  }

/* Copy audio */

int lqt_copy_audio(int16_t ** dst_i, float ** dst_f,
                   int16_t ** src_i, float ** src_f,
                   int dst_pos, int src_pos,
                   int dst_size, int src_size, int num_channels)
  {
  
  int i, j, i_tmp;
  int samples_to_copy;
  samples_to_copy = src_size < dst_size ? src_size : dst_size;

  
  if(src_i)
    {
    for(i = 0; i < num_channels; i++)
      {
      if(dst_i && dst_i[i]) /* int -> int */
        {
        memcpy(dst_i[i] + dst_pos, src_i[i] + src_pos, samples_to_copy * sizeof(int16_t));
        }
      if(dst_f && dst_f[i]) /* int -> float */
        {
        for(j = 0; j < samples_to_copy; j++)
          {
          dst_f[i][dst_pos + j] = (float)src_i[i][src_pos + j] / 32767.0;
          }
        }
      }
    }
  else if(src_f)
    {
    for(i = 0; i < num_channels; i++)
      {
      if(dst_i && dst_i[i]) /* float -> int */
        {
        for(j = 0; j < samples_to_copy; j++)
          {
          i_tmp = (int)(src_f[i][src_pos + j] * 32767.0);
          
          if(i_tmp > 32767)
            i_tmp = 32767;

          if(i_tmp < -32768)
            i_tmp = -32768;
          
          dst_i[i][dst_pos + j] = i_tmp;
          }
        }
      if(dst_f && dst_f[i]) /* float -> float */
        {
        memcpy(dst_f[i] + dst_pos, src_f[i] + src_pos, samples_to_copy * sizeof(float));
        }
      }
    }
  return samples_to_copy;
  }


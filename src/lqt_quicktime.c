/*******************************************************************************
 lqt_quicktime.c

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
#include "lqt_fseek.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>

#define LOG_DOMAIN "core"

static int64_t get_file_length(quicktime_t *file)
  {
  int64_t current_pos, total_bytes;
  current_pos = ftello(file->stream);
  fseeko(file->stream, 0, SEEK_END);
  total_bytes = ftello(file->stream);
  fseeko(file->stream, current_pos, SEEK_CUR);
  return total_bytes;
  }

int lqt_fileno(quicktime_t *file)
  {
  FILE *fp;

  fp = file->stream;
  return(fileno(fp));
  }

int quicktime_make_streamable(char *in_path, char *out_path)
  {
  quicktime_t file, *old_file, new_file;
  int moov_exists = 0, mdat_exists = 0, ftyp_exists = 0, result, atoms = 1;
  int64_t mdat_start = 0, mdat_size = 0;
  quicktime_atom_t leaf_atom;
  int64_t moov_length = 0, moov_start;
        
  memset(&new_file,0,sizeof(new_file));
        
  quicktime_init(&file);

  /* find the moov atom in the old file */
	
  if(!(file.stream = fopen(in_path, "rb")))
    {
    perror("quicktime_make_streamable");
    return 1;
    }

  file.total_length = get_file_length(&file);

  /* get the locations of moov and mdat atoms */
  do
    {
    result = quicktime_atom_read_header(&file, &leaf_atom);

    if(!result)
      {
      if(quicktime_atom_is(&leaf_atom, "moov"))
        {
        moov_exists = atoms;
        moov_length = leaf_atom.size;
        }
      else
        if(quicktime_atom_is(&leaf_atom, "ftyp"))
          {
          ftyp_exists = atoms;
          }
        else
          if(quicktime_atom_is(&leaf_atom, "mdat"))
            {
            mdat_start = quicktime_position(&file) - HEADER_LENGTH;
            mdat_size = leaf_atom.size;
            mdat_exists = atoms;
            }

      quicktime_atom_skip(&file, &leaf_atom);

      atoms++;
      }
    }while(!result && quicktime_position(&file) <
           file.total_length);
  fclose(file.stream);

  if(!moov_exists)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
            "quicktime_make_streamable: no moov atom");
    return 1;
    }

  if(!mdat_exists)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
            "quicktime_make_streamable: no mdat atom");
    return 1;
    }

  /* copy the old file to the new file */
  if(moov_exists && mdat_exists)
    {
    /* moov comes after mdat */
    if(moov_exists > mdat_exists)
      {
      uint8_t *buffer;
      int64_t buf_size = 1000000;

      result = 0;

      /* read the header proper */
      if(!(old_file = quicktime_open(in_path, 1, 0)))
        {
        return 1;
        }

      quicktime_shift_offsets(&old_file->moov, moov_length+8);

      /* open the output file */
			
			
      if(!(new_file.stream = fopen(out_path, "wb")))
        {
        lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
                "quicktime_make_streamable: cannot open output file: %s",
                strerror(errno));
        result =  1;
        }
      else
        {
        /* set up some flags */
        new_file.wr = 1;
        new_file.rd = 0;
        new_file.presave_buffer = calloc(1, QUICKTIME_PRESAVE);
        new_file.file_type = old_file->file_type;
        if(old_file->has_ftyp)
          quicktime_write_ftyp(&new_file, &old_file->ftyp);
                        
        moov_start = quicktime_position(&new_file);
        quicktime_write_moov(&new_file, &old_file->moov);

        if(moov_length !=
           quicktime_position(&new_file) - moov_start)
          {
          lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
                  "quicktime_make_streamable: moov size changed from %"PRId64" to %"PRId64" (Pos: %"PRId64", start: %"PRId64")",
                  moov_length, quicktime_position(&new_file) - moov_start,
                  quicktime_position(&new_file), moov_start);
          quicktime_set_position(&new_file, moov_start + moov_length);
          }
                          
				

        quicktime_atom_write_header64(&new_file, 
                                         
                                      &new_file.mdat.atom, 
                                      "mdat");
				
        quicktime_set_position(old_file, mdat_start);

        if(!(buffer = calloc(1, buf_size)))
          {
          result = 1;
          printf("quicktime_make_streamable: out of memory\n");
          }
        else
          {
          while(quicktime_position(old_file) < mdat_start +
                mdat_size && !result)
            {
            if(quicktime_position(old_file) + buf_size >
               mdat_start + mdat_size)
              buf_size = mdat_start + mdat_size -
                quicktime_position(old_file);

            if(!quicktime_read_data(old_file, buffer, buf_size))
              result = 1;
            if(!result)
              {
              if(!quicktime_write_data(&new_file, buffer,
                                       buf_size)) result = 1;
 
              }
            }
          free(buffer);
          }

        quicktime_atom_write_footer(&new_file, 
                                         
                                    &new_file.mdat.atom);
			
				
        if(new_file.presave_size)
          {
          quicktime_fseek(&new_file,
                          new_file.presave_position - new_file.presave_size);
          fwrite(new_file.presave_buffer, 1,
                 new_file.presave_size, new_file.stream);
          new_file.presave_size = 0;
          }
        free(new_file.presave_buffer);
        fclose(new_file.stream);
        }
      quicktime_close(old_file);
      }
    else
      {
      printf("quicktime_make_streamable: header already at 0 offset\n");
      return 0;
      }
    }
	
  return 0;
  }




void lqt_set_audio_parameter(quicktime_t *file,int stream, const char *key,
                             const void *value)
  {
  quicktime_codec_t *codec = file->atracks[stream].codec;
  if(codec->set_parameter) codec->set_parameter(file, stream, key, value);
  }

void lqt_set_video_parameter(quicktime_t *file,int stream, const char *key,
                             const void *value)
  {
  quicktime_codec_t *codec = file->vtracks[stream].codec;
  if(codec->set_parameter) codec->set_parameter(file, stream, key, value);
  }

void quicktime_set_parameter(quicktime_t *file, char *key, void *value)
  {
  int i;
  for(i = 0; i < file->total_vtracks; i++)
    {
    lqt_set_video_parameter(file,i, key,value);
    }

  for(i = 0; i < file->total_atracks; i++)
    {
    lqt_set_audio_parameter(file,i, key,value);
    }
  }



void quicktime_set_jpeg(quicktime_t *file, int quality, int use_float)
  {
  quicktime_set_parameter( file, "jpeg_quality", &quality );
  quicktime_set_parameter( file, "jpeg_usefloat", &use_float );
  }


void quicktime_set_copyright(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.copyright,
                            &file->moov.udta.copyright_len, string);
  }

void quicktime_set_name(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.name,
                            &file->moov.udta.name_len, string);
  }

void quicktime_set_info(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.info, &file->moov.udta.info_len, string);
  }

char* quicktime_get_copyright(quicktime_t *file)
  {
  return file->moov.udta.copyright;
  }

char* quicktime_get_name(quicktime_t *file)
  {
  return file->moov.udta.name;
  }

char* quicktime_get_info(quicktime_t *file)
  {
  return file->moov.udta.info;
  }

/* Extended metadata support */

void lqt_set_album(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.album,
                            &file->moov.udta.album_len, string);
  }

void lqt_set_artist(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.artist,
                            &file->moov.udta.artist_len, string);
  }

void lqt_set_genre(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.genre,
                            &file->moov.udta.genre_len, string);
  }

void lqt_set_track(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.track,
                            &file->moov.udta.track_len, string);
  }

void lqt_set_comment(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.comment,
                            &file->moov.udta.comment_len, string);
  }

void lqt_set_author(quicktime_t *file, char *string)
  {
  quicktime_set_udta_string(&file->moov.udta.author,
                            &file->moov.udta.author_len, string);
  }

void lqt_set_creation_time(quicktime_t *file, unsigned long time)
  {
  file->moov.mvhd.creation_time = time;
  }

char * lqt_get_album(quicktime_t * file)
  {
  return file->moov.udta.album;
  }

char * lqt_get_artist(quicktime_t * file)
  {
  return file->moov.udta.artist;
  }

char * lqt_get_genre(quicktime_t * file)
  {
  return file->moov.udta.genre;
  }

char * lqt_get_track(quicktime_t * file)
  {
  return file->moov.udta.track;
  }

char * lqt_get_comment(quicktime_t *file)
  {
  return file->moov.udta.comment;
  }

char * lqt_get_author(quicktime_t *file)
  {
  return file->moov.udta.author;
  }

unsigned long lqt_get_creation_time(quicktime_t * file)
  {
  return file->moov.mvhd.creation_time;
  }


int quicktime_video_tracks(quicktime_t *file)
  {
  int i, result = 0;
  for(i = 0; i < file->moov.total_tracks; i++)
    {
    if(file->moov.trak[i]->mdia.minf.is_video) result++;
    }
  return result;
  }

int quicktime_audio_tracks(quicktime_t *file)
  {
  int i, result = 0;
  quicktime_minf_t *minf;
  for(i = 0; i < file->moov.total_tracks; i++)
    {
    minf = &file->moov.trak[i]->mdia.minf;
    if(minf->is_audio)
      result++;
    }
  return result;
  }

int lqt_set_audio_codec(quicktime_t *file, int track,
                        lqt_codec_info_t * info)
  {
  quicktime_stsd_t * stsd;
  
  stsd = &file->atracks[track].track->mdia.minf.stbl.stsd;
  
  quicktime_stsd_set_audio_codec(stsd, info->fourccs[0]);
  
  quicktime_init_audio_map(file, &file->atracks[track],
                           file->wr,
                           info);
  lqt_set_default_audio_parameters(file, track);
  return 0;
  }

int lqt_add_audio_track_internal(quicktime_t *file,
                                 int channels, long sample_rate, int bits,
                                 lqt_codec_info_t * codec_info,
                                 const lqt_compression_info_t * ci)
  {
  quicktime_trak_t *trak;
  char * compressor = codec_info ? codec_info->fourccs[0] : NULL;
  
  file->atracks = realloc(file->atracks,
                          (file->total_atracks+1)*sizeof(*file->atracks));

  memset(&file->atracks[file->total_atracks], 0, sizeof(*file->atracks));
  
  if(ci)
    lqt_compression_info_copy(&file->atracks[file->total_atracks].ci, ci);
  
  trak = quicktime_add_track(file);
  quicktime_trak_init_audio(file, trak, channels,
                            sample_rate, bits, compressor);
  file->atracks[file->total_atracks].track = trak;
  
  file->total_atracks++;
  
  if(codec_info)
    return lqt_set_audio_codec(file, file->total_atracks-1,
                               codec_info);
  return 0;
  }

int lqt_add_audio_track(quicktime_t *file,
                        int channels, long sample_rate, int bits,
                        lqt_codec_info_t * codec_info)
  {
  return lqt_add_audio_track_internal(file,
                                      channels,
                                      sample_rate,
                                      bits,
                                      codec_info, NULL);
  }

int lqt_set_audio(quicktime_t *file, int channels,
                  long sample_rate,  int bits,
                  lqt_codec_info_t * codec_info)
  {
  lqt_add_audio_track(file, channels, sample_rate, bits, codec_info);
  return 0;
  }

int quicktime_set_audio(quicktime_t *file, 
                        int channels,
                        long sample_rate,
                        int bits,
                        char *compressor)
  {
  lqt_codec_info_t ** info;
  info = lqt_find_audio_codec(compressor, 1);
  lqt_set_audio(file, channels, sample_rate, bits, *info);
  lqt_destroy_codec_info(info);
  return 1;   /* Return the number of tracks created */
  }

int lqt_set_video(quicktime_t *file, 
                  int tracks, 
                  int frame_w, 
                  int frame_h,
                  int frame_duration,
                  int timescale,
                  lqt_codec_info_t * info)
  {
  int i;

  for(i = 0; i < tracks; i++)
    {
    if(lqt_add_video_track(file, frame_w, frame_h,
                           frame_duration, timescale, info))
      return 1;
    }
  return 0;
  }

int quicktime_set_video(quicktime_t *file, 
                        int tracks, 
                        int frame_w, 
                        int frame_h,
                        double frame_rate,
                        char *compressor)
  {
  lqt_codec_info_t ** info;
  int timescale, frame_duration;
  timescale = quicktime_get_timescale(frame_rate);
  frame_duration = (int)((double)(timescale)/frame_rate+0.5);
  info = lqt_find_video_codec(compressor, 1);
  lqt_set_video(file, tracks, frame_w, frame_h, frame_duration, timescale, *info);
  lqt_destroy_codec_info(info);
  return 0;
  }

static int check_image_size(lqt_codec_info_t * info,
                            int frame_w, int frame_h)
  {
  int i;
  if(info->num_image_sizes)
    {
    for(i = 0; i < info->num_image_sizes; i++)
      {
      if((frame_w == info->image_sizes[i].width) &&
         (frame_h == info->image_sizes[i].height))
        return 1;
      }
    
    return 0;
    }
  else
    return 1;
  }

int lqt_set_video_codec(quicktime_t *file, int track,
                        lqt_codec_info_t * info)
  {
  quicktime_stsd_t *stsd;
  
  if(!check_image_size(info,
                       quicktime_video_width(file, track),
                       quicktime_video_height(file, track)))
    return 1;

  stsd = &file->vtracks[track].track->mdia.minf.stbl.stsd;
  
  quicktime_stsd_set_video_codec(stsd, info->fourccs[0]);
  
  quicktime_init_video_map(&file->vtracks[track],
                           file->wr, info);
  
  lqt_set_default_video_parameters(file, track);
  
  /* Get encoding colormodel */
  file->vtracks[file->total_vtracks-1].codec->encode_video(file,
                                                           (uint8_t**)0,
                                                           track);
  file->vtracks[track].io_cmodel =
    file->vtracks[track].stream_cmodel;
  
  
  return 0;
  }

int lqt_add_video_track_internal(quicktime_t *file,
                                 int frame_w, int frame_h,
                                 int frame_duration, int timescale,
                                 lqt_codec_info_t * info,
                                 const lqt_compression_info_t * ci)
  {
  char * compressor = info ? info->fourccs[0] : NULL;
  quicktime_trak_t *trak;

  /* Check if the image size is supported */
  if(info && !check_image_size(info, frame_w, frame_h))
    {
    lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
            "Adding video track failed, unsupported image size");
    return 1;
    }
  
  if(!file->total_vtracks)
    quicktime_mhvd_init_video(file, &file->moov.mvhd, timescale);
  file->vtracks = realloc(file->vtracks, (file->total_vtracks+1) *
                          sizeof(*file->vtracks));
  memset(&file->vtracks[file->total_vtracks], 0, sizeof(*file->vtracks));
  
  if(ci)
    {
    lqt_compression_info_copy(&file->vtracks[file->total_vtracks].ci, ci);
    file->vtracks[file->total_vtracks].stream_cmodel = ci->colormodel;
    }
  trak = quicktime_add_track(file);
  file->vtracks[file->total_vtracks].track = trak;
  
  file->total_vtracks++;
        
  quicktime_trak_init_video(file, trak, frame_w, frame_h,
                            frame_duration, timescale, compressor);


  if(info)
    return lqt_set_video_codec(file, file->total_vtracks-1, info);
  
  return 0;
  }

int lqt_add_video_track(quicktime_t *file,
                        int frame_w, int frame_h,
                        int frame_duration, int timescale,
                        lqt_codec_info_t * info)
  {
  return lqt_add_video_track_internal(file,
                                      frame_w, frame_h,
                                      frame_duration, timescale,
                                      info, NULL);
  }

void quicktime_set_framerate(quicktime_t *file, double framerate)
  {
  int i;
  int new_time_scale, new_sample_duration;

  if(!file->wr)
    {
    lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN,
            "quicktime_set_framerate shouldn't be called in read mode.");
    return;
    }

  new_time_scale = quicktime_get_timescale(framerate);
  new_sample_duration = (int)((float)new_time_scale / framerate + 0.5);

  for(i = 0; i < file->total_vtracks; i++)
    {
    file->vtracks[i].track->mdia.mdhd.time_scale = new_time_scale;
    file->vtracks[i].track->mdia.minf.stbl.stts.table[0].sample_duration =
      new_sample_duration;
    }
  }

/* Used for writing only */
quicktime_trak_t* quicktime_add_track(quicktime_t *file)
  {
  quicktime_moov_t *moov = &file->moov;
  quicktime_trak_t *trak;
  
  //  for(i = moov->total_tracks; i > 0; i--)
  //    moov->trak[i] = moov->trak[i - 1];
  
  trak =
    moov->trak[moov->total_tracks] =
    calloc(1, sizeof(quicktime_trak_t));

  quicktime_trak_init(trak, file->file_type);

  moov->trak[moov->total_tracks]->tkhd.track_id = moov->mvhd.next_track_id;
  
  moov->total_tracks++;
  moov->mvhd.next_track_id++;
  return trak;
  }

/* ============================= Initialization functions */

int quicktime_init(quicktime_t *file)
  {
  bzero(file, sizeof(quicktime_t));
  //	quicktime_atom_write_header64(new_file, &file->mdat.atom, "mdat");
  quicktime_moov_init(&file->moov);
  file->max_riff_size = 0x40000000;
  //	file->color_model = BC_RGB888;
  return 0;
  }

static int quicktime_delete(quicktime_t *file)
  {
  int i;
  if(file->total_atracks) 
    {
    for(i = 0; i < file->total_atracks; i++)
      quicktime_delete_audio_map(&file->atracks[i]);
    free(file->atracks);
    }
  if(file->total_vtracks)
    {
    for(i = 0; i < file->total_vtracks; i++)
      quicktime_delete_video_map(&file->vtracks[i]);
    free(file->vtracks);
    }
  if(file->total_ttracks)
    {
    for(i = 0; i < file->total_ttracks; i++)
      lqt_delete_text_map(file, &file->ttracks[i]);
    free(file->ttracks);
    }
  file->total_atracks = 0;
  file->total_vtracks = 0;

  if(file->moov_data)
    free(file->moov_data);
        
  if(file->preload_size)
    {
    free(file->preload_buffer);
    file->preload_size = 0;
    }
  if(file->presave_buffer)
    {
    free(file->presave_buffer);
    }
  for(i = 0; i < file->total_riffs; i++)
    {
    quicktime_delete_riff(file, file->riff[i]);
    }

  quicktime_moov_delete(&file->moov);
  quicktime_mdat_delete(&file->mdat);
  quicktime_ftyp_delete(&file->ftyp);
  return 0;
  }

/* =============================== Optimization functions */

int quicktime_set_cpus(quicktime_t *file, int cpus)
  {
  return 0;
  }

void quicktime_set_preload(quicktime_t *file, int64_t preload)
  {
  file->preload_size = preload;
  if(file->preload_buffer) free(file->preload_buffer);
  file->preload_buffer = 0;
  if(preload)
    file->preload_buffer = calloc(1, preload);
  file->preload_start = 0;
  file->preload_end = 0;
  file->preload_ptr = 0;
  }


int quicktime_get_timescale(double frame_rate)
  {
  int timescale = 600;
  /* Encode the 29.97, 23.976, 59.94 framerates */
  if(frame_rate - (int)frame_rate != 0) 
    timescale = (int)(frame_rate * 1001 + 0.5);
  else
    if((600 / frame_rate) - (int)(600 / frame_rate) != 0) 
      timescale = (int)(frame_rate * 100 + 0.5);
  return timescale;
  }


int quicktime_seek_start(quicktime_t *file)
  {
  int i;
  for(i = 0; i < file->total_atracks; i++)
    quicktime_set_audio_position(file, 0, i);
  for(i = 0; i < file->total_vtracks; i++)
    quicktime_set_video_position(file, 0, i);
        
        
  return 0;
  }

long quicktime_audio_length(quicktime_t *file, int track)
  {
  if(file->total_atracks > 0) 
    return file->atracks[track].total_samples;

  return 0;
  }

long quicktime_video_length(quicktime_t *file, int track)
  {
  /*printf("quicktime_video_length %d %d\n", quicktime_track_samples(file, file->vtracks[track].track), track); */
  if(file->total_vtracks > 0)
    return quicktime_track_samples(file, file->vtracks[track].track);
  return 0;
  }

long quicktime_audio_position(quicktime_t *file, int track)
  {
  return file->atracks[track].current_position;
  }

long quicktime_video_position(quicktime_t *file, int track)
  {
  return file->vtracks[track].current_position;
  }

int quicktime_set_audio_position(quicktime_t *file, int64_t sample, int track)
  {
  if((track >= 0) && (track < file->total_atracks))
    {
    /* We just set the current position, Codec will do the rest */
    
    file->atracks[track].current_position = sample;
    file->atracks[track].eof = 0;
    }
  else
    lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
            "quicktime_set_audio_position: track >= file->total_atracks\n");
  return 0;
  }

int quicktime_set_video_position(quicktime_t *file, int64_t frame, int track)
  {
  int64_t chunk_sample, chunk;
  quicktime_trak_t *trak;
  quicktime_codec_t * codec;
  
  if((track < 0) || (track >= file->total_vtracks))
    return 0;

  trak = file->vtracks[track].track;

  if((frame < 0) || (frame >= quicktime_track_samples(file, trak)))
    return 0;
  
  file->vtracks[track].current_position = frame;
  quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, frame);
  file->vtracks[track].cur_chunk = chunk;
  
  file->vtracks[track].timestamp =
    quicktime_sample_to_time(&trak->mdia.minf.stbl.stts,
                             frame,
                             &file->vtracks[track].stts_index,
                             &file->vtracks[track].stts_count);

  /* Resync codec */

  codec = file->vtracks[track].codec;
  if(codec && codec->resync)
    codec->resync(file, track);
    
  return 0;
  }

void lqt_seek_video(quicktime_t * file, int track, int64_t time)
  {
  int64_t frame;
  quicktime_trak_t *trak;

  if((track < 0) || (track >= file->total_vtracks))
    return;
  
  trak = file->vtracks[track].track;

  file->vtracks[track].timestamp = time;
  
  frame =
    quicktime_time_to_sample(&trak->mdia.minf.stbl.stts,
                             &file->vtracks[track].timestamp,
                             &file->vtracks[track].stts_index,
                             &file->vtracks[track].stts_count);


  quicktime_set_video_position(file, frame, track);
  }


#define FRAME_PADDING 128

int lqt_read_video_frame(quicktime_t * file,
                         uint8_t ** buffer, int * buffer_alloc,
                         int64_t frame, int64_t * time, int track)
  {
  int64_t offset, chunk_sample, chunk;
  int result;
  quicktime_trak_t *trak;
  int len;
  
  if((track >= file->total_vtracks) || (track < 0))
    return 0;
  
  trak = file->vtracks[track].track;

  if((frame < 0) || (frame >= quicktime_track_samples(file, trak)))
    return 0;
  
  //  file->vtracks[track].current_position = frame;
  quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, frame);
  file->vtracks[track].cur_chunk = chunk;
  offset = quicktime_sample_to_offset(file, trak, frame);
  quicktime_set_position(file, offset);

  if(time)
    *time = quicktime_sample_to_time(&trak->mdia.minf.stbl.stts,
                                     frame,
                                     &file->vtracks[track].stts_index,
                                     &file->vtracks[track].stts_count);

  len = quicktime_frame_size(file, frame, track);
  
  if(len + FRAME_PADDING > *buffer_alloc)
    {
    *buffer_alloc = len + FRAME_PADDING + 1024;
    *buffer = realloc(*buffer, *buffer_alloc);
    }

  result = quicktime_read_data(file, *buffer, len);


  if(result < len)
    {
    return 0;
    }
  memset(*buffer + len, 0, FRAME_PADDING);
  return len;
  }

#undef FRAME_PADDING


int quicktime_has_audio(quicktime_t *file)
  {
  if(quicktime_audio_tracks(file)) return 1;
  return 0;
  }

long quicktime_sample_rate(quicktime_t *file, int track)
  {
  if(file->total_atracks)
    return file->atracks[track].samplerate;
  return 0;
  }

int quicktime_audio_bits(quicktime_t *file, int track)
  {
  if(file->total_atracks)
    return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_size;

  return 0;
  }

char* quicktime_audio_compressor(quicktime_t *file, int track)
  {
  return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].format;
  }

int quicktime_track_channels(quicktime_t *file, int track)
  {
  if(track < file->total_atracks)
    return file->atracks[track].channels;

  return 0;
  }

int quicktime_channel_location(quicktime_t *file,
                               int *quicktime_track,
                               int *quicktime_channel, int channel)
  {
  int current_channel = 0, current_track = 0;
  *quicktime_channel = 0;
  *quicktime_track = 0;
  for(current_channel = 0, current_track = 0; current_track < file->total_atracks; )
    {
    if(channel >= current_channel)
      {
      *quicktime_channel = channel - current_channel;
      *quicktime_track = current_track;
      }

    current_channel += file->atracks[current_track].channels;
    current_track++;
    }
  return 0;
  }

int quicktime_has_video(quicktime_t *file)
  {
  if(quicktime_video_tracks(file)) return 1;
  return 0;
  }

int quicktime_video_width(quicktime_t *file, int track)
  {
  //	if(file->total_vtracks)
  //          return file->vtracks[track].track->tkhd.track_width;
  //	return 0;
  if((track < 0) || (track >= file->total_vtracks))
    return 0;
  return file->vtracks[track].track->mdia.minf.stbl.stsd.table->width;
  
  }

int quicktime_video_height(quicktime_t *file, int track)
  {
  //	if(file->total_vtracks)
  //          return file->vtracks[track].track->tkhd.track_height;
  //	return 0;
  if((track < 0) || (track >= file->total_vtracks))
    return 0;
  return file->vtracks[track].track->mdia.minf.stbl.stsd.table->height;
  }

int quicktime_video_depth(quicktime_t *file, int track)
  {
  if(file->total_vtracks)
    return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].depth;
  return 0;
  }

long lqt_video_edit_list_total_entries(quicktime_t * file, int track)
{
   if(track < 0 || track >= quicktime_video_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }
      
   return file->vtracks[track].track->edts.elst.total_entries;
}

long lqt_video_edit_duration(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_video_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }

   if(entry_index < 0 || entry_index >= file->vtracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0;
   }

   // convert to media timescale
   return (long)((double)file->vtracks[track].track->edts.elst.table[entry_index].duration / file->moov.mvhd.time_scale * file->vtracks[track].track->mdia.mdhd.time_scale + 0.5);
}

long lqt_video_edit_time(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_video_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }

   if(entry_index < 0 || entry_index >= file->vtracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0;
   }

   return file->vtracks[track].track->edts.elst.table[entry_index].time;
}

float lqt_video_edit_rate(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_video_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0.0f;
   }

   if(entry_index < 0 || entry_index >= file->vtracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0.0f;
   }

   return file->vtracks[track].track->edts.elst.table[entry_index].rate;
}

void quicktime_set_cmodel(quicktime_t *file, int colormodel)
  {
  int i;
  for(i = 0; i < file->total_vtracks; i++)
    file->vtracks[i].io_cmodel = colormodel;
  }

int lqt_get_cmodel(quicktime_t * file, int track)
  {
  if((track < file->total_vtracks) && (track >= 0))
    return file->vtracks[track].io_cmodel;
  else
    return LQT_COLORMODEL_NONE;
  }


void lqt_set_cmodel(quicktime_t *file, int track, int colormodel)
  {
  if((track < file->total_vtracks) && (track >= 0))
    {
    file->vtracks[track].io_cmodel = colormodel;

    /* Maybe switch the encoding colormodel to better match the IO one. */
    if(file->wr && !file->encoding_started)
      {
      lqt_codec_info_t * info = file->vtracks[track].codec->info;
      int encoding_cmodel = lqt_get_best_target_colormodel(
              colormodel, info->encoding_colormodels);
      
      if (encoding_cmodel != LQT_COLORMODEL_NONE)
        {
        file->vtracks[track].stream_cmodel = encoding_cmodel;
        }
      }
    }
  else
    lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "lqt_set_cmodel: No track No. %d", track);
  }

void quicktime_set_row_span(quicktime_t *file, int row_span)
  {
  int i;
  for(i = 0; i < file->total_vtracks; i++)
    file->vtracks[i].io_row_span = row_span;
  }

void lqt_set_row_span(quicktime_t *file, int track, int row_span)
  {
  file->vtracks[track].io_row_span = row_span;
  }

void lqt_set_row_span_uv(quicktime_t *file, int track, int row_span_uv)
  {
  file->vtracks[track].io_row_span_uv = row_span_uv;
  }
#if 0
static void quicktime_set_window(quicktime_t *file,
                                 int in_x, /* Location of input frame to take picture */
                                 int in_y,
                                 int in_w,
                                 int in_h,
                                 int out_w, /* Dimensions of output frame */
                                 int out_h)
  {
  if(in_x >= 0 && in_y >= 0 && in_w > 0 && in_h > 0 && out_w > 0 && out_h > 0)
    {
    file->in_x = in_x;
    file->in_y = in_y;
    file->in_w = in_w;
    file->in_h = in_h;
    file->out_w = out_w;
    file->out_h = out_h;
    }
  }
#endif

void quicktime_set_depth(quicktime_t *file, int depth, int track)
  {
  int i;

  for(i = 0; i < file->total_vtracks; i++)
    {
    file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].depth = depth;
    }
  }

double quicktime_frame_rate(quicktime_t *file, int track)
  {
  if(file->total_vtracks > track)
    {
    if(file->vtracks[track].track->mdia.minf.stbl.stts.table)
      return (float)file->vtracks[track].track->mdia.mdhd.time_scale / 
        file->vtracks[track].track->mdia.minf.stbl.stts.table[0].sample_duration;
    else
      return (float)file->vtracks[track].track->mdia.mdhd.time_scale / 
        file->vtracks[track].track->mdia.minf.stbl.stts.default_duration;
    }
  return 0;
  }

int64_t lqt_get_frame_time(quicktime_t * file, int track, int frame)
  {
  int64_t dummy1;
  int64_t dummy2;
  return
    quicktime_sample_to_time(&file->vtracks[track].track->mdia.minf.stbl.stts, frame,
                             &dummy1, &dummy2);
  }

/*
 *  Return the timestamp of the NEXT frame to be decoded.
 *  Call this BEFORE one of the decoding functions.
 */
  
int64_t lqt_frame_time(quicktime_t * file, int track)
  {
  return file->vtracks[track].timestamp;
  }

/*
 *  Return the Duration of the entire track
 */

int64_t lqt_video_duration(quicktime_t * file, int track)
  {
  int64_t dummy1;
  int64_t dummy2;
  return
    quicktime_sample_to_time(&file->vtracks[track].track->mdia.minf.stbl.stts, -1,
                             &dummy1, &dummy2);
  }


/*
 *  Get the timescale of the track. Divide the return values
 *  of lqt_frame_duration and lqt_frame_time by the scale to
 *  get the time in seconds.
 */
  
int lqt_video_time_scale(quicktime_t * file, int track)
  {
  if(file->total_vtracks <= track)
    return 0;
  return file->vtracks[track].track->mdia.mdhd.time_scale;
  }

/*
 *  Get the duration of the NEXT frame to be decoded.
 *  If constant is not NULL it will be set to 1 if the
 *  frame duration is constant throughout the whole track
 */

int lqt_frame_duration(quicktime_t * file, int track, int *constant)
  {
  if(file->total_vtracks <= track)
    return 0;

  if(constant)
    {
    if(file->vtracks[track].track->mdia.minf.stbl.stts.total_entries == 1)
      *constant = 1;
    else if((file->vtracks[track].track->mdia.minf.stbl.stts.total_entries == 2) && 
            (file->vtracks[track].track->mdia.minf.stbl.stts.table[1].sample_count == 1))
      *constant = 1;
    else
      *constant = 0;
    }
  return
    file->vtracks[track].track->mdia.minf.stbl.stts.table[file->vtracks[track].stts_index].sample_duration;
  }


char* quicktime_video_compressor(quicktime_t *file, int track)
  {
  if ((track < 0) || (track >= file->total_vtracks))
    return NULL;
  return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].format;
  }

int quicktime_write_audio(quicktime_t *file, 
                          uint8_t *audio_buffer, 
                          long samples, 
                          int track)
  {
  int result;
  int64_t bytes;
  quicktime_audio_map_t *track_map = &file->atracks[track];
  quicktime_trak_t *trak = track_map->track;
                                                                                                                  
  /* write chunk for 1 track */
  bytes = samples * quicktime_audio_bits(file, track) / 8 * file->atracks[track].channels;
  quicktime_write_chunk_header(file, trak);
  result = !quicktime_write_data(file, audio_buffer, bytes);

  trak->chunk_samples = samples;
  quicktime_write_chunk_footer(file, trak);
  
  /*      file->atracks[track].current_position += samples; */
  file->atracks[track].cur_chunk++;
  return result;
  }

long lqt_audio_edit_list_total_entries(quicktime_t * file, int track)
{
   if(track < 0 || track >= quicktime_audio_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }
      
   return file->atracks[track].track->edts.elst.total_entries;
}

long lqt_audio_edit_duration(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_audio_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }

   if(entry_index < 0 || entry_index >= file->atracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0;
   }

   // convert to media timescale
   return (long)((double)file->atracks[track].track->edts.elst.table[entry_index].duration / file->moov.mvhd.time_scale * file->atracks[track].track->mdia.mdhd.time_scale + 0.5);
}

long lqt_audio_edit_time(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_audio_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0;
   }

   if(entry_index < 0 || entry_index >= file->atracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0;
   }

   return file->atracks[track].track->edts.elst.table[entry_index].time;
}

float lqt_audio_edit_rate(quicktime_t * file, int track, int entry_index)
{
   if(track < 0 || track >= quicktime_audio_tracks(file)) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal track index");
      return 0.0f;
   }

   if(entry_index < 0 || entry_index >= file->atracks[track].track->edts.elst.total_entries) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN, "illegal edit list entry");
      return 0.0f;
   }

   return file->atracks[track].track->edts.elst.table[entry_index].rate;
}

int quicktime_write_frame(quicktime_t *file,
                          unsigned char *video_buffer,
                          int64_t bytes, int track)
  {
  int result = 0;
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  
  lqt_video_append_timestamp(file, track, vtrack->timestamp,
                             file->vtracks[track].track->mdia.minf.stbl.stts.default_duration);
  vtrack->timestamp += file->vtracks[track].track->mdia.minf.stbl.stts.default_duration;

  lqt_write_frame_header(file, track, file->vtracks[track].current_position,
                         -1, 0 /* int keyframe */ );
  
  result = !quicktime_write_data(file, video_buffer, bytes);

  lqt_write_frame_footer(file, track);
  
  if(file->vtracks[track].timecode_track)
    lqt_flush_timecode(file, track,
                       file->vtracks[track].current_position*
                       (int64_t)file->vtracks[track].track->mdia.minf.stbl.stts.default_duration, 0);
  
  file->vtracks[track].current_position++;
  return result;
  }

long quicktime_frame_size(quicktime_t *file, long frame, int track)
  {
  long bytes = 0;
  quicktime_trak_t *trak = file->vtracks[track].track;

  if(trak->mdia.minf.stbl.stsz.sample_size)
    {
    bytes = trak->mdia.minf.stbl.stsz.sample_size;
    }
  else
    {
    long total_frames = quicktime_track_samples(file, trak);
    if(frame < 0) frame = 0;
    else
      if(frame > total_frames - 1) frame = total_frames - 1;
    bytes = trak->mdia.minf.stbl.stsz.table[frame].size;
    }


  return bytes;
  }


int quicktime_read_frame_init(quicktime_t *file, int track)
  {
  int64_t offset;
  quicktime_trak_t *trak = file->vtracks[track].track;
  offset = quicktime_sample_to_offset(file, trak, file->vtracks[track].current_position);
  quicktime_set_position(file, offset);
  
  if(quicktime_ftell(file) != file->file_position) 
    {
    fseeko(file->stream, file->file_position, SEEK_SET);
    file->ftell_position = file->file_position;
    }
  return 0;
  }

int quicktime_read_frame_end(quicktime_t *file, int track)
  {
  file->file_position = quicktime_ftell(file);
  file->vtracks[track].current_position++;
  return 0;
  }


long quicktime_read_frame(quicktime_t *file, unsigned char *video_buffer, int track)
  {
  int64_t bytes, offset, chunk_sample, chunk;
  int result = 0;
  quicktime_trak_t *trak = file->vtracks[track].track;
    
  bytes = quicktime_frame_size(file, file->vtracks[track].current_position, track);

  quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, file->vtracks[track].current_position);
  file->vtracks[track].cur_chunk = chunk;
  offset = quicktime_sample_to_offset(file, trak, file->vtracks[track].current_position);
  quicktime_set_position(file, offset);
          
  result = quicktime_read_data(file, video_buffer, bytes);
  lqt_update_frame_position(&file->vtracks[track]);

  if(!result) return 0;
  return bytes;
  }

long quicktime_get_keyframe_before(quicktime_t *file, long frame, int track)
  {
  quicktime_trak_t *trak = file->vtracks[track].track;
  quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
  int i;
  
  // Offset 1
  frame++;
  
  for(i = stss->total_entries - 1; i >= 0; i--)
    {
    if(stss->table[i].sample <= frame) return stss->table[i].sample - 1;
    }

  return 0;
  }

#if 0
static long quicktime_get_keyframe_after(quicktime_t *file, long frame, int track)
  {
  quicktime_trak_t *trak = file->vtracks[track].track;
  quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
  int i;
  // Offset 1
  frame++;
  for(i = 0; i < stss->total_entries; i++)
    {
    if(stss->table[i].sample >= frame) return stss->table[i].sample - 1;
    }
  return 0;
  }
#endif

void quicktime_insert_keyframe(quicktime_t *file, long frame, int track)
  {
  quicktime_trak_t *trak = file->vtracks[track].track;
  quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;

  if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
    {
    // Set keyframe flag in idx1 table.
    if(file->total_riffs == 1)
      quicktime_set_idx1_keyframe(file, trak, frame);
    // Set keyframe flag in indx table.
    if(file->file_type == LQT_FILE_AVI_ODML)
      {
      quicktime_set_indx_keyframe(file, trak, frame);
      }
    }
  
  // Expand table
  if(stss->entries_allocated <= stss->total_entries)
    {
    stss->entries_allocated += 1024;
    stss->table = realloc(stss->table,
                          sizeof(*stss->table) *
                          stss->entries_allocated);
    }
  
  stss->table[stss->total_entries].sample = frame+1;
  stss->total_entries++;
  }


int quicktime_has_keyframes(quicktime_t *file, int track)
  {
  quicktime_trak_t *trak = file->vtracks[track].track;
  quicktime_stss_t *stss = &trak->mdia.minf.stbl.stss;
	
  return stss->total_entries > 0;
  }

int lqt_is_keyframe(quicktime_t *file, int track, int frame)
  {
  int i;
  quicktime_stss_t *stss = &file->vtracks[track].track->mdia.minf.stbl.stss;

  if(!stss->total_entries)
    return 1;

  frame++;
  
  for(i = 0; i < stss->total_entries; i++)
    {
    if(stss->table[i].sample == frame)
      return 1;
    }
  return 0;
  }


int quicktime_init_video_map(quicktime_video_map_t *vtrack,
                             int encode,
                             lqt_codec_info_t * info)
  {
  vtrack->current_position = 0;
  vtrack->cur_chunk = 0;
  vtrack->io_cmodel = BC_RGB888;
  quicktime_init_vcodec(vtrack, encode, info);
  return 0;
  }

int quicktime_delete_video_map(quicktime_video_map_t *vtrack)
  {
  quicktime_delete_codec(vtrack->codec);
  if(vtrack->temp_frame)
    lqt_rows_free(vtrack->temp_frame);
  if(vtrack->timecodes)
    free(vtrack->timecodes);
  if(vtrack->timestamps)
    free(vtrack->timestamps);
  if(vtrack->picture_numbers)
    free(vtrack->picture_numbers);
  
  lqt_compression_info_free(&vtrack->ci);
        
  return 0;
  }

int quicktime_init_audio_map(quicktime_t * file,
                             quicktime_audio_map_t *atrack,
                             int encode,
                             lqt_codec_info_t * info)
  {
  if(!encode)
    atrack->total_samples = quicktime_track_samples(file, atrack->track);
  atrack->channels = atrack->track->mdia.minf.stbl.stsd.table[0].channels;
  atrack->samplerate = (int)(atrack->track->mdia.minf.stbl.stsd.table[0].samplerate + 0.5);
  atrack->current_position = 0;
  atrack->cur_chunk = 0;

  if(!encode) 
    {
    /* Set channel setup */
    if(atrack->track->mdia.minf.stbl.stsd.table[0].has_chan)
      quicktime_get_chan(atrack);
    }
  quicktime_init_acodec(atrack, encode, info);
  return 0;
  }

int quicktime_delete_audio_map(quicktime_audio_map_t *atrack)
  {
  quicktime_delete_codec(atrack->codec);
  if(atrack->sample_buffer)
    free(atrack->sample_buffer);
  if(atrack->channel_setup)
    free(atrack->channel_setup);
  lqt_compression_info_free(&atrack->ci);
  return 0;
  }

// Initialize maps, for reading only



void quicktime_init_maps(quicktime_t * file)
  {
  int i, j, k, dom, track;

  /* Initialize trak for reading */

  for(i = 0; i < file->moov.total_tracks; i++)
    {
    quicktime_trak_fix_counts(file, file->moov.trak[i],
                              file->moov.mvhd.time_scale);
    }
  
  /* get tables for all the different tracks */
  file->total_atracks = quicktime_audio_tracks(file);

  if(file->total_atracks)
    {
    file->atracks = calloc(1, sizeof(*file->atracks) * file->total_atracks);
    for(i = 0, track = 0; i < file->total_atracks; i++, track++)
      {
      while(!file->moov.trak[track]->mdia.minf.is_audio)
        track++;
      file->atracks[i].track = file->moov.trak[track];
      quicktime_init_audio_map(file, &file->atracks[i],
                               file->wr,
                               (lqt_codec_info_t*)0);
      /* Some codecs set the channel setup */
      file->atracks[i].codec->decode_audio(file, (void*)0, 0, i);
      }
    }

  file->total_vtracks = quicktime_video_tracks(file);

  if(file->total_vtracks)
    {
    file->vtracks = calloc(1, sizeof(*file->vtracks) * file->total_vtracks);
    
    for(track = 0, i = 0; i < file->total_vtracks; i++, track++)
      {
      while(!file->moov.trak[track]->mdia.minf.is_video)
        track++;

      file->vtracks[i].track = file->moov.trak[track];
      quicktime_init_video_map(&file->vtracks[i],
                               file->wr,
                               (lqt_codec_info_t*)0);
      /* Get decoder colormodel */
      file->vtracks[i].codec->decode_video(file, (uint8_t**)0, i);
      file->vtracks[i].io_cmodel = file->vtracks[i].stream_cmodel;
      
      lqt_get_default_rowspan(file->vtracks[i].stream_cmodel,
                              quicktime_video_width(file, i),
                              &file->vtracks[i].stream_row_span,
                              &file->vtracks[i].stream_row_span_uv);

      /* Get interlace mode */
      if((file->vtracks[i].interlace_mode == LQT_INTERLACE_NONE) &&
         (file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].has_fiel))
        {
        dom = file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].fiel.dominance;
        if (file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].fiel.fields == 2)
          {
          if (dom == 14 || dom == 6)
            file->vtracks[i].interlace_mode = LQT_INTERLACE_BOTTOM_FIRST;
          else if (dom == 9 || dom == 1)
            file->vtracks[i].interlace_mode = LQT_INTERLACE_TOP_FIRST;
          }
        }
      /* Timecode track */
      if(file->moov.trak[track]->has_tref)
        {
        for(j = 0; j < file->moov.trak[track]->tref.num_references; j++)
          {
          /* Track reference has type tmcd */
          if(quicktime_match_32(file->moov.trak[track]->tref.references[j].type, "tmcd"))
            {
            for(k = 0; k < file->moov.total_tracks; k++)
              {
              if(file->moov.trak[track]->tref.references[j].tracks[0] ==
                 file->moov.trak[k]->tkhd.track_id)
                {
                file->vtracks[i].timecode_track = file->moov.trak[k];
                break;
                }
              }
            break;
            }
          }
        }
      
      }
    }

  /* Text tracks */

  file->total_ttracks = lqt_text_tracks(file);

  if(file->total_ttracks)
    {
    file->ttracks = calloc(file->total_ttracks, sizeof(*file->ttracks));

    for(track = 0, i = 0; i < file->total_ttracks; i++, track++)
      {
      while(!file->moov.trak[track]->mdia.minf.is_text)
        track++;
      lqt_init_text_map(file,
                        &file->ttracks[i], file->moov.trak[track], 0);
      }
    }
  
  

  }

int quicktime_read_info(quicktime_t *file)
  {
  int result = 0, got_header = 0;
  int64_t start_position = quicktime_position(file);
  quicktime_atom_t leaf_atom;
  uint8_t avi_avi[4];
  int got_avi = 0;

  quicktime_set_position(file, 0LL);

  /* Test file format */
  do
    {
    file->file_type = LQT_FILE_AVI;
    result = quicktime_atom_read_header(file, &leaf_atom);
    if(!result && quicktime_atom_is(&leaf_atom, "RIFF"))
      {
      quicktime_read_data(file, avi_avi, 4);
      if(quicktime_match_32(avi_avi, "AVI "))
        {
        got_avi = 1;
        }
      else
        {
        result = 0;
        break;
        }
      }
    else
      {
      result = 0;
      break;
      }
    }while(1);
  if(!got_avi) file->file_type = LQT_FILE_NONE;
  quicktime_set_position(file, 0LL);

  /* McRoweSoft AVI section */
  if(file->file_type == LQT_FILE_AVI)
    {
    /* Import first RIFF */
    do
      {
      result = quicktime_atom_read_header(file, &leaf_atom);
      if(!result)
        {
        if(quicktime_atom_is(&leaf_atom, "RIFF"))
          {
          quicktime_read_riff(file, &leaf_atom);
          /* Return success */
          got_header = 1;
          }
        }
      }while(!result &&
             !got_header &&
             quicktime_position(file) < file->total_length);

    /* Construct indexes. */
    if(quicktime_import_avi(file))
      return 1;
    }
  /* Quicktime section */
  else
    if(!(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML)))
      {
      do
        {
        result = quicktime_atom_read_header(file, &leaf_atom);

        if(!result)
          {
          if(quicktime_atom_is(&leaf_atom, "mdat"))
            {
            quicktime_read_mdat(file, &file->mdat, &leaf_atom);
            }
          else
            if(quicktime_atom_is(&leaf_atom, "ftyp"))
              {
              quicktime_read_ftyp(file, &file->ftyp, &leaf_atom);
              file->file_type = quicktime_ftyp_get_file_type(&file->ftyp);
              file->has_ftyp = 1;
              }
            else
              if(quicktime_atom_is(&leaf_atom, "moov"))
                {
                /* Set preload and preload the moov atom here */
                int64_t start_position = quicktime_position(file);
                long temp_size = leaf_atom.end - start_position;
                unsigned char *temp = malloc(temp_size);
                quicktime_set_preload(file,
                                      (temp_size < 0x100000) ? 0x100000 : temp_size);
                quicktime_read_data(file, temp, temp_size);
                quicktime_set_position(file, start_position);
                free(temp);

                quicktime_read_moov(file, &file->moov, &leaf_atom);
                got_header = 1;
                }
              else
                quicktime_atom_skip(file, &leaf_atom);
          }
        }while(!result && quicktime_position(file) < file->total_length);
		
      /* read QTVR sample atoms -- object */
      if (lqt_qtvr_get_object_track(file) >= 0)
        {
        quicktime_qtatom_t leaf_atom, root_atom;
        int64_t start_position = quicktime_position(file);
        quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_object_track(file)]->mdia.minf.stbl.stco.table[0].offset);
        quicktime_qtatom_read_container_header(file);
        /* root qtatom "sean" */
        quicktime_qtatom_read_header(file, &root_atom);
			
        do
          {
          quicktime_qtatom_read_header(file, &leaf_atom);
          if(quicktime_qtatom_is(&leaf_atom, "obji"))
            {
            quicktime_read_obji(file, &file->qtvr_node[0].obji);
            }     
          else
            if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
              {
              quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);
              }     
            else
              quicktime_qtatom_skip(file, &leaf_atom);
          } while(quicktime_position(file) < root_atom.end);
			
        quicktime_set_position(file, start_position);
        }
		
      /* read QTVR sample atoms  -- panorama */
      if (lqt_qtvr_get_panorama_track(file) >= 0 && lqt_qtvr_get_qtvr_track(file) >= 0)
        {
        quicktime_qtatom_t leaf_atom, root_atom;
        int64_t start_position = quicktime_position(file);
        quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stco.table[0].offset);
        quicktime_qtatom_read_container_header(file);
        /* root qtatom "sean" */
        quicktime_qtatom_read_header(file, &root_atom);
			
        do
          {
          quicktime_qtatom_read_header(file, &leaf_atom);
          if(quicktime_qtatom_is(&leaf_atom, "pdat"))
            {
            quicktime_read_pdat(file, &file->qtvr_node[0].pdat);
            }     
          else
            if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
              {
              quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);
              }     
            else
              quicktime_qtatom_skip(file, &leaf_atom);
          } while(quicktime_position(file) < root_atom.end);
			
        quicktime_set_position(file, start_position);
        }
		
      if (lqt_qtvr_get_qtvr_track(file) >= 0)
        {
        quicktime_qtatom_t leaf_atom, root_atom;
        int64_t start_position = quicktime_position(file);
        quicktime_set_position(file, file->moov.trak[lqt_qtvr_get_qtvr_track(file)]->mdia.minf.stbl.stco.table[0].offset);
        quicktime_qtatom_read_container_header(file);
        /* root qtatom "sean" */
        quicktime_qtatom_read_header(file, &root_atom);
			
        do
          {
          quicktime_qtatom_read_header(file, &leaf_atom);
          if(quicktime_qtatom_is(&leaf_atom, "ndhd"))
            {
            quicktime_read_ndhd(file, &file->qtvr_node[0].ndhd);
            }     
          else
            quicktime_qtatom_skip(file, &leaf_atom);
          } while(quicktime_position(file) < root_atom.end);
			
        quicktime_set_position(file, start_position);
        }
      /* go back to the original position */
      quicktime_set_position(file, start_position);
      }

  /* Set file type if no ftyp is there (must be done before initializing the codecs) */
  if(file->file_type == LQT_FILE_NONE)
    file->file_type = LQT_FILE_QT_OLD;

  
  /* Initialize track map objects */
  if(got_header)
    {
    quicktime_init_maps(file);
    }

        
  /* Shut down preload in case of an obsurdly high temp_size */
  quicktime_set_preload(file, 0);
  return !got_header;
  }

int quicktime_dump(quicktime_t *file)
  {
  lqt_dump("quicktime_dump\n");
  if(file->has_ftyp)
    quicktime_ftyp_dump(&file->ftyp);
	
  lqt_dump("movie data (mdat)\n");
  lqt_dump(" size %"PRId64"\n", file->mdat.atom.size);
  lqt_dump(" start %"PRId64"\n", file->mdat.atom.start);
  quicktime_moov_dump(&file->moov);
  if (lqt_qtvr_get_object_track(file) >= 0)
    {
    quicktime_obji_dump(&file->qtvr_node[0].obji);
    }
  if (lqt_qtvr_get_panorama_track(file) >= 0)
    {
    quicktime_pdat_dump(&file->qtvr_node[0].pdat);
    }
  if (lqt_qtvr_get_qtvr_track(file) >= 0)
    {
    quicktime_ndhd_dump(&file->qtvr_node[0].ndhd);
    }
  if(file->file_type & (LQT_FILE_AVI | LQT_FILE_AVI_ODML))
    {
    quicktime_riff_dump(file->riff[0]);
    }
  return 0;
  }


// ================================== Entry points =============================

int quicktime_check_sig(char *path)
  {
  quicktime_t file;
  quicktime_atom_t leaf_atom;
  int result = 0, result1 = 0, result2 = 0;
  uint8_t avi_test[12];
  quicktime_init(&file);
  result = quicktime_file_open(&file, path, 1, 0);
  if(!result)
    {
    // Check for Microsoft AVI
    quicktime_read_data(&file, avi_test, 12);
    quicktime_set_position(&file, 0);
    if(quicktime_match_32(avi_test, "RIFF") &&
       quicktime_match_32(avi_test + 8, "AVI "))
      {
      result2 = 1;
      }
    else
      {
      do
        {
        result1 = quicktime_atom_read_header(&file, &leaf_atom);
        if(!result1)
          {
          /* just want the "moov" atom */
          if(quicktime_atom_is(&leaf_atom, "moov"))
            {
            result2 = 1;
            }
          else
            quicktime_atom_skip(&file, &leaf_atom);
          }
        }while(!result1 && !result2 && quicktime_position(&file) < file.total_length);
      }
    }
  quicktime_file_close(&file);
  quicktime_delete(&file);
  return result2;
  }

void quicktime_set_avi(quicktime_t *file, int value)
  {
  if(value)
    file->file_type = LQT_FILE_AVI_ODML;
  }
#if 0

static int quicktime_is_avi(quicktime_t *file)
  {
  return !!(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML));
  }
#endif

static quicktime_t* do_open(const char *filename, int rd, int wr, lqt_file_type_t type,
                            lqt_log_callback_t log_cb, void * log_data)
  {
  int i;
  quicktime_t *new_file;
  int result = 0;

  new_file = calloc(1, sizeof(*new_file));

  new_file->log_callback = log_cb;
  new_file->log_data = log_data;
        
  if(rd && wr)
    {
    lqt_log(new_file, LQT_LOG_ERROR, LOG_DOMAIN, "read/write mode is not supported");
    free(new_file);
    return (quicktime_t*)0;
    }

  quicktime_init(new_file);
  new_file->wr = wr;
  new_file->rd = rd;
  new_file->mdat.atom.start = 0;
  if(wr)
    {
    new_file->file_type = type;
    quicktime_ftyp_init(&new_file->ftyp, type);
    if(new_file->ftyp.major_brand)
      new_file->has_ftyp = 1;

    /* Switch the iods atom on for MP4 */
    if(new_file->file_type & LQT_FILE_MP4)
      new_file->moov.has_iods = 1;
    }
  result = quicktime_file_open(new_file, filename, rd, wr);

  if(!result)
    {
    if(rd)
      {
      if(quicktime_read_info(new_file))
        {
        lqt_log(new_file, LQT_LOG_ERROR, LOG_DOMAIN, "Opening failed (unsupported filetype)");
        quicktime_close(new_file);
        new_file = 0;
        }
      }
          
    /* start the data atom */
    /* also don't want to do this if making a streamable file */
    if(wr)
      {
      if(new_file->has_ftyp)
        quicktime_write_ftyp(new_file, &new_file->ftyp);
      quicktime_atom_write_header64(new_file, 
                                    &new_file->mdat.atom, 
                                    "mdat");
      }
    }
  else
    {
    /* If the open failed due to permission or path errors then there
     * may not be a file pointer allocated and attempting to close will
     * coredump due to a NULL pointer dereference.
     */
    if (new_file->stream)
      quicktime_close(new_file);
    else
      free(new_file);
    new_file = 0;
    }
        
        
        
  if(rd && new_file)
    {
    /* Set default decoding parameters */
    for(i = 0; i < new_file->total_atracks; i++)
      lqt_set_default_audio_parameters(new_file, i);

    for(i = 0; i < new_file->total_vtracks; i++)
      lqt_set_default_video_parameters(new_file, i);
    }
        
  return new_file;
  }

quicktime_t* quicktime_open(const char *filename, int rd, int wr)
  {
  return do_open(filename, rd, wr, LQT_FILE_QT_OLD, NULL, NULL);
  }

quicktime_t * lqt_open_read(const char * filename)
  {
  return do_open(filename, 1, 0, LQT_FILE_NONE, NULL, NULL);
  }

quicktime_t * lqt_open_write(const char * filename, lqt_file_type_t type)
  {
  return do_open(filename, 0, 1, type, NULL, NULL);
  }

quicktime_t * lqt_open_read_with_log(const char * filename,
                                     lqt_log_callback_t cb, void * log_data)
  {
  return do_open(filename, 1, 0, LQT_FILE_NONE, cb, log_data);
  }

quicktime_t * lqt_open_write_with_log(const char * filename, lqt_file_type_t type,
                                      lqt_log_callback_t cb, void * log_data)
  {
  return do_open(filename, 0, 1, type, cb, log_data);
  }


int quicktime_close(quicktime_t *file)
  {
  int i;
  int result = 0;
  if(file->wr)
    {
    /* Finish final chunk if necessary */
    if(file->write_trak)
      quicktime_write_chunk_footer(file, file->write_trak);
    
    quicktime_codecs_flush(file);

    for(i = 0; i < file->total_vtracks; i++)
      {
      lqt_video_build_timestamp_tables(file, i);

      /* Fix stts for timecode track */
      if(file->vtracks[i].timecode_track &&
         file->vtracks[i].timecodes_written)
        {
        int64_t duration;
        quicktime_trak_duration(file->vtracks[i].track,
                                &duration, (int*)0);
        lqt_flush_timecode(file, i, duration, 1);
        }
      }
    if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
      {
#if 0
      quicktime_atom_t junk_atom;
      int64_t position = quicktime_position(file);
#endif
      // total_riffs is 0 if no A/V frames have been written
      if(file->total_riffs)
        {
        // Finalize last header
        quicktime_finalize_riff(file, file->riff[file->total_riffs - 1]);
        // Finalize the odml header
        quicktime_finalize_odml(file, &file->riff[0]->hdrl);
        
        if(file->file_type == LQT_FILE_AVI_ODML)
          {
          for(i = 0; i < file->moov.total_tracks; i++)
            {
            quicktime_finalize_indx(file, &file->moov.trak[i]->strl->indx);
            }
          }
        }
      }
    else
      {
      if (lqt_qtvr_get_object_track(file) >= 0)
        {
        lqt_qtvr_add_object_node(file);
        }
      else if (lqt_qtvr_get_panorama_track(file) >= 0)
        {
        lqt_qtvr_add_panorama_node(file);
        }
      // Atoms are only written here
      quicktime_atom_write_footer(file, &file->mdat.atom);
      quicktime_finalize_moov(file, &file->moov);
      quicktime_write_moov(file, &file->moov);
      }
    }
  quicktime_file_close(file);
  quicktime_delete(file);
  free(file);
  return result;
  }



/*
 *  Apply default parameters for a codec
 */

static void apply_default_parameters(quicktime_t * file,
                                     int track,
                                     quicktime_codec_t * codec,
                                     int encode)
  {
  int num_parameters;
  lqt_parameter_info_t * parameter_info;
  int j;
  
  lqt_codec_info_t * codec_info = codec->info;

  if(!codec_info)
    return;
  
  if(encode)
    {
    num_parameters = codec_info->num_encoding_parameters;
    parameter_info = codec_info->encoding_parameters;
    }
  else
    {
    num_parameters = codec_info->num_decoding_parameters;
    parameter_info = codec_info->decoding_parameters;
    }
  
  for(j = 0; j < num_parameters; j++)
    {
    switch(parameter_info[j].type)
      {
      case LQT_PARAMETER_INT:
        lqt_log(file, LQT_LOG_DEBUG, LOG_DOMAIN, "Setting parameter %s to %d",
                parameter_info[j].name,
                parameter_info[j].val_default.val_int);
//        codec->set_parameter(file, track, parameter_info[j].name,
//                             &parameter_info[j].val_default.val_int);
        break;
      case LQT_PARAMETER_FLOAT:
        lqt_log(file, LQT_LOG_DEBUG, LOG_DOMAIN, "Setting parameter %s to %f",
                parameter_info[j].name,
                parameter_info[j].val_default.val_float);
//        codec->set_parameter(file, track, parameter_info[j].name,
//                             &parameter_info[j].val_default.val_float);
        break;
      case LQT_PARAMETER_STRING:
      case LQT_PARAMETER_STRINGLIST:
        lqt_log(file, LQT_LOG_DEBUG, LOG_DOMAIN, "Setting parameter %s to %s",
                parameter_info[j].name,
                parameter_info[j].val_default.val_string);
//        codec->set_parameter(file, track, parameter_info[j].name,
//                             parameter_info[j].val_default.val_string);
        break;
      case LQT_PARAMETER_SECTION:
        break; /* NOP */
      }

    }                      
  }

void lqt_set_default_video_parameters(quicktime_t * file, int track)
  {
  int i;

  for(i = 0; i < file->total_vtracks; i++)
    {
    apply_default_parameters(file, track, file->vtracks[track].codec,
                             file->wr);
    }
  }

	

void lqt_set_default_audio_parameters(quicktime_t * file, int track)
  {
  int i;
  for(i = 0; i < file->total_atracks; i++)
    {
    apply_default_parameters(file, i, file->atracks[track].codec,
                             file->wr);
    }
  }

int quicktime_major()
  {
  return QUICKTIME_MAJOR;
  }

int quicktime_minor()
  {
  return QUICKTIME_MINOR;
  }

int quicktime_release()
  {
  return QUICKTIME_RELEASE;
  }

int quicktime_div3_is_key(unsigned char *data, long size)
  {
  int result = 0;

// First 2 bits are pict type.
  result = (data[0] & 0xc0) == 0;


  return result;
  }


/*
 *  AVI Specific stuff
 */

int lqt_is_avi(quicktime_t *file)
  {
  return (file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML));
  }

int lqt_get_wav_id(quicktime_t *file, int track)
  {
  quicktime_trak_t * trak;
  trak = file->atracks[track].track;
  return trak->mdia.minf.stbl.stsd.table[0].compression_id;
  }

int64_t * lqt_get_chunk_sizes(quicktime_t * file, quicktime_trak_t *trak)
  {
  int i, j;
  int64_t * ret;
  int64_t next_offset;
  long num_chunks;
  int num_tracks;
  int * chunk_indices;
  
  num_chunks = trak->mdia.minf.stbl.stco.total_entries;
  ret = calloc(num_chunks, sizeof(int64_t));

  num_tracks = file->moov.total_tracks;

  chunk_indices = malloc(num_tracks * sizeof(int));

  for(i = 0; i < num_tracks; i++)
    {
    chunk_indices[i] = 0;
    }
  
  for(i = 0; i < num_chunks; i++)
    {
    next_offset = -1;
    for(j = 0; j < num_tracks; j++)
      {
      if(chunk_indices[j] < 0)
        continue;

      while(file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset <=
            trak->mdia.minf.stbl.stco.table[i].offset)
        {
        if(chunk_indices[j] >=
           file->moov.trak[j]->mdia.minf.stbl.stco.total_entries - 1)
          {
          chunk_indices[j] = -1;
          break;
          }
        else
          chunk_indices[j]++;
        }
      if(chunk_indices[j] < 0)
        continue;
      if((next_offset == -1) ||
         (file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset <
          next_offset))
        next_offset = file->moov.trak[j]->mdia.minf.stbl.stco.table[chunk_indices[j]].offset;
      }
    if(next_offset > 0)
      {
      ret[i] = next_offset - trak->mdia.minf.stbl.stco.table[i].offset;
      }
    else /* Last chunk: Take the end of the mdat atom */
      {
      ret[i] = file->mdat.atom.start + file->mdat.atom.size -
        trak->mdia.minf.stbl.stco.table[i].offset;
      if(ret[i] < 0)
        ret[i] = 0;
      }
    }
  free(chunk_indices);
  return ret;
  }

int lqt_read_audio_chunk(quicktime_t * file, int track,
                         long chunk,
                         uint8_t ** buffer, int * buffer_alloc, int * samples)
  {
  int64_t offset;
  quicktime_trak_t * trak;
  int result;

  trak = file->atracks[track].track;

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    {
    /* Read beyond EOF */
    file->atracks[track].eof = 1;
    return 0;
    }
  if(!trak->chunk_sizes)
    {
    trak->chunk_sizes = lqt_get_chunk_sizes(file, trak);
    }
  if(samples)
    *samples = quicktime_chunk_samples(trak, chunk);
  /* Reallocate buffer */

  if(*buffer_alloc < trak->chunk_sizes[chunk] + 16)
    {
    *buffer_alloc = trak->chunk_sizes[chunk] + 32;
    *buffer = realloc(*buffer, *buffer_alloc);
    }
  
  /* Get offset */
  
  offset = quicktime_chunk_to_offset(file, trak, chunk);

  quicktime_set_position(file, offset);

  result = quicktime_read_data(file, *buffer, trak->chunk_sizes[chunk]);

  memset((*buffer) + trak->chunk_sizes[chunk], 0, 16);
  
  return result ? trak->chunk_sizes[chunk] : 0;
  }

int lqt_append_audio_chunk(quicktime_t * file, int track,
                           long chunk,
                           uint8_t ** buffer, int * buffer_alloc,
                           int initial_bytes)
  {
  int64_t offset;
  quicktime_trak_t * trak;
  int result;

  trak = file->atracks[track].track;

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    {
    /* Read beyond EOF */
    file->atracks[track].eof = 1;
    return 0;
    }

  if(!trak->chunk_sizes)
    {
    trak->chunk_sizes = lqt_get_chunk_sizes(file, trak);
    }

  /* Reallocate buffer */

  if(*buffer_alloc < trak->chunk_sizes[chunk] + 16 + initial_bytes)
    {
    *buffer_alloc = trak->chunk_sizes[chunk] + 32 + initial_bytes;
    *buffer = realloc(*buffer, *buffer_alloc);
    }
  
  /* Get offset */
  
  offset = quicktime_chunk_to_offset(file, trak, chunk);

  quicktime_set_position(file, offset);

  result = quicktime_read_data(file, (*buffer) + initial_bytes, trak->chunk_sizes[chunk]);

  memset((*buffer) + initial_bytes + trak->chunk_sizes[chunk], 0, 16);
  
  return result ? trak->chunk_sizes[chunk] : 0;
  }


int64_t lqt_last_audio_position(quicktime_t * file, int track)
  {
  return file->atracks[track].last_position;
  }

/* Interlace mode */

static struct
  {
  lqt_interlace_mode_t mode;
  const char * name;
  }
interlace_modes[] =
  {
    { LQT_INTERLACE_NONE,         "None (Progressive)" },
    { LQT_INTERLACE_TOP_FIRST,    "Top field first"    },
    { LQT_INTERLACE_BOTTOM_FIRST, "Bottom field first" }
  };
  
lqt_interlace_mode_t lqt_get_interlace_mode(quicktime_t * file, int track)
  {
  if(track < 0 || track > file->total_vtracks)
    return LQT_INTERLACE_NONE;
  return file->vtracks[track].interlace_mode;
  }

int lqt_set_interlace_mode(quicktime_t * file, int track,
                           lqt_interlace_mode_t mode)
  {
  if(track < 0 || track > file->total_vtracks)
    return 0;
  file->vtracks[track].interlace_mode = mode;
  return 1;
  }

const char * lqt_interlace_mode_to_string(lqt_interlace_mode_t mode)
  {
  int i;
  for(i = 0; i < sizeof(interlace_modes)/sizeof(interlace_modes[0]); i++)
    {
    if(interlace_modes[i].mode == mode)
      return interlace_modes[i].name;
    }
  return interlace_modes[0].name;
  }

/* Chroma placement */

static struct
  {
  lqt_chroma_placement_t placement;
  const char * name;
  }
chroma_placements[] =
  {
    { LQT_CHROMA_PLACEMENT_DEFAULT,  "MPEG-1/JPEG" },
    { LQT_CHROMA_PLACEMENT_MPEG2,    "MPEG-2" },
    { LQT_CHROMA_PLACEMENT_DVPAL,    "PAL DV" }
  };

const char * lqt_chroma_placement_to_string(lqt_chroma_placement_t placement)
  {
  int i;
  for(i = 0; i < sizeof(chroma_placements)/sizeof(chroma_placements[0]); i++)
    {
    if(chroma_placements[i].placement == placement)
      return chroma_placements[i].name;
    }
  return chroma_placements[0].name;
  }

lqt_chroma_placement_t lqt_get_chroma_placement(quicktime_t * file, int track)
  {
  if(track < 0 || track > file->total_vtracks)
    return LQT_INTERLACE_NONE;
  return file->vtracks[track].chroma_placement;
  }

/* Sample format */

static struct
  {
  lqt_sample_format_t format;
  const char * name;
  }
sample_formats[] =
  {
    { LQT_SAMPLE_UNDEFINED, "Undefined" }, /* If this is returned, we have an error */
    { LQT_SAMPLE_INT8, "8 bit signed" },
    { LQT_SAMPLE_UINT8, "8 bit unsigned" },
    { LQT_SAMPLE_INT16, "16 bit signed" },
    { LQT_SAMPLE_INT32, "32 bit signed" },
    { LQT_SAMPLE_FLOAT, "Floating point" }, /* Float is ALWAYS machine native */
    { LQT_SAMPLE_DOUBLE, "Double precision" } /* Double is ALWAYS machine native */
  };

const char * lqt_sample_format_to_string(lqt_sample_format_t format)
  {
  int i;
  for(i = 0; i < sizeof(sample_formats)/sizeof(sample_formats[0]); i++)
    {
    if(sample_formats[i].format == format)
      return sample_formats[i].name;
    }
  return sample_formats[0].name;
  }

lqt_sample_format_t lqt_get_sample_format(quicktime_t * file, int track)
  {
  quicktime_audio_map_t * atrack;

  if(track < 0 || track > file->total_atracks)
    return LQT_SAMPLE_UNDEFINED;

  atrack = &file->atracks[track];

  if(atrack->sample_format == LQT_SAMPLE_UNDEFINED)
    {
    if(file->wr)
      {
      atrack->codec->encode_audio(file, (void*)0, 0, track);
      
      }
    else
      {
      atrack->codec->decode_audio(file, (void*)0, 0, track);

      }
    }
  
  return atrack->sample_format;
  }

void lqt_init_vbr_audio(quicktime_t * file, int track)
  {
  quicktime_trak_t * trak = file->atracks[track].track;
  trak->mdia.minf.stbl.stsd.table[0].compression_id = -2;
  trak->mdia.minf.stbl.stsz.sample_size = 0;
  trak->mdia.minf.is_audio_vbr = 1;

  if(trak->strl)
    {
    trak->strl->strh.dwRate = quicktime_sample_rate(file, track);
    trak->strl->strh.dwScale = 0;
    trak->strl->strh.dwSampleSize = 0;
    
    trak->strl->strf.wf.f.WAVEFORMAT.nBlockAlign = 0;
    trak->strl->strf.wf.f.WAVEFORMAT.nAvgBytesPerSec =  18120; // Probably doesn't matter
    trak->strl->strf.wf.f.PCMWAVEFORMAT.wBitsPerSample = 0;
    }
  }

LQT_EXTERN void lqt_set_audio_bitrate(quicktime_t * file, int track, int bitrate)
  {
  quicktime_trak_t * trak = file->atracks[track].track;

  if(!trak->strl)
    return;
  
  /* strh stuff */
  trak->strl->strh.dwRate = bitrate / 8;
  trak->strl->strh.dwScale = 1;
  trak->strl->strh.dwSampleSize = 1;
  /* WAVEFORMATEX stuff */
  
  trak->strl->strf.wf.f.WAVEFORMAT.nBlockAlign = 1;
  trak->strl->strf.wf.f.WAVEFORMAT.nAvgBytesPerSec =  bitrate / 8;
  trak->strl->strf.wf.f.PCMWAVEFORMAT.wBitsPerSample = 0;
  }


void lqt_start_audio_vbr_frame(quicktime_t * file, int track)
  {
  quicktime_audio_map_t * atrack = &file->atracks[track];
  
  /* Make chunk at maximum 10 VBR packets large */
  if((file->write_trak == atrack->track) &&
     (atrack->track->chunk_samples >= 10))
    {
    quicktime_write_chunk_footer(file, file->write_trak);
    quicktime_write_chunk_header(file, atrack->track);
    }
  atrack->vbr_frame_start = quicktime_position(file);
  
  }

void lqt_finish_audio_vbr_frame(quicktime_t * file, int track, int num_samples)
  {
  int size;
  quicktime_stsz_t * stsz;
  quicktime_stts_t * stts;
  long vbr_frames_written;
  quicktime_audio_map_t * atrack = &file->atracks[track];
  
  stsz = &atrack->track->mdia.minf.stbl.stsz;
  stts = &atrack->track->mdia.minf.stbl.stts;

  vbr_frames_written = stsz->total_entries;
  
  /* Update stsz */

  size = quicktime_position(file) - atrack->vbr_frame_start;
  
  quicktime_update_stsz(stsz, vbr_frames_written, size);
  
  if(atrack->track->strl)
    {
    quicktime_strl_t * strl = atrack->track->strl;
    
    if(size > strl->strf.wf.f.WAVEFORMAT.nBlockAlign)
      strl->strf.wf.f.WAVEFORMAT.nBlockAlign = size;

    if(!strl->strh.dwScale)
      strl->strh.dwScale = num_samples;

    strl->strh.dwLength++;
    }
  
  /* Update stts */
  
  quicktime_update_stts(stts, vbr_frames_written, num_samples);
  
  atrack->track->chunk_samples++;
  }

/* VBR Reading support */

/* Check if VBR reading should be enabled */

int lqt_audio_is_vbr(quicktime_t * file, int track)
  {
  return file->atracks[track].track->mdia.minf.is_audio_vbr;
  }



/* Get the index of the VBR packet (== sample) containing a specified
   uncompressed sample */

static uint64_t packet_of_sample(quicktime_stts_t * stts, int64_t sample)
  {
  uint32_t i;
  int64_t packet_count = 0; 
  int64_t sample_count = 0; 
  
  for(i = 0; i < stts->total_entries; i++)
    {
    if(sample_count + stts->table[i].sample_count * stts->table[i].sample_duration > sample)
      {
      return packet_count + (sample - sample_count) / stts->table[i].sample_duration;
      }
    sample_count += stts->table[i].sample_count * stts->table[i].sample_duration;
    packet_count += stts->table[i].sample_count;
    }
  return -1;
  }

/*
 *  Helper function: Get the "durarion of a sample range" (which means the
 *  uncompressed samples in a range of VBR packets)
 */

static int64_t get_uncompressed_samples(quicktime_stts_t * stts, long start_sample,
                                    long end_sample)
  {
  long count, i, stts_index = 0, stts_count = 0;

  int64_t ret;
  
  count = 0;
  ret = 0;
  
  for(i = 0; i < stts->total_entries; i++)
    {
    if(count + stts->table[i].sample_count > start_sample)
      {
      stts_index = i;
      stts_count = start_sample - count;
      break;
      }
    count += stts->table[i].sample_count;
    }

  ret = 0;
  for(i = start_sample; i < end_sample; i++)
    {
    ret += stts->table[stts_index].sample_duration;
    stts_count++;
    if(stts_count >= stts->table[stts_index].sample_count)
      {
      stts_index++;
      stts_count = 0;
      }
    }
  return ret;
  }

/* Analog for quicktime_chunk_samples for VBR files */

int lqt_chunk_of_sample_vbr(int64_t *chunk_sample, 
                            int64_t *chunk, 
                            quicktime_trak_t *trak, 
                            int64_t sample)
  {
  int64_t packet;
  int64_t chunk_packet;
  
  /* Get the index of the packet containing the uncompressed sample */
  packet = packet_of_sample(&trak->mdia.minf.stbl.stts, sample);

  /* Get the chunk of the packet */
  
  quicktime_chunk_of_sample(&chunk_packet, 
                            chunk, 
                            trak, 
                            packet);

  /* Get the first uncompressed sample of the first packet of
     this chunk */
  
  *chunk_sample =
    get_uncompressed_samples(&trak->mdia.minf.stbl.stts, 0,
                             chunk_packet);
  return 0;
  }


/* Determine the number of VBR packets (=samples) in one chunk */

int lqt_audio_num_vbr_packets(quicktime_t * file,
                              int track, long chunk, int * samples)
  {
  int64_t start_sample;
  
  quicktime_trak_t * trak;
  long result = 0;
  quicktime_stsc_t *stsc;
  long i;

  
  trak = file->atracks[track].track;
    
  stsc = &trak->mdia.minf.stbl.stsc;

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    return 0;
  
  i = stsc->total_entries - 1;
  
  if(!stsc->total_entries)
    return 0;
  
  start_sample = 0;

  for(i = 0; i < stsc->total_entries; i++)
    {
    if(((i < stsc->total_entries - 1) && (stsc->table[i+1].chunk > chunk+1)) ||
       (i == stsc->total_entries - 1))
      {
      start_sample +=
        (chunk - stsc->table[i].chunk) * stsc->table[i].samples;
      result = stsc->table[i].samples;
      break;
      }
    else
      start_sample +=
        (stsc->table[i+1].chunk - stsc->table[i].chunk) * stsc->table[i].samples;
    }
  if(samples)
    *samples = get_uncompressed_samples(&trak->mdia.minf.stbl.stts,
                                        start_sample, start_sample + result);
  
  return result;
  }

/* Read one VBR packet */
int lqt_audio_read_vbr_packet(quicktime_t * file, int track, long chunk, int packet,
                              uint8_t ** buffer, int * buffer_alloc, int * samples)
  {
  int64_t offset;
  long i, stsc_index;
  quicktime_trak_t * trak;
  quicktime_stsc_t *stsc;
  int packet_size;
  long first_chunk_packet; /* Index of first packet in the chunk */
  
  trak = file->atracks[track].track;
  stsc = &trak->mdia.minf.stbl.stsc;

  if(chunk >= trak->mdia.minf.stbl.stco.total_entries)
    return 0;
    
  i = 0;
  stsc_index = 0;
  first_chunk_packet = 0;

  for(i = 0; i < chunk; i++)
    {
    if((stsc_index < stsc->total_entries-1) && (stsc->table[stsc_index+1].chunk-1 == i))
      stsc_index++;
    first_chunk_packet += stsc->table[stsc_index].samples;
    }

  /* Get offset */
  offset = trak->mdia.minf.stbl.stco.table[chunk].offset;
  for(i = 0; i < packet; i++)
    {
    if(trak->mdia.minf.stbl.stsz.table)
      offset += trak->mdia.minf.stbl.stsz.table[first_chunk_packet+i].size;
    else
      offset += trak->mdia.minf.stbl.stsz.sample_size;
    }
  /* Get packet size */
  if(trak->mdia.minf.stbl.stsz.table)
    packet_size = trak->mdia.minf.stbl.stsz.table[first_chunk_packet+packet].size;
  else
    packet_size = trak->mdia.minf.stbl.stsz.sample_size;
  
  /* Get number of audio samples */
  if(samples)
    *samples = get_uncompressed_samples(&trak->mdia.minf.stbl.stts,
                                        first_chunk_packet+packet,
                                        first_chunk_packet+packet+1);
  
  /* Read the data */
  if(*buffer_alloc < packet_size+16)
    {
    *buffer_alloc = packet_size + 128;
    *buffer = realloc(*buffer, *buffer_alloc);
    }
  quicktime_set_position(file, offset);
  quicktime_read_data(file, *buffer, packet_size);
  return packet_size;
  }

static struct
  {
  lqt_file_type_t type;
  char * name;
  }
filetypes[] =
  {
      { LQT_FILE_NONE,     "Unknown/Undefined" },
      { LQT_FILE_QT_OLD,   "Quicktime"         },
      { LQT_FILE_QT,       "Quicktime"         },
      { LQT_FILE_AVI,      "AVI"               },
      { LQT_FILE_AVI_ODML, "AVI ODML"          },
      { LQT_FILE_MP4,      "MP4"               },
      { LQT_FILE_M4A,      "M4A"               },
      { LQT_FILE_3GP,      "3GP"               }
  };
  
const char * lqt_file_type_to_string(lqt_file_type_t type)
  {
  int i;
  for(i = 0; i < sizeof(filetypes)/sizeof(filetypes[0]); i++)
    {
    if(filetypes[i].type == type)
      return filetypes[i].name;
    }
  return filetypes[0].name;
  }

lqt_file_type_t lqt_get_file_type(quicktime_t * file)
  {
  return file->file_type;
  }

void lqt_set_max_riff_size(quicktime_t * file, int size)
  {
  file->max_riff_size = size * 1024 * 1024;
  }

/* PTS offsets */

/** \ingroup audio_encode
 *  \brief Set an audio pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS of the first audio sample (in samples)
 */

void lqt_set_audio_pts_offset(quicktime_t * file, int track, int64_t offset)
  {
  quicktime_trak_t * trak;

  if((track < 0) && (track >= file->total_atracks))
    return;

  trak = file->atracks[track].track;
  trak->pts_offset = offset;
  }
  
/** \ingroup audio_decode
 *  \brief Get an audio pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS of the first audio sample (in samples)
 */

int64_t lqt_get_audio_pts_offset(quicktime_t * file, int track)
  {
  quicktime_trak_t * trak;
  if((track < 0) && (track >= file->total_atracks))
    return 0;

  trak = file->atracks[track].track;
  return trak->pts_offset;
  }


/** \ingroup video_encode
 *  \brief Set an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS of the first video frame (in timescale units)
 */

void lqt_set_video_pts_offset(quicktime_t * file, int track, int64_t offset)
  {
  quicktime_trak_t * trak;
  if((track < 0) && (track >= file->total_vtracks))
    return;
  trak = file->vtracks[track].track;
  trak->pts_offset = offset;
  }

  
/** \ingroup video_decode
 *  \brief Get an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS of the first video frame (in timescale units)
 */

int64_t lqt_get_video_pts_offset(quicktime_t * file, int track)
  {
  quicktime_trak_t * trak;
  if((track < 0) && (track >= file->total_vtracks))
    return 0;
  trak = file->vtracks[track].track;
  return trak->pts_offset;

  }


/** \ingroup text_encode
 *  \brief Set an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS offset of the subtitles (in timescale units)
 */

void lqt_set_text_pts_offset(quicktime_t * file, int track, int64_t offset)
  {
  quicktime_trak_t * trak;
  if((track < 0) && (track >= file->total_ttracks))
    return;
  trak = file->ttracks[track].track;
  trak->pts_offset = offset;
  
  }

  
/** \ingroup text_decode
 *  \brief Get an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS offset of the subtitles (in timescale units)
 */

int64_t lqt_get_text_pts_offset(quicktime_t * file, int track)
  {
  quicktime_trak_t * trak;
  if((track < 0) && (track >= file->total_ttracks))
    return 0;
  trak = file->ttracks[track].track;
  return trak->pts_offset;
  
  }


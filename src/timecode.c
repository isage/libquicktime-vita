/*******************************************************************************
 timecode.c

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

#include <stdlib.h>
#include <string.h>

#include "lqt_private.h"

#define TIMECODES_PER_CHUNK 16

void lqt_add_timecode_track(quicktime_t * file, int track,
                            uint32_t flags, int framerate)
  {
  quicktime_video_map_t * vm = file->vtracks + track;
  int time_scale, frame_duration, constant;
  int width, height;
  double fps;

  time_scale = lqt_video_time_scale(file, track);
  width = quicktime_video_width(file, track);
  height = quicktime_video_height(file, track);
  frame_duration = lqt_frame_duration(file, track, &constant);

  fps = (double)(time_scale) / (double)(frame_duration);
  
  vm->timecode_track = quicktime_add_track(file);
  quicktime_trak_init_timecode(file, vm->timecode_track, time_scale, frame_duration,
                               framerate, width, height, flags);
  
  /* reference timecode track */
  quicktime_tref_init_tmcd(&vm->track->tref, vm->timecode_track->tkhd.track_id);
  vm->track->has_tref = 1;

  vm->timecodes = malloc(TIMECODES_PER_CHUNK * sizeof(*vm->timecodes));
  vm->cur_timecode_chunk = 0;
  }
  
void lqt_write_timecode(quicktime_t * file, int track,
                        uint32_t timecode)
  {
  quicktime_video_map_t * vm = file->vtracks + track;

  vm->encode_timecode = timecode;
  vm->has_encode_timecode = 1;

  /* Rest is done by lqt_flush_timecode (see below) */
  
  }


int lqt_has_timecode_track(quicktime_t * file, int track,
                           uint32_t * flags, int * framerate)
  {
  quicktime_video_map_t * vm = file->vtracks + track;

  if(vm->timecode_track)
    {
    if(flags)
      *flags = vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.flags;
    if(framerate)
      *framerate = vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.numframes;
    
    return 1;
    }
  else
    return 0;
  }

static uint32_t * cache_timecodes(quicktime_t * file,
                                  quicktime_trak_t * trak, int * total)
  {
  uint32_t * ret;
  int i, j;
  int num_read = 0, num;
  int stsc_pos = 0;

  *total = quicktime_track_samples(file, trak);
  ret = malloc(*total * sizeof(*ret));
  
  for(i = 0; i < trak->mdia.minf.stbl.stco.total_entries; i++)
    {
    if((stsc_pos < trak->mdia.minf.stbl.stsc.total_entries-1) &&
       /* chunks start with 1 */
       (i + 2 == trak->mdia.minf.stbl.stsc.table[stsc_pos+1].chunk))
      {
      stsc_pos++;
      }

    num = trak->mdia.minf.stbl.stsc.table[stsc_pos].samples;

/*
    fprintf(stderr, "Reading timecode chunk: num: %d off: %ld samples: %d\n",
            i, trak->mdia.minf.stbl.stco.table[i].offset, num);
*/
    
    quicktime_set_position(file, trak->mdia.minf.stbl.stco.table[i].offset);

    for(j = 0; j < num; j++)
      {
      ret[num_read + j] = quicktime_read_int32(file);
      // quicktime_read_data(file, (uint8_t*)(ret + num_read), num * sizeof(*ret));
      }
    
    num_read += num;
    }
  
  return ret;
  }

int lqt_read_timecode(quicktime_t * file, int track,
                      uint32_t * timecode)
  {
  int64_t sample;
  int64_t time;
  int64_t stts_index;
  int64_t stts_count;
  
  quicktime_video_map_t * vm = file->vtracks + track;
  if(!vm->timecode_track)
    return 0;

  /* Read all timecodes at once */
  if(!vm->timecodes)
    {
    vm->timecodes = cache_timecodes(file,
                                    vm->timecode_track,
                                    &vm->num_timecodes);
    }

  time = vm->timestamp;
  sample =
    quicktime_time_to_sample(&vm->timecode_track->mdia.minf.stbl.stts,
                             &time, &stts_index, &stts_count);
  /* Got a timecode */
  if(time == vm->timestamp)
    {
    *timecode = vm->timecodes[sample];
    return 1;
    }
  return 0;
  }

/* private to lqt */

void lqt_flush_timecode(quicktime_t * file, int track, int64_t time,
                        int force)
  {
  int i;
  quicktime_video_map_t * vm = file->vtracks + track;
  quicktime_stts_t * stts;

  if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
    return;

  if(!force)
    {
    if(!vm->has_encode_timecode)
      return;
    
    /* Append to array */
  
    vm->timecodes[vm->num_timecodes] = vm->encode_timecode;
    vm->has_encode_timecode = 0;
    vm->num_timecodes++;

    /* Update stts */

    if(vm->timecodes_written)
      {
      stts = &vm->timecode_track->mdia.minf.stbl.stts;
      quicktime_update_stts(stts, vm->timecodes_written-1,
                            time - vm->timecode_timestamp);
      vm->timecode_timestamp = time;
      }
    
    vm->timecodes_written++;
    }
  else
    {
    if(vm->timecodes_written)
      {
      stts = &vm->timecode_track->mdia.minf.stbl.stts;
      quicktime_update_stts(stts, vm->timecodes_written-1,
                            time - vm->timecode_timestamp);
      vm->timecode_timestamp = time;
      }
    }
  
  /* Write chunk */
  if(vm->num_timecodes &&
     ((vm->num_timecodes >= TIMECODES_PER_CHUNK) || force))
    {
    quicktime_write_chunk_header(file, vm->timecode_track);

    for(i = 0; i < vm->num_timecodes; i++)
      quicktime_write_int32(file, vm->timecodes[i]);

    
    vm->timecode_track->chunk_samples = vm->num_timecodes;
    quicktime_write_chunk_footer(file, vm->timecode_track);
    vm->cur_timecode_chunk++;
    vm->num_timecodes = 0;
    }
  }

const char * lqt_get_timecode_tape_name(quicktime_t * file, int track)
  {
    quicktime_video_map_t * vm = file->vtracks + track;
    return vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.name;
  }

void lqt_set_timecode_tape_name(quicktime_t * file, int track,
				const char * tapename)
  {
  quicktime_video_map_t * vm = file->vtracks + track;
  if(vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.name)
    free(vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.name);
  vm->timecode_track->mdia.minf.stbl.stsd.table[0].tmcd.name = strdup(tapename);
  }

int lqt_get_timecode_track_enabled(quicktime_t * file, int track)
  {
    quicktime_video_map_t * vm = file->vtracks + track;
    return vm->timecode_track->tkhd.flags & 1;
  }

void lqt_set_timecode_track_enabled(quicktime_t * file, int track,
				    int enabled)
  {
    quicktime_video_map_t * vm = file->vtracks + track;
    if(enabled)
      vm->timecode_track->tkhd.flags |= 1;
    else
      vm->timecode_track->tkhd.flags &= ~1;
  }

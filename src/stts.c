/*******************************************************************************
 stts.c

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
#include <stdlib.h>
#include <string.h>

void quicktime_stts_init(quicktime_stts_t *stts)
  {
  stts->version = 0;
  stts->flags = 0;
  stts->total_entries = 0;
  }

void quicktime_stts_init_table(quicktime_stts_t *stts)
  {
  if(!stts->total_entries)
    {
    stts->total_entries = 1;
    stts->entries_allocated = 1;
    stts->table = calloc(stts->total_entries, sizeof(*stts->table));
    }
  }

void quicktime_stts_init_qtvr(quicktime_t *file,
                              quicktime_stts_t *stts, int frame_duration)
  {
  quicktime_stts_table_t *table;
  quicktime_stts_init_table(stts);
  table = &stts->table[0];
  table->sample_duration = frame_duration;
  }

void quicktime_stts_init_panorama(quicktime_t *file,
                                  quicktime_stts_t *stts, int frame_duration)
  {
  quicktime_stts_table_t *table;
  quicktime_stts_init_table(stts);
  table = &stts->table[0];
  table->sample_duration = frame_duration;
  }

void quicktime_stts_init_video(quicktime_t *file, quicktime_stts_t *stts,
                               int frame_duration)
  {
  quicktime_stts_table_t *table;
  quicktime_stts_init_table(stts);
  table = &stts->table[0];
  table->sample_duration = frame_duration;
  stts->default_duration = frame_duration;
  }

void quicktime_stts_init_audio(quicktime_t *file, quicktime_stts_t *stts,
                               int sample_rate)
  {
  quicktime_stts_table_t *table;
  quicktime_stts_init_table(stts);
  table = &stts->table[0];
  table->sample_duration = 1;
  }

void quicktime_stts_init_timecode(quicktime_t *file, quicktime_stts_t *stts)
  {
  quicktime_stts_init_table(stts);
  }

void quicktime_stts_delete(quicktime_stts_t *stts)
  {
  if(stts->total_entries) free(stts->table);
  stts->total_entries = 0;
  }

void quicktime_stts_dump(quicktime_stts_t *stts)
  {
  int i;
  lqt_dump("     time to sample (stts)\n");
  lqt_dump("      version %d\n", stts->version);
  lqt_dump("      flags %ld\n", stts->flags);
  lqt_dump("      total_entries %ld\n", stts->total_entries);
  for(i = 0; i < stts->total_entries; i++)
    {
    lqt_dump("       count %d duration %d\n",
             stts->table[i].sample_count, stts->table[i].sample_duration);
    }
  }

void quicktime_read_stts(quicktime_t *file, quicktime_stts_t *stts)
  {
  int i;
  stts->version = quicktime_read_char(file);
  stts->flags = quicktime_read_int24(file);
  stts->total_entries = quicktime_read_int32(file);

  stts->table = malloc(sizeof(*stts->table) * stts->total_entries);
  for(i = 0; i < stts->total_entries; i++)
    {
    stts->table[i].sample_count = quicktime_read_int32(file);
    stts->table[i].sample_duration = quicktime_read_int32(file);
    }
  }

void quicktime_write_stts(quicktime_t *file, quicktime_stts_t *stts)
  {
  int i;
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "stts");

  quicktime_write_char(file, stts->version);
  quicktime_write_int24(file, stts->flags);
  quicktime_write_int32(file, stts->total_entries);
  for(i = 0; i < stts->total_entries; i++)
    {
    quicktime_write_int32(file, stts->table[i].sample_count);
    quicktime_write_int32(file, stts->table[i].sample_duration);
    }

  quicktime_atom_write_footer(file, &atom);
  }

int64_t quicktime_time_to_sample(quicktime_stts_t *stts, int64_t * time,
                                 int64_t * stts_index, int64_t * stts_count)
  {
  int64_t ret = 0;
  int64_t time_count = 0;

  *stts_index = 0;

  while(1)
    {
    if(time_count + stts->table[*stts_index].sample_duration *
       stts->table[*stts_index].sample_count >= *time)
      {
      *stts_count = (*time - time_count) / stts->table[*stts_index].sample_duration;

      time_count += *stts_count * stts->table[*stts_index].sample_duration;
      ret += *stts_count;
      break;
      }
    else
      {
      time_count += stts->table[*stts_index].sample_duration *
        stts->table[*stts_index].sample_count;
      ret += stts->table[*stts_index].sample_count;
      (*stts_index)++;
      }
    if(*stts_index >= stts->total_entries)
      break;
    }
  *time = time_count;
  return ret;
  }

/* sample = -1 returns the total duration */

int64_t quicktime_sample_to_time(quicktime_stts_t *stts, int64_t sample,
                                 int64_t * stts_index, int64_t * stts_count)
  {
  int64_t ret = 0;
  int64_t sample_count;

  if(sample < 0)
    {
    for(*stts_index = 0; *stts_index < stts->total_entries; (*stts_index)++)
      {
      ret += stts->table[*stts_index].sample_duration *
        stts->table[*stts_index].sample_count;
      }
    return ret;
    }

  *stts_index = 0;
  //  *stts_count = 0;

  sample_count = 0;
  
  while(1)
    {
    if(sample_count + stts->table[*stts_index].sample_count > sample)
      {
      *stts_count = (sample - sample_count);

      ret += *stts_count * stts->table[*stts_index].sample_duration;
      break;
      }
    else
      {
      sample_count += stts->table[*stts_index].sample_count;
      ret += stts->table[*stts_index].sample_count * stts->table[*stts_index].sample_duration;
      (*stts_index)++;
      }
    }
  return ret;
  }

/* If one calls quicktime_update_stts(), the table is set up with one entry per sample.
   Before writing, call quicktime_compress_stts() to kick out redundant entries */

void quicktime_update_stts(quicktime_stts_t *stts, long sample, long duration)
  {
  if(sample >= stts->entries_allocated)
    {
    stts->entries_allocated = sample + 1024;
    stts->table = realloc(stts->table,
                          stts->entries_allocated * sizeof(*(stts->table)));
    }
  stts->table[sample].sample_count = 1;
  if(duration)
    stts->table[sample].sample_duration = duration;
  else
    stts->table[sample].sample_duration = stts->default_duration;
  
  if(sample >= stts->total_entries)
    stts->total_entries = sample + 1;
  }

void quicktime_compress_stts(quicktime_stts_t *stts)
  {
  long sample = 0;
  long i;
  int compress;
  
  if(stts->total_entries <= 1)
    return;
    
  while(sample < stts->total_entries)
    {
    i = 1;
    compress = 0;
    while(sample + i < stts->total_entries)
      {
      if(stts->table[sample + i].sample_duration == stts->table[sample].sample_duration)
        {
        stts->table[sample].sample_count += stts->table[sample + i].sample_count;
        compress++;
        }
      else
        {
        break;
        }
      i++;
      }

    if(stts->table[sample].sample_count > 1)
      {
      /* Compress */
      if(stts->total_entries - sample - i)
        memmove(&stts->table[sample + 1], &stts->table[sample + i],
                sizeof(stts->table[sample + i]) * (stts->total_entries - sample - i));

      stts->total_entries -= compress;
      }
    sample++;
    }
  }


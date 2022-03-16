/*******************************************************************************
 ctts.c

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

void quicktime_ctts_init(quicktime_ctts_t *ctts)
  {
  
  }

void quicktime_ctts_delete(quicktime_ctts_t *ctts)
  {
  if(ctts->table)
    free(ctts->table);
  }

void quicktime_ctts_dump(quicktime_ctts_t *ctts)
  {
  int i;
  lqt_dump("     composition time to sample (ctts)\n");
  lqt_dump("      version %d\n", ctts->version);
  lqt_dump("      flags %ld\n", ctts->flags);
  lqt_dump("      total_entries %ld\n", ctts->total_entries);
  for(i = 0; i < ctts->total_entries; i++)
    {
    lqt_dump("       count %d duration %d\n", ctts->table[i].sample_count,
           ctts->table[i].sample_duration);
    }
  }

void quicktime_read_ctts(quicktime_t *file, quicktime_ctts_t *ctts)
  {
  int i;
  ctts->version = quicktime_read_char(file);
  ctts->flags = quicktime_read_int24(file);
  ctts->total_entries = quicktime_read_int32(file);
  
  ctts->table = malloc(sizeof(quicktime_ctts_table_t) * ctts->total_entries);
  for(i = 0; i < ctts->total_entries; i++)
    {
    ctts->table[i].sample_count = quicktime_read_int32(file);
    ctts->table[i].sample_duration = quicktime_read_int32(file);
    }
  }

void quicktime_write_ctts(quicktime_t *file, quicktime_ctts_t *ctts)
  {
  int i;
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "ctts");

  quicktime_write_char(file, ctts->version);
  quicktime_write_int24(file, ctts->flags);
  quicktime_write_int32(file, ctts->total_entries);
  for(i = 0; i < ctts->total_entries; i++)
    {
    quicktime_write_int32(file, ctts->table[i].sample_count);
    quicktime_write_int32(file, ctts->table[i].sample_duration);
    }

  quicktime_atom_write_footer(file, &atom);

  }

void quicktime_update_ctts(quicktime_ctts_t *ctts, long sample, long duration)
  {
  if(sample >= ctts->entries_allocated)
    {
    ctts->entries_allocated = sample + 1024;
    ctts->table = realloc(ctts->table, ctts->entries_allocated * sizeof(*(ctts->table)));
    }
  ctts->table[sample].sample_count = 1;
  ctts->table[sample].sample_duration = duration;
  
  if(sample >= ctts->total_entries)
    ctts->total_entries = sample + 1;
  }

void quicktime_compress_ctts(quicktime_ctts_t *ctts)
  {
  long sample = 0;
  long i;
  
  while(sample < ctts->total_entries)
    {
    i = 1;

    while(sample + i < ctts->total_entries)
      {
      if(ctts->table[sample + i].sample_duration ==  ctts->table[sample].sample_duration)
        {
        ctts->table[sample].sample_count++;
        }
      else
        {
        break;
        }
      i++;
      }

    if(ctts->table[sample].sample_count > 1)
      {
      /* Compress */
      if(ctts->total_entries - sample - i)
        memmove(&ctts->table[sample + 1], &ctts->table[sample + i],
                sizeof(ctts->table[sample + i]) * (ctts->total_entries - sample - i));

      ctts->total_entries -= ctts->table[sample].sample_count-1;
      }
    sample++;
    }
  
  }

void quicktime_fix_ctts(quicktime_ctts_t *ctts)
  {
  int min_ctts = 0, i;
  for(i = 0; i < ctts->total_entries; i++)
    {
    if((int)(ctts->table[i].sample_duration) < min_ctts)
      min_ctts = (int)(ctts->table[i].sample_duration);
    }
  if(!min_ctts)
    return;
  min_ctts = -min_ctts;
  for(i = 0; i < ctts->total_entries; i++)
    {
    ctts->table[i].sample_duration = (int)(ctts->table[i].sample_duration) +
      min_ctts;
    }
  }

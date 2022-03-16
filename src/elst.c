/*******************************************************************************
 elst.c

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

void quicktime_elst_table_init(quicktime_elst_table_t *table)
  {
  table->duration = 0;
  table->time = 0;
  table->rate = 1;
  }

void quicktime_elst_table_delete(quicktime_elst_table_t *table)
  {
  }

void quicktime_read_elst_table(quicktime_t *file, quicktime_elst_table_t *table)
  {
  table->duration = quicktime_read_int32(file);
  table->time = quicktime_read_int32(file);
  table->rate = quicktime_read_fixed32(file);
  }

void quicktime_write_elst_table(quicktime_t *file, quicktime_elst_table_t *table)
  {
  quicktime_write_int32(file, table->duration);
  quicktime_write_int32(file, table->time);
  quicktime_write_fixed32(file, table->rate);
  }

void quicktime_elst_table_dump(quicktime_elst_table_t *table)
  {
  lqt_dump("    edit list table\n");
  lqt_dump("     duration %d\n", table->duration);
  lqt_dump("     time %d\n", table->time);
  lqt_dump("     rate %f\n", table->rate);
  }

void quicktime_elst_init(quicktime_elst_t *elst)
  {
  elst->version = 0;
  elst->flags = 0;
  elst->total_entries = 0;
  elst->table = 0;
  }

void quicktime_elst_init_all(quicktime_elst_t *elst)
  {
  if(!elst->total_entries)
    {
    elst->total_entries = 1;
    elst->table = realloc(NULL, sizeof(*elst->table) * elst->total_entries);
    quicktime_elst_table_init(&elst->table[0]);
    }
  }

void quicktime_elst_fix_counts(quicktime_elst_t *elst,
                               int moov_scale, quicktime_trak_t * trak, int timescale)
  {
  int64_t offset_scaled;
  offset_scaled = (int64_t)((double)trak->pts_offset / timescale * moov_scale + 0.5);
  elst->table[0].duration = trak->tkhd.duration;
  
  if(trak->pts_offset < 0)
    {
    elst->table[0].time = -trak->pts_offset;
    }
  else if(offset_scaled > 0)
    {
    /* Insert empty edit */
    elst->total_entries++;
    elst->table = realloc(elst->table, sizeof(*elst->table) * elst->total_entries);
    memmove(elst->table + 1, elst->table, sizeof(*elst->table) * (elst->total_entries-1));
    elst->table[0].time = -1;
    elst->table[0].duration = offset_scaled;
    elst->table[0].rate = 1.0;
    }
  }

int64_t quicktime_elst_get_pts_offset(quicktime_elst_t *elst,
                                      int moov_scale, int timescale)
  {
  if(elst->total_entries == 1)
    {
    if(elst->table[0].time > 0)
      return - elst->table[0].time;
    }
  /* Detect empty edit */
  else if((elst->total_entries == 2) &&
          (elst->table[0].time == -1))
    return (int64_t)((double)elst->table[0].duration / moov_scale * timescale + 0.5);
  return 0;
  }

void quicktime_elst_delete(quicktime_elst_t *elst)
  {
  int i;
  if(elst->total_entries)
    {
    for(i = 0; i < elst->total_entries; i++)
      quicktime_elst_table_delete(&elst->table[i]);
    free(elst->table);
    }
  elst->total_entries = 0;
  }

void quicktime_elst_dump(quicktime_elst_t *elst)
  {
  int i;
  lqt_dump("   edit list (elst)\n");
  lqt_dump("    version %d\n", elst->version);
  lqt_dump("    flags %ld\n", elst->flags);
  lqt_dump("    total_entries %ld\n", elst->total_entries);
  
  for(i = 0; i < elst->total_entries; i++)
    {
    quicktime_elst_table_dump(&elst->table[i]);
    }
  }

void quicktime_read_elst(quicktime_t *file, quicktime_elst_t *elst)
  {
  int i;

  elst->version = quicktime_read_char(file);
  elst->flags = quicktime_read_int24(file);
  elst->total_entries = quicktime_read_int32(file);
  elst->table = (quicktime_elst_table_t*)calloc(1, sizeof(quicktime_elst_table_t) * elst->total_entries);
  for(i = 0; i < elst->total_entries; i++)
    {
    quicktime_elst_table_init(&elst->table[i]);
    quicktime_read_elst_table(file, &elst->table[i]);
    }
  }

void quicktime_write_elst(quicktime_t *file, quicktime_elst_t *elst)
  {
  quicktime_atom_t atom;
  int i;
  quicktime_atom_write_header(file, &atom, "elst");
  
  quicktime_write_char(file, elst->version);
  quicktime_write_int24(file, elst->flags);
  quicktime_write_int32(file, elst->total_entries);
  for(i = 0; i < elst->total_entries; i++)
    quicktime_write_elst_table(file, &elst->table[i]);
  quicktime_atom_write_footer(file, &atom);
  }

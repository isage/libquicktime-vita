/*******************************************************************************
 stss.c

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

void quicktime_stss_init(quicktime_stss_t *stss)
  {
  stss->version = 0;
  stss->flags = 0;
  stss->total_entries = 0;
  stss->entries_allocated = 0;
  stss->table = NULL;
  }

void quicktime_stss_delete(quicktime_stss_t *stss)
  {
  if(stss->table) free(stss->table);
  stss->total_entries = 0;
  stss->entries_allocated = 0;
  stss->table = 0;
  }

void quicktime_stss_dump(quicktime_stss_t *stss)
  {
  int i;
  lqt_dump("     sync sample (stss)\n");
  lqt_dump("      version %d\n", stss->version);
  lqt_dump("      flags %ld\n", stss->flags);
  lqt_dump("      total_entries %ld\n", stss->total_entries);
  for(i = 0; i < stss->total_entries; i++)
    {
    lqt_dump("       sample %lx\n", stss->table[i].sample);
    }
  }

void quicktime_read_stss(quicktime_t *file, quicktime_stss_t *stss)
  {
  int i;
  stss->version = quicktime_read_char(file);
  stss->flags = quicktime_read_int24(file);
  stss->total_entries = quicktime_read_int32(file);

  if(stss->entries_allocated < stss->total_entries)
    {
    stss->entries_allocated = stss->total_entries;
    stss->table = (quicktime_stss_table_t*)realloc(stss->table, sizeof(quicktime_stss_table_t) * stss->entries_allocated);
    }

  for(i = 0; i < stss->total_entries; i++)
    {
    stss->table[i].sample = quicktime_read_int32(file);
    }
  }


void quicktime_write_stss(quicktime_t *file, quicktime_stss_t *stss)
  {
  int i;
  quicktime_atom_t atom;

  if(stss->total_entries)
    {
    quicktime_atom_write_header(file, &atom, "stss");

    quicktime_write_char(file, stss->version);
    quicktime_write_int24(file, stss->flags);
    quicktime_write_int32(file, stss->total_entries);
    for(i = 0; i < stss->total_entries; i++)
      {
      quicktime_write_int32(file, stss->table[i].sample);
      }

    quicktime_atom_write_footer(file, &atom);
    }
  }

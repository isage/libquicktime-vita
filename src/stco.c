/*******************************************************************************
 stco.c

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

#define LOG_DOMAIN "stco"

void quicktime_stco_init(quicktime_stco_t *stco)
  {
  stco->version = 0;
  stco->flags = 0;
  stco->total_entries = 0;
  stco->entries_allocated = 0;
  }

void quicktime_stco_delete(quicktime_stco_t *stco)
  {
  if(stco->table) free(stco->table);
  stco->total_entries = 0;
  stco->entries_allocated = 0;
  }

void quicktime_stco_init_common(quicktime_t *file, quicktime_stco_t *stco)
  {
  if(!stco->entries_allocated)
    {
    stco->entries_allocated = 2048;
    stco->total_entries = 0;
    stco->table = (quicktime_stco_table_t*)malloc(sizeof(quicktime_stco_table_t) * stco->entries_allocated);
    }
  }

void quicktime_stco_dump(quicktime_stco_t *stco)
  {
  int i;
  if(stco->co64)
    lqt_dump("     chunk offset (co64)\n");
  else
    lqt_dump("     chunk offset (stco)\n");
  lqt_dump("      version %d\n", stco->version);
  lqt_dump("      flags %ld\n", stco->flags);
  lqt_dump("      total_entries %ld\n", stco->total_entries);
  for(i = 0; i < stco->total_entries; i++)
    {
    lqt_dump("       offset %d %"PRId64" (%"PRIx64")\n",
             i, stco->table[i].offset,
             stco->table[i].offset);
    }
  }

void quicktime_read_stco(quicktime_t *file, quicktime_stco_t *stco)
  {
  int i;
  stco->version = quicktime_read_char(file);
  stco->flags = quicktime_read_int24(file);
  stco->total_entries = quicktime_read_int32(file);
  stco->entries_allocated = stco->total_entries;
  stco->table = (quicktime_stco_table_t*)calloc(1, sizeof(quicktime_stco_table_t) * stco->entries_allocated);
  for(i = 0; i < stco->total_entries; i++)
    {
    stco->table[i].offset = quicktime_read_uint32(file);
    }
  }

void quicktime_read_stco64(quicktime_t *file, quicktime_stco_t *stco)
  {
  int i;
  stco->version = quicktime_read_char(file);
  stco->flags = quicktime_read_int24(file);
  stco->total_entries = quicktime_read_int32(file);
  stco->entries_allocated = stco->total_entries;
  stco->table = (quicktime_stco_table_t*)calloc(1, sizeof(quicktime_stco_table_t) * stco->entries_allocated);
  for(i = 0; i < stco->total_entries; i++)
    {
    stco->table[i].offset = quicktime_read_int64(file);
    }
  stco->co64 = 1;
  }

void quicktime_write_stco(quicktime_t *file, quicktime_stco_t *stco)
  {
  int i;
  quicktime_atom_t atom;

  if(stco->co64)
    quicktime_atom_write_header(file, &atom, "co64");
  else
    quicktime_atom_write_header(file, &atom, "stco");
        
  quicktime_write_char(file, stco->version);
  quicktime_write_int24(file, stco->flags);
  quicktime_write_int32(file, stco->total_entries);

  if(stco->co64)
    {
    for(i = 0; i < stco->total_entries; i++)
      quicktime_write_int64(file, stco->table[i].offset);
    }
  else
    {
    for(i = 0; i < stco->total_entries; i++)
      quicktime_write_int32(file, stco->table[i].offset);
    }
  quicktime_atom_write_footer(file, &atom);
  }

void quicktime_update_stco(quicktime_stco_t *stco, long chunk, int64_t offset)
  {
  // Chunk starts at 1
  chunk++;
  if(chunk <= 0)
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
            "quicktime_update_stco chunk must start at 1. chunk=%ld\n",
            chunk);
        
  if(chunk > stco->entries_allocated)
    {
    stco->entries_allocated = chunk * 2;
    stco->table = (quicktime_stco_table_t*)realloc(stco->table, sizeof(quicktime_stco_table_t) * stco->entries_allocated);
    }
	
  stco->table[chunk - 1].offset = offset;
  if(chunk > stco->total_entries) stco->total_entries = chunk;
  if(offset >= 0x100000000LL)
    stco->co64 = 1;
  }


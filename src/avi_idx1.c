/*******************************************************************************
 avi_idx1.c

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

typedef struct
  {
  char tag[4];
  int32_t flags;
  int32_t offset;
  int32_t size;
  } avi_tag_t;



void quicktime_delete_idx1(quicktime_idx1_t *idx1)
  {
  if(idx1->table) free(idx1->table);
  }

void quicktime_idx1_dump(quicktime_idx1_t *idx1)
  {
  int i;
  lqt_dump("idx1\n");
  for(i = 0; i < idx1->table_size; i++)
    {
    quicktime_idx1table_t *idx1table = idx1->table + i;
    lqt_dump("  T: %c%c%c%c, F: %08x, O: %d, S: %d\n",
             idx1table->tag[0],
             idx1table->tag[1],
             idx1table->tag[2],
             idx1table->tag[3],
             idx1table->flags,
             idx1table->offset,
             idx1table->size);
    }

  }


void quicktime_read_idx1(quicktime_t *file, 
                         quicktime_riff_t *riff,
                         quicktime_atom_t *parent_atom)
  {
  int i;
  quicktime_idx1_t *idx1 = &riff->idx1;


  // Allocate table.
  idx1->table_size = (parent_atom->end - quicktime_position(file)) / 16;

  if(idx1->table_size <= 0)
    {
    idx1->table_size = 0;
    return;
    }
  idx1->table_allocation = idx1->table_size;
  idx1->table = calloc(sizeof(quicktime_idx1table_t), idx1->table_size);

  // Store it in idx1 table now.
  // Wait for full ix table discovery before converting to stco.
  for(i = 0; i < idx1->table_size; i++)
    {
    quicktime_idx1table_t *idx1table = idx1->table + i;

    quicktime_read_data(file, (uint8_t*)(idx1table->tag), 4);
    idx1table->flags = quicktime_read_int32_le(file);
    idx1table->offset = quicktime_read_int32_le(file);
    idx1table->size = quicktime_read_int32_le(file);
    }



  }

void quicktime_write_idx1(quicktime_t *file, 
                          quicktime_idx1_t *idx1)
  {
  int i;
  quicktime_idx1table_t *table = idx1->table;
  int table_size = idx1->table_size;


  // Write table
  quicktime_atom_write_header(file, &idx1->atom, "idx1");

  for(i = 0; i < table_size; i++)
    {
    quicktime_idx1table_t *entry = &table[i];
    quicktime_write_char32(file, entry->tag);
    quicktime_write_int32_le(file, entry->flags);
    quicktime_write_int32_le(file, entry->offset);
    quicktime_write_int32_le(file, entry->size);
    }


  quicktime_atom_write_footer(file, &idx1->atom);
  }

void quicktime_set_idx1_keyframe(quicktime_t *file, 
                                 quicktime_trak_t *trak,
                                 int new_keyframe)
  {
  quicktime_riff_t *riff = file->riff[0];
  quicktime_hdrl_t *hdrl = &riff->hdrl;
  quicktime_strl_t *strl = hdrl->strl[trak->tkhd.track_id - 1];
  char *tag = strl->tag;
  quicktime_idx1_t *idx1 = &riff->idx1;
  int i;
  int counter = -1;
  // Search through entire index for right numbered tag.
  // Since all the tracks are combined in the same index, this is unavoidable.
  for(i = 0; i < idx1->table_size; i++)
    {
    quicktime_idx1table_t *idx1_table = &idx1->table[i];
    if(!memcmp(idx1_table->tag, tag, 4))
      {
      counter++;
      if(counter == new_keyframe)
        {
        idx1_table->flags |= AVI_KEYFRAME;
        break;
        }
      }
    }
  }

void quicktime_update_idx1table(quicktime_t *file, 
                                quicktime_trak_t *trak, 
                                int offset,
                                int size)
  {
  quicktime_riff_t *riff = file->riff[0];
  quicktime_strl_t *strl = trak->strl;
  char *tag = strl->tag;
  quicktime_idx1_t *idx1 = &riff->idx1;
  quicktime_movi_t *movi = &riff->movi;
  quicktime_idx1table_t *idx1_table;
#if 0
  int keyframe_frame = idx1->table_size + 1;

  // Set flag for keyframe
  for(i = stss->total_entries - 1; i >= 0; i--)
    {
    if(stss->table[i].sample == keyframe_frame)
      {
      flags |= AVI_KEYFRAME;
      break;
      }
    else
      if(stss->table[i].sample < keyframe_frame)
        {
        break;
        }
    }
#endif

  // Allocation
  if(idx1->table_size >= idx1->table_allocation)
    {
    idx1->table_allocation += 1024;
    
    idx1->table = realloc(idx1->table, sizeof(*idx1->table) * idx1->table_allocation);
    memset(idx1->table + idx1->table_size, 0,
           sizeof(*idx1->table) * (idx1->table_allocation - idx1->table_size));
    }
  
  // Appendage
  idx1_table = &idx1->table[idx1->table_size];
  memcpy(idx1_table->tag, tag, 4);
        
  if(trak->mdia.minf.is_audio ||
     !trak->mdia.minf.stbl.stss.total_entries)
    idx1_table->flags = AVI_KEYFRAME;
  else
    idx1_table->flags = 0;
        
  idx1_table->offset = offset - 8 - movi->atom.start;
  idx1_table->size = size;
  idx1->table_size++;
  }





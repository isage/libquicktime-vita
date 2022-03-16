/*******************************************************************************
 avi_indx.c

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

void quicktime_delete_indx(quicktime_indx_t *indx)
  {
  int i;
  if(indx->table)
    {
    for(i = 0; i < indx->table_size; i++)
      {
      quicktime_indxtable_t *indx_table = &indx->table[i];
      if(indx_table->ix) quicktime_delete_ix(indx_table->ix);
      }
    free(indx->table);
    }
  }

void quicktime_init_indx(quicktime_t *file, 
                         quicktime_indx_t *indx, 
                         quicktime_strl_t *strl)
  {
  indx->longs_per_entry = 4;
  indx->index_subtype = 0;
  indx->index_type = AVI_INDEX_OF_INDEXES;
  memcpy(indx->chunk_id, strl->tag, 4);
  }

void quicktime_indx_init_riff(quicktime_t *file, quicktime_trak_t * trak)
  {
  quicktime_strl_t * strl;
  quicktime_indx_t * indx;
  quicktime_indxtable_t *indx_table;
  
  strl = trak->strl;
  indx = &strl->indx;

  
  if(indx->table_size >= indx->table_allocation)
    {
    int new_allocation = indx->table_allocation * 2;
    if(new_allocation < 1) new_allocation = 1;
    
    indx->table = realloc(indx->table, new_allocation * sizeof(quicktime_indxtable_t));
    memset(indx->table + indx->table_size, 0, new_allocation - indx->table_size);
    indx->table_allocation = new_allocation;
    }
  
  /* Append */
  indx_table = &indx->table[indx->table_size++];
  indx_table->ix = quicktime_new_ix(file, trak, strl);

  }

void quicktime_indx_finalize_riff(quicktime_t *file, quicktime_trak_t * trak)
  {
  quicktime_strl_t * strl;
  quicktime_indx_t * indx;
  quicktime_indxtable_t *indx_table;
  
  strl = trak->strl;
  indx = &strl->indx;

  indx_table = &indx->table[indx->table_size-1];

  quicktime_write_ix(file, trak);
  
  indx_table->index_offset = indx_table->ix->atom.start - 8;
  indx_table->index_size   = indx_table->ix->atom.size;
  indx_table->duration     = indx_table->ix->table_size;
  }


void quicktime_finalize_indx(quicktime_t *file, quicktime_indx_t * indx)
  {
  int j;
  quicktime_atom_t junk_atom;

  quicktime_set_position(file, indx->offset);
  
  /* Write indx */
  //  quicktime_set_position(file, strl->indx_offset);
  quicktime_atom_write_header(file, &indx->atom, "indx");
  /* longs per entry */
  quicktime_write_int16_le(file, indx->longs_per_entry);
  /* index sub type */
  quicktime_write_char(file, indx->index_subtype);
  /* index type */
  quicktime_write_char(file, indx->index_type);
  /* entries in use */
  quicktime_write_int32_le(file, indx->table_size);
  /* chunk ID */
  quicktime_write_char32(file, indx->chunk_id);
  /* reserved */
  quicktime_write_int32_le(file, 0);
  quicktime_write_int32_le(file, 0);
  quicktime_write_int32_le(file, 0);
          
  /* table */
  for(j = 0; j < indx->table_size; j++)
    {
    quicktime_indxtable_t *indx_table = &indx->table[j];
    quicktime_write_int64_le(file, indx_table->index_offset);
    quicktime_write_int32_le(file, indx_table->index_size);
    quicktime_write_int32_le(file, indx_table->duration);
    }
          
  quicktime_atom_write_footer(file, &indx->atom);

  quicktime_atom_write_header(file, &junk_atom, "JUNK");

  while(quicktime_position(file) < indx->offset + indx->size)
    quicktime_write_char(file, 0);
  quicktime_atom_write_footer(file, &junk_atom);
  }

void quicktime_read_indx(quicktime_t *file, 
                         quicktime_strl_t *strl, 
                         quicktime_atom_t *parent_atom)
  {
  quicktime_indx_t *indx = &strl->indx;
  quicktime_indxtable_t *indx_table;
  quicktime_ix_t *ix;
  int i;
  int64_t offset;

  file->file_type = LQT_FILE_AVI_ODML;
        
  indx->longs_per_entry = quicktime_read_int16_le(file);
  indx->index_subtype = quicktime_read_char(file);
  indx->index_type = quicktime_read_char(file);
  indx->table_size = quicktime_read_int32_le(file);
  quicktime_read_char32(file, indx->chunk_id);
  quicktime_read_int32_le(file);
  quicktime_read_int32_le(file);
  quicktime_read_int32_le(file);

  /* Read indx entries */
  indx->table = calloc(indx->table_size, sizeof(quicktime_indxtable_t));
  for(i = 0; i < indx->table_size; i++)
    {
    indx_table = &indx->table[i];
    indx_table->index_offset = quicktime_read_int64_le(file);
    indx_table->index_size = quicktime_read_int32_le(file);
    indx_table->duration = quicktime_read_int32_le(file);
    offset = quicktime_position(file);
                
    /* Now read the partial index */
    ix = indx_table->ix = calloc(1, sizeof(quicktime_ix_t));
    quicktime_set_position(file, indx_table->index_offset);
    quicktime_read_ix(file, ix);
    quicktime_set_position(file, offset);
    }

  }

void quicktime_set_indx_keyframe(quicktime_t *file, 
                                 quicktime_trak_t *trak,
                                 long new_keyframe)
  {
  long frame_count;
  int i;
  quicktime_indx_t *indx = &trak->strl->indx;

  /* Get the right ix table */
  frame_count = 0;
  i = 0;


  while(frame_count + indx->table[i].ix->table_size < new_keyframe)
    {
    frame_count+= indx->table[i].ix->table_size;
    i++;
    }
  indx->table[i].ix->table[new_keyframe - frame_count].size &= 0x7fffffff;
  }

void quicktime_indx_dump(quicktime_indx_t *indx)
  {
  int i;
  lqt_dump(" indx");
  lqt_dump(" longs_per_entry: %d\n", indx->longs_per_entry);
  lqt_dump(" index_subtype:   %d\n", indx->index_subtype);
  lqt_dump(" index_type:      %d\n", indx->index_type);
  lqt_dump(" chunk_id:        %s\n", indx->chunk_id);
  lqt_dump(" table_size:      %d\n", indx->table_size);

  for(i = 0; i < indx->table_size; i++)
    {
    lqt_dump("   index_offset: %"PRId64"\n", indx->table[i].index_offset);
    lqt_dump("   index_size:   %d\n", indx->table[i].index_size);
    lqt_dump("   duration:     %d\n", indx->table[i].duration);
    quicktime_ix_dump(indx->table[i].ix);
    }
  
  }


/*******************************************************************************
 stsd.c

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

#define LOG_DOMAIN "stsd"

/* Tape name in timecode track */
#define TCMI_NAME "Untitled"

void quicktime_stsd_init(quicktime_stsd_t *stsd)
  {
  stsd->version = 0;
  stsd->flags = 0;
  stsd->total_entries = 0;
  }

void quicktime_stsd_init_table(quicktime_stsd_t *stsd)
  {
  if(!stsd->total_entries)
    {
    stsd->total_entries = 1;
    stsd->table = (quicktime_stsd_table_t*)calloc(1, sizeof(quicktime_stsd_table_t) * stsd->total_entries);
    quicktime_stsd_table_init(&stsd->table[0]);
    }
  }

int quicktime_stsd_init_qtvr(quicktime_t *file, 
                             quicktime_stsd_t *stsd,
                             int track_type)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_init_table(stsd);

  table = &stsd->table[0];
  switch(track_type)
	{
	case QTVR_OBJ:
	case QTVR_PAN:
	  table->format[0] = '\0';
	  table->format[1] = '\0';
	  table->format[2] = '\0';
	  table->format[3] = '\0';
	  break;
	case QTVR_QTVR_OBJ:
	case QTVR_QTVR_PAN:
	  table->format[0] = 'q';
	  table->format[1] = 't';
	  table->format[2] = 'v';
	  table->format[3] = 'r';
	  break;
	default:
	  lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
			  "quicktime_stsd_init_qtvr invalid track type supplied.");
	  return -1;
	}

  return 0;
  }


void quicktime_stsd_init_panorama(quicktime_t *file, 
                                  quicktime_stsd_t *stsd,
                                  int width,
                                  int height)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_init_table(stsd);
	
  table = &stsd->table[0];
  table->format[0] = 'p';
  table->format[1] = 'a';
  table->format[2] = 'n';
  table->format[3] = 'o';
  table->pano.SWidth = width;
  table->pano.SHeight = height;
  }

void quicktime_stsd_set_video_codec(quicktime_stsd_t *stsd, 
                                    char * compression)
  {
  quicktime_stsd_table_t *table;
  table = &stsd->table[0];
  quicktime_copy_char32(table->format, compression);
  }

void quicktime_stsd_init_video(quicktime_t *file, 
                               quicktime_stsd_t *stsd, 
                               int frame_w,
                               int frame_h, 
                               char * compression)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_init_table(stsd);
  table = &stsd->table[0];
  
  if(compression)
    quicktime_stsd_set_video_codec(stsd, compression);
  
  table->width = frame_w;
  table->height = frame_h;
  table->frames_per_sample = 1;
  table->depth = 24;
  table->ctab_id = 65535;
  }

void quicktime_stsd_set_audio_codec(quicktime_stsd_t *stsd, 
                                    char * compressor)
  {
  quicktime_stsd_table_t *table;
  table = &stsd->table[0];
  quicktime_copy_char32(table->format, compressor);
  quicktime_copy_char32(table->wave.frma.codec, compressor);
  }

void quicktime_stsd_init_audio(quicktime_t *file, 
                               quicktime_stsd_t *stsd, 
                               int channels,
                               int sample_rate, 
                               int bits, 
                               char *compressor)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_init_table(stsd);
  table = &stsd->table[0];
  
  if(compressor)
    quicktime_stsd_set_audio_codec(stsd, compressor);
  
  table->channels = channels;
  table->sample_size = bits;
  table->samplerate = sample_rate;
  }

void quicktime_stsd_init_text(quicktime_t *file, 
                              quicktime_stsd_t *stsd)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_text_t * text;
  
  quicktime_stsd_init_table(stsd);
  table = &stsd->table[0];

  table->format[0] = 't';
  table->format[1] = 'e';
  table->format[2] = 'x';
  table->format[3] = 't';
  
  text = &table->text;

  text->displayFlags = 0;
  text->textJustification = 1;
  text->bgColor[0] = 0;
  text->bgColor[1] = 0;
  text->bgColor[2] = 0;

  text->defaultTextBox[0] = 0;
  text->defaultTextBox[1] = 0;
  text->defaultTextBox[2] = 0;
  text->defaultTextBox[3] = 0;

  text->scrpStartChar = 0;

  text->scrpHeight =  16;
  text->scrpFont   =   0;
  text->scrpFace   =   0;
  text->scrpSize   =  12;

  text->scrpColor[0] = 65535;
  text->scrpColor[1] = 65535;
  text->scrpColor[2] = 65535;

  strcpy(text->font_name, "Sans-Serif");
  }

void quicktime_stsd_init_tx3g(quicktime_t *file, 
                              quicktime_stsd_t *stsd)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_tx3g_t *tx3g;
  
  quicktime_stsd_init_table(stsd);
  table = &stsd->table[0];

  table->format[0] = 't';
  table->format[1] = 'x';
  table->format[2] = '3';
  table->format[3] = 'g';
  
  tx3g = &table->tx3g;

  tx3g->horizontal_justification = 1;
  tx3g->vertical_justification   = 255;
  
  tx3g->back_color[0] = 0;
  tx3g->back_color[1] = 0;
  tx3g->back_color[2] = 0;
  tx3g->back_color[3] = 0;

  tx3g->defaultTextBox[0] = 0;
  tx3g->defaultTextBox[1] = 0;
  tx3g->defaultTextBox[2] = 0;
  tx3g->defaultTextBox[3] = 0;
  tx3g->start_char_offset = 0;
  tx3g->end_char_offset   = 0;
  tx3g->font_id           = 1;
  tx3g->style_flags       = 0;
  tx3g->font_size         = 12;
  tx3g->text_color[0]     = 255;
  tx3g->text_color[1]     = 255;
  tx3g->text_color[2]     = 255;
  tx3g->text_color[3]     = 255;
  quicktime_ftab_init(&tx3g->ftab, 1, "Sans-Serif");
  tx3g->has_ftab = 1;
  }

void quicktime_stsd_init_timecode(quicktime_t *file,
				  quicktime_stsd_t *stsd,
				  int timescale,
				  int frameduration,
				  int numframes, uint32_t flags)
  {
  quicktime_stsd_table_t *table;
  quicktime_stsd_init_table(stsd);
  table = &stsd->table[0];
  
  table->format[0] = 't';
  table->format[1] = 'm';
  table->format[2] = 'c';
  table->format[3] = 'd';
  table->tmcd.timescale     = timescale;
  table->tmcd.frameduration = frameduration;
  table->tmcd.numframes     = numframes;
  table->tmcd.flags         = flags;
  
  table->tmcd.name = strdup(TCMI_NAME);
  }

void quicktime_stsd_delete(quicktime_stsd_t *stsd)
  {
  int i;
  if(stsd->total_entries)
    {
    for(i = 0; i < stsd->total_entries; i++)
      quicktime_stsd_table_delete(&stsd->table[i]);
    free(stsd->table);
    }

  stsd->total_entries = 0;
  }

void quicktime_stsd_dump(void *minf_ptr, quicktime_stsd_t *stsd)
  {
  int i;
  lqt_dump("     sample description (stsd)\n");
  lqt_dump("      version %d\n", stsd->version);
  lqt_dump("      flags %ld\n", stsd->flags);
  lqt_dump("      total_entries %ld\n", stsd->total_entries);
	
  for(i = 0; i < stsd->total_entries; i++)
    {
    quicktime_stsd_table_dump(minf_ptr, &stsd->table[i]);
    }
  }

void quicktime_read_stsd(quicktime_t *file, quicktime_stsd_t *stsd)
  {
  int i;

  stsd->version = quicktime_read_char(file);
  stsd->flags = quicktime_read_int24(file);
  stsd->total_entries = quicktime_read_int32(file);
  stsd->table = calloc(stsd->total_entries, sizeof(quicktime_stsd_table_t));
  for(i = 0; i < stsd->total_entries; i++)
    {
    quicktime_read_stsd_table_raw(file, &stsd->table[i]);
    }
  }

void quicktime_finalize_stsd(quicktime_t * file, quicktime_trak_t * trak,
                             quicktime_stsd_t * stsd)
  {

  int64_t old_preload_size;
  uint8_t *old_preload_buffer;
  int64_t old_preload_start;
  int64_t old_preload_end;
  int64_t old_preload_ptr;
  int64_t old_position;
  int i;

  /* Save old buffers from the file */
  old_preload_size = file->preload_size;
  old_preload_buffer = file->preload_buffer;
  old_preload_start = file->preload_start;
  old_preload_end = file->preload_end;
  old_preload_ptr = file->preload_ptr;
  old_position = quicktime_position(file);
        
  for(i = 0; i < stsd->total_entries; i++)
    {
    quicktime_stsd_table_init(&stsd->table[i]);
                
                
    quicktime_set_position(file, 0);

    file->preload_size = stsd->table[i].table_raw_size;
    file->preload_buffer = stsd->table[i].table_raw;
    file->preload_start = 0;
    file->preload_end = file->preload_start + stsd->table[i].table_raw_size;
    file->preload_ptr = 0;
                
    quicktime_read_stsd_table(file, &trak->mdia.minf, &stsd->table[i]);
    if(trak->mdia.minf.is_video && !stsd->table[i].width && !stsd->table[i].height)
      {
      stsd->table[i].width =  (int)(trak->tkhd.track_width);
      stsd->table[i].height = (int)(trak->tkhd.track_height);
      }
    }
  file->preload_size = old_preload_size;
  file->preload_buffer = old_preload_buffer;
  file->preload_start = old_preload_start;
  file->preload_end = old_preload_end;
  file->preload_ptr = old_preload_ptr;
  quicktime_set_position(file, old_position);
  }

void quicktime_write_stsd(quicktime_t *file,
                          quicktime_minf_t *minf, quicktime_stsd_t *stsd)
  {
  quicktime_atom_t atom;
  int i;
  quicktime_atom_write_header(file, &atom, "stsd");

  quicktime_write_char(file, stsd->version);
  quicktime_write_int24(file, stsd->flags);
  quicktime_write_int32(file, stsd->total_entries);
  for(i = 0; i < stsd->total_entries; i++)
    {
    quicktime_write_stsd_table(file, minf, stsd->table);
    }

  quicktime_atom_write_footer(file, &atom);
  }




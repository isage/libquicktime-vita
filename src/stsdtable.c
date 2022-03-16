/*******************************************************************************
 stsdtable.c

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
#include <stdio.h>
#include <string.h>

#define LOG_DOMAIN "stsdtable"

static void quicktime_stsdtable_read_text(quicktime_t *file,
                                          quicktime_stsd_table_t *table,
                                          quicktime_atom_t *parent_atom)
  {
  table->text.displayFlags = quicktime_read_int32(file);
  table->text.textJustification = quicktime_read_int32(file);

  table->text.bgColor[0] = quicktime_read_int16(file);
  table->text.bgColor[1] = quicktime_read_int16(file);
  table->text.bgColor[2] = quicktime_read_int16(file);

  table->text.defaultTextBox[0] = quicktime_read_int16(file);
  table->text.defaultTextBox[1] = quicktime_read_int16(file);
  table->text.defaultTextBox[2] = quicktime_read_int16(file);
  table->text.defaultTextBox[3] = quicktime_read_int16(file);

  table->text.scrpStartChar = quicktime_read_int32(file);

  table->text.scrpHeight = quicktime_read_int16(file);
  table->text.scrpAscent = quicktime_read_int16(file);
  table->text.scrpFont = quicktime_read_int16(file);
  table->text.scrpFace = quicktime_read_int16(file);
  table->text.scrpSize = quicktime_read_int16(file);
  
  table->text.scrpColor[0] = quicktime_read_int16(file);
  table->text.scrpColor[1] = quicktime_read_int16(file);
  table->text.scrpColor[2] = quicktime_read_int16(file);

  quicktime_read_pascal(file, table->text.font_name);
  }

static void quicktime_write_stsd_text(quicktime_t *file,
                                      quicktime_stsd_table_t *table)
  {
  quicktime_write_int32(file, table->text.displayFlags);
  quicktime_write_int32(file, table->text.textJustification);

  quicktime_write_int16(file, table->text.bgColor[0]);
  quicktime_write_int16(file, table->text.bgColor[1]);
  quicktime_write_int16(file, table->text.bgColor[2]);

  quicktime_write_int16(file, table->text.defaultTextBox[0]);
  quicktime_write_int16(file, table->text.defaultTextBox[1]);
  quicktime_write_int16(file, table->text.defaultTextBox[2]);
  quicktime_write_int16(file, table->text.defaultTextBox[3]);

  quicktime_write_int32(file, table->text.scrpStartChar);

  quicktime_write_int16(file, table->text.scrpHeight);
  quicktime_write_int16(file, table->text.scrpAscent);
  quicktime_write_int16(file, table->text.scrpFont);
  quicktime_write_int16(file, table->text.scrpFace);
  quicktime_write_int16(file, table->text.scrpSize);
  
  quicktime_write_int16(file, table->text.scrpColor[0]);
  quicktime_write_int16(file, table->text.scrpColor[1]);
  quicktime_write_int16(file, table->text.scrpColor[2]);

  quicktime_write_pascal(file, table->text.font_name);
  }

static void quicktime_stsdtable_dump_text(quicktime_stsd_table_t *table)
  {
  lqt_dump("       displayFlags      %08x\n", table->text.displayFlags);
  lqt_dump("       textJustification %d\n", table->text.textJustification);
  lqt_dump("       bgColor:          [%d,%d,%d]\n", table->text.bgColor[0],
           table->text.bgColor[1],table->text.bgColor[2]);
  lqt_dump("       defaultTextBox:   [%d,%d,%d,%d]\n",
           table->text.defaultTextBox[0],
           table->text.defaultTextBox[1],
           table->text.defaultTextBox[2],
           table->text.defaultTextBox[3]);
  lqt_dump("       scrpStartChar:    %d\n", table->text.scrpStartChar);
  lqt_dump("       scrpHeight:       %d\n", table->text.scrpHeight);
  lqt_dump("       scrpFont:         %d\n", table->text.scrpFont);
  lqt_dump("       scrpFace:         %d\n", table->text.scrpFace);
  lqt_dump("       scrpSize:         %d\n", table->text.scrpSize);
  lqt_dump("       scrpColor:        [%d,%d,%d]\n",
           table->text.scrpColor[0],
           table->text.scrpColor[1],
           table->text.scrpColor[2]);
  lqt_dump("       Font:             %s\n", table->text.font_name);
  }

static void
quicktime_stsdtable_read_tx3g(quicktime_t *file,
                              quicktime_stsd_table_t *table,
                              quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  table->tx3g.display_flags = quicktime_read_int32(file);
  quicktime_read_data(file, &table->tx3g.horizontal_justification, 1);
  quicktime_read_data(file, &table->tx3g.vertical_justification, 1);
  quicktime_read_data(file, table->tx3g.back_color, 4);
  table->tx3g.defaultTextBox[0] = quicktime_read_int16(file);
  table->tx3g.defaultTextBox[1] = quicktime_read_int16(file);
  table->tx3g.defaultTextBox[2] = quicktime_read_int16(file);
  table->tx3g.defaultTextBox[3] = quicktime_read_int16(file);
  table->tx3g.start_char_offset = quicktime_read_int16(file);
  table->tx3g.end_char_offset   = quicktime_read_int16(file);
  table->tx3g.font_id           = quicktime_read_int16(file);
  quicktime_read_data(file, &table->tx3g.style_flags, 1);
  quicktime_read_data(file, &table->tx3g.font_size, 1);
  quicktime_read_data(file, table->tx3g.text_color, 4);

  while(quicktime_position(file) < parent_atom->end)
    {
    quicktime_atom_read_header(file, &leaf_atom);
    if(quicktime_atom_is(&leaf_atom, "ftab"))
      {
      quicktime_read_ftab(file, &table->tx3g.ftab);
      table->tx3g.has_ftab = 1;
      }
    else
      quicktime_atom_skip(file, &leaf_atom);
    }
  }

static void
quicktime_write_stsd_tx3g(quicktime_t *file,
                          quicktime_stsd_table_t *table)
  {
  quicktime_write_int32(file, table->tx3g.display_flags);
  quicktime_write_data(file, &table->tx3g.horizontal_justification, 1);
  quicktime_write_data(file, &table->tx3g.vertical_justification, 1);
  quicktime_write_data(file, table->tx3g.back_color, 4);
  quicktime_write_int16(file, table->tx3g.defaultTextBox[0]);
  quicktime_write_int16(file, table->tx3g.defaultTextBox[1]);
  quicktime_write_int16(file, table->tx3g.defaultTextBox[2]);
  quicktime_write_int16(file, table->tx3g.defaultTextBox[3]);
  quicktime_write_int16(file, table->tx3g.start_char_offset);
  quicktime_write_int16(file, table->tx3g.end_char_offset);
  quicktime_write_int16(file, table->tx3g.font_id);
  quicktime_write_data(file, &table->tx3g.style_flags, 1);
  quicktime_write_data(file, &table->tx3g.font_size, 1);
  quicktime_write_data(file, table->tx3g.text_color, 4);
  
  if(table->tx3g.has_ftab)
    quicktime_write_ftab(file, &table->tx3g.ftab);
  }

static void quicktime_stsdtable_dump_tx3g(quicktime_stsd_table_t *table)
  {
  lqt_dump("       display_flags:            %08x\n", table->tx3g.display_flags);
  lqt_dump("       horizontal_justification: %d\n", table->tx3g.horizontal_justification);
  lqt_dump("       vertical_justification:   %d\n", table->tx3g.vertical_justification);
  lqt_dump("       back_color:               [%d,%d,%d,%d]\n",
           table->tx3g.back_color[0],
           table->tx3g.back_color[1],
           table->tx3g.back_color[2],
           table->tx3g.back_color[3]);
  lqt_dump("       defaultTextBox:           [%d,%d,%d,%d]\n",
           table->tx3g.defaultTextBox[0],
           table->tx3g.defaultTextBox[1],
           table->tx3g.defaultTextBox[2],
           table->tx3g.defaultTextBox[3]);
  lqt_dump("       start_char_offset:        %d\n", table->tx3g.start_char_offset);
  lqt_dump("       end_char_offset:          %d\n", table->tx3g.end_char_offset);
  lqt_dump("       font_id:                  %d\n", table->tx3g.font_id);
  lqt_dump("       style_flags:              %02x\n", table->tx3g.style_flags);
  lqt_dump("       font_size:                %d\n", table->tx3g.font_size);
  lqt_dump("       text_color:               [%d,%d,%d,%d]\n",
           table->tx3g.text_color[0], table->tx3g.text_color[1],
           table->tx3g.text_color[2], table->tx3g.text_color[3]);

  if(table->tx3g.has_ftab)
    {
    quicktime_ftab_dump(&table->tx3g.ftab);
    }
  }

/* Timecode */

static void
quicktime_stsdtable_read_timecode(quicktime_t *file,
                                  quicktime_stsd_table_t *table,
                                  quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  table->tmcd.reserved2 = quicktime_read_int32(file);
  table->tmcd.flags = quicktime_read_int32(file);
  table->tmcd.timescale = quicktime_read_int32(file);
  table->tmcd.frameduration = quicktime_read_int32(file);
  table->tmcd.numframes = quicktime_read_char(file);
  table->tmcd.reserved3 = quicktime_read_char(file);
  //  table->tmcd.reserved3[1] = quicktime_read_char(file);
  //  table->tmcd.reserved3[2] = quicktime_read_char(file);

  while(quicktime_position(file) < parent_atom->end)
    {
    quicktime_atom_read_header(file, &leaf_atom);
    if(quicktime_atom_is(&leaf_atom, "name"))
      {
      int len = 0;
      quicktime_read_udta_string(file,
                                 &table->tmcd.name,
                                 &len, 0);
      }
    else
      quicktime_atom_skip(file, &leaf_atom);
    }
  
  quicktime_atom_skip(file, parent_atom);
  }

static void
quicktime_write_stsd_timecode(quicktime_t *file,
                              quicktime_stsd_table_t *table)
  {
  lqt_charset_converter_t * cnv = (lqt_charset_converter_t *)0;
  quicktime_atom_t child_atom;
  quicktime_write_int32(file, table->tmcd.reserved2);
  quicktime_write_int32(file, table->tmcd.flags);
  quicktime_write_int32(file, table->tmcd.timescale);
  quicktime_write_int32(file, table->tmcd.frameduration);
  quicktime_write_char(file, table->tmcd.numframes);
  quicktime_write_char(file, table->tmcd.reserved3);
  
  quicktime_atom_write_header(file, &child_atom, "name");
  quicktime_write_udta_string(file, table->tmcd.name,
                              0, &cnv);
  
  quicktime_atom_write_footer(file, &child_atom);
  if(cnv)
    lqt_charset_converter_destroy(cnv);
  }

static void quicktime_stsdtable_dump_timecode(quicktime_stsd_table_t *table)
  {
  printf("       reserved2       %d\n", table->tmcd.reserved2);
  printf("       flags          %d\n", table->tmcd.flags);
  printf("       timescale      %d\n", table->tmcd.timescale);
  printf("       frameduration  %d\n", table->tmcd.frameduration);
  printf("       numframes      %d\n", table->tmcd.numframes);
  printf("       reserved3      %02x\n",
         table->tmcd.reserved3);
  printf("       name:          %s\n", table->tmcd.name);
  }

/* Audio */

void quicktime_read_stsd_audio(quicktime_t *file, quicktime_stsd_table_t *table,
                               quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  table->version = quicktime_read_int16(file);
  table->revision = quicktime_read_int16(file);
  quicktime_read_data(file, (uint8_t*)(table->vendor), 4);

  if(table->version < 2)
    {
    table->channels = quicktime_read_int16(file);
    table->sample_size = quicktime_read_int16(file);
    table->compression_id = quicktime_read_int16(file);
    table->packet_size = quicktime_read_int16(file);
    table->samplerate = quicktime_read_fixed32(file);
    // Kluge for fixed32 limitation
    if(table->samplerate + 65536 == 96000 ||
       table->samplerate + 65536 == 88200) table->samplerate += 65536;
          
          
    if(table->version == 1)
      {
      table->audio_samples_per_packet = quicktime_read_int32(file);
      table->audio_bytes_per_packet = quicktime_read_int32(file);
      table->audio_bytes_per_frame  = quicktime_read_int32(file);
      table->audio_bytes_per_sample  = quicktime_read_int32(file);
            
      if(table->version == 2) // Skip another 20 bytes
        quicktime_set_position(file, quicktime_position(file) + 20);
            
      }
    }
  else /* SoundDescriptionV2 */
    {
    /*
     *  SInt16     always3;
     *  SInt16     always16;
     *  SInt16     alwaysMinus2;
     *  SInt16     always0;
     *  UInt32     always65536;
     *  UInt32     sizeOfStructOnly;
     */
    quicktime_set_position(file, quicktime_position(file) + 16);
    //          quicktime_set_position(file, quicktime_position(file) + 12);
          
    /*
     * Float64    audioSampleRate;
     */
    table->samplerate = quicktime_read_double64(file);

    /*
     * UInt32     numAudioChannels;
     */

    table->channels = quicktime_read_int32(file);
          
    /*
     * SInt32     always7F000000;
     */
          
    quicktime_set_position(file, quicktime_position(file) + 4);

          

    table->sample_size = quicktime_read_int32(file);
    table->formatSpecificFlags = quicktime_read_int32(file);

    /* The following 2 are (hopefully) unused */
          
          
    table->constBytesPerAudioPacket = quicktime_read_int32(file);
    table->constLPCMFramesPerAudioPacket = quicktime_read_int32(file);

    //          quicktime_set_position(file, quicktime_position(file) + 8);

    }
        
  /* Read additional atoms */
  while(quicktime_position(file) < parent_atom->end)
    {
    quicktime_atom_read_header(file, &leaf_atom);
    if(quicktime_atom_is(&leaf_atom, "wave"))
      {
      quicktime_read_wave(file, &table->wave, &leaf_atom);
      table->has_wave = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "esds"))
      {
      quicktime_read_esds(file, &table->esds);
      table->has_esds = 1;
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, "chan"))
      {
      quicktime_read_chan(file, &table->chan);
      table->has_chan = 1;
      quicktime_atom_skip(file, &leaf_atom);
      }
    else
      {
      lqt_log(file, LQT_LOG_INFO, LOG_DOMAIN, "Skipping unknown atom \"%4s\" inside audio stsd",
              leaf_atom.type);
      quicktime_atom_skip(file, &leaf_atom);
      }
    } 

  }

void quicktime_write_stsd_audio(quicktime_t *file, quicktime_stsd_table_t *table)
  {
  int tmp_version = file->file_type & (LQT_FILE_QT|LQT_FILE_QT_OLD) ? table->version : 0;
        
  if(IS_MP4(file->file_type))
    {
    quicktime_write_int32(file, 0);
    quicktime_write_int32(file, 0);
    }
  else
    {
    quicktime_write_int16(file, tmp_version);
    quicktime_write_int16(file, table->revision);
    quicktime_write_data(file, (uint8_t*)(table->vendor), 4);
    }

  if(tmp_version < 2)
    {
    quicktime_write_int16(file, table->channels);
    quicktime_write_int16(file, (file->file_type & (LQT_FILE_QT|LQT_FILE_QT_OLD)) ? table->sample_size : 16);
    quicktime_write_int16(file, (file->file_type & (LQT_FILE_QT|LQT_FILE_QT_OLD)) ? table->compression_id : 0);
    quicktime_write_int16(file, table->packet_size);
    quicktime_write_fixed32(file, table->samplerate);
    if(tmp_version == 1)
      {
      quicktime_write_int32(file, table->audio_samples_per_packet);
      quicktime_write_int32(file, table->audio_bytes_per_packet);
      quicktime_write_int32(file, table->audio_bytes_per_frame);
      quicktime_write_int32(file, table->audio_bytes_per_sample);
      }
    }
  else
    {
    quicktime_write_int16(file, 0x0003); //                               SInt16 always3;
    quicktime_write_int16(file, 0x0010); //                               SInt16 always16;
    quicktime_write_int16(file, 0xFFFE); //                               SInt16 alwaysMinus2;
    quicktime_write_int16(file, 0x0000); //                               SInt16 always0;
    quicktime_write_int32(file, 0x00010000); //                           UInt32 always65536;
    quicktime_write_int32(file, 0x00000048); //                           UInt32 sizeOfStructOnly;
    quicktime_write_double64(file, table->samplerate); //                 Float64 audioSampleRate;
    quicktime_write_int32(file, table->channels); //                      UInt32 numAudioChannels;
    quicktime_write_int32(file, 0x7F000000); //                           SInt32 always7F000000;
    quicktime_write_int32(file, table->sample_size); //                   UInt32 constBitsPerChannel;
    quicktime_write_int32(file, table->formatSpecificFlags); //          UInt32 formatSpecificFlags;
    quicktime_write_int32(file, table->constBytesPerAudioPacket);      // UInt32 constBytesPerAudioPacket;
    quicktime_write_int32(file, table->constLPCMFramesPerAudioPacket); // UInt32 constLPCMFramesPerAudioPacket;
    }
  if(file->file_type & (LQT_FILE_QT|LQT_FILE_QT_OLD))
    {
    if(table->has_wave)
      {
      /* For quicktime, we must put the esds atom into the wave atom */
      if(table->has_esds)
        {
        memcpy(&table->wave.esds, &table->esds, sizeof(table->wave.esds));
        table->wave.has_esds = 1;
        }
      quicktime_write_wave(file, &table->wave);
      if(table->has_esds)
        {
        memset(&table->wave.esds, 0, sizeof(table->wave.esds));
        table->wave.has_esds = 0;
        }
      }
    if(table->has_chan)
      quicktime_write_chan(file, &table->chan);
    }
  else
    {
    if(table->has_esds)
      quicktime_write_esds(file, &table->esds);
    }
  quicktime_write_user_atoms(file, &table->user_atoms);
  }


void quicktime_read_stsd_table_raw(quicktime_t *file, quicktime_stsd_table_t *table)
  {
  quicktime_atom_t leaf_atom;
  int64_t old_position;
  old_position = quicktime_position(file);

  quicktime_atom_read_header(file, &leaf_atom);

  /* We write the raw atom verbatim into the raw table */
  table->table_raw_size = leaf_atom.size;

  table->table_raw = malloc(table->table_raw_size);
  quicktime_set_position(file, leaf_atom.start);
  quicktime_read_data(file, table->table_raw, table->table_raw_size);
  

  }


void quicktime_read_stsd_video(quicktime_t *file, quicktime_stsd_table_t *table,
                               quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  int len, bits_per_pixel;
        
  table->version = quicktime_read_int16(file);
  table->revision = quicktime_read_int16(file);
  quicktime_read_data(file, (uint8_t*)table->vendor, 4);
  table->temporal_quality = quicktime_read_int32(file);
  table->spatial_quality = quicktime_read_int32(file);
  table->width = quicktime_read_int16(file);
  table->height = quicktime_read_int16(file);
  table->dpi_horizontal = quicktime_read_fixed32(file);
  table->dpi_vertical = quicktime_read_fixed32(file);
  table->data_size = quicktime_read_int32(file);
  table->frames_per_sample = quicktime_read_int16(file);
  len = quicktime_read_char(file);
  quicktime_read_data(file, (uint8_t*)table->compressor_name, 31);
  table->depth = quicktime_read_int16(file);
  table->ctab_id = quicktime_read_int16(file);
        
        
  /*  If ctab_id is zero, the colortable follows immediately
   *  after the ctab ID
   */
  bits_per_pixel = table->depth & 0x1f;
  if(!table->ctab_id &&
     ((bits_per_pixel == 1) ||
      (bits_per_pixel == 2) ||
      (bits_per_pixel == 4) ||
      (bits_per_pixel == 8)))
    {
    quicktime_read_ctab(file, &table->ctab);
    table->has_ctab = 1;
    }
  else
    quicktime_default_ctab(&table->ctab, table->depth);
        
  while(quicktime_position(file) + 8 < parent_atom->end)
    {
    quicktime_atom_read_header(file, &leaf_atom);
    if(quicktime_atom_is(&leaf_atom, "ctab"))
      {
      quicktime_read_ctab(file, &table->ctab);
      table->has_ctab = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "gama"))
      {
      quicktime_read_gama(file, &table->gama);
      table->has_gama = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "fiel"))
      {
      quicktime_read_fiel(file, &table->fiel);
      table->has_fiel = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "pasp"))
      {
      quicktime_read_pasp(file, &table->pasp);
      table->has_pasp = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "clap"))
      {
      quicktime_read_clap(file, &table->clap);
      table->has_clap = 1;
      }
    else if (quicktime_atom_is(&leaf_atom, "colr"))
      {
      quicktime_read_colr(file, &table->colr);
      table->has_colr = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "esds"))
      {
      // printf("esds: %"PRId64" bytes\n", leaf_atom.size);
      quicktime_read_esds(file, &table->esds);
      table->has_esds = 1;
      quicktime_atom_skip(file, &leaf_atom);
      }
    else
      {
      quicktime_user_atoms_read_atom(file,
                                     &table->user_atoms,
                                     &leaf_atom);
      }
    quicktime_atom_skip(file, &leaf_atom);
    }
  }


void quicktime_write_stsd_video(quicktime_t *file,
                                quicktime_stsd_table_t *table)
  {
  int compressor_name_len, i;
  int terminate = 0;
  compressor_name_len = strlen(table->compressor_name);
  if(file->file_type & (LQT_FILE_QT|LQT_FILE_QT_OLD))
    {
    quicktime_write_int16(file, table->version);
    quicktime_write_int16(file, table->revision);
    quicktime_write_data(file, (uint8_t*)table->vendor, 4);
    quicktime_write_int32(file, table->temporal_quality);
    quicktime_write_int32(file, table->spatial_quality);
    quicktime_write_int16(file, table->width);
    quicktime_write_int16(file, table->height);
    quicktime_write_fixed32(file, table->dpi_horizontal);
    quicktime_write_fixed32(file, table->dpi_vertical);
    quicktime_write_int32(file, table->data_size);
    quicktime_write_int16(file, table->frames_per_sample);
    quicktime_write_char(file, compressor_name_len);
    quicktime_write_data(file, (uint8_t*)table->compressor_name, 31);
    quicktime_write_int16(file, table->depth);
    quicktime_write_int16(file, table->ctab_id);

    if (table->has_pasp)
      {
      quicktime_write_pasp(file, &table->pasp);
      terminate = 1;
      }
    if (table->has_clap)
      {
      quicktime_write_clap(file, &table->clap);
      terminate = 1;
      }
    if (table->has_colr)
      {
      quicktime_write_colr(file, &table->colr);
      terminate = 1;
      }
    if (table->has_fiel)
      {
      quicktime_write_fiel(file, &table->fiel);
      terminate = 1;
      }
    if (table->has_gama)
      {
      quicktime_write_gama(file, &table->gama);
      terminate = 1;
      }
    }
  else /* Different stsd formats for mp4 variants */
    {
    quicktime_write_int16(file, 0);
    quicktime_write_int16(file, 0);
    quicktime_write_int32(file, 0);
    quicktime_write_int32(file, 0);
    quicktime_write_int32(file, 0);
    quicktime_write_int16(file, table->width);
    quicktime_write_int16(file, table->height);
    quicktime_write_fixed32(file, 0x00480000);
    quicktime_write_fixed32(file, 0x00480000);
    quicktime_write_int32(file, 0);
    quicktime_write_int16(file, 1);
    quicktime_write_data(file, (uint8_t*)table->compressor_name, compressor_name_len);
    for(i = 0; i < 32 - compressor_name_len; i++)
      quicktime_write_char(file, 0);
    
    quicktime_write_int16(file, 24);
    quicktime_write_int16(file, -1);
    }
  quicktime_write_user_atoms(file,
                             &table->user_atoms);
  
  if(table->has_esds)
    quicktime_write_esds(file, &table->esds);
  
  if(terminate)
    quicktime_write_int32(file, 0);
  }

void quicktime_read_stsd_table(quicktime_t *file, quicktime_minf_t *minf,
                               quicktime_stsd_table_t *table)
  {
  quicktime_atom_t leaf_atom;

  quicktime_atom_read_header(file, &leaf_atom);

  table->format[0] = leaf_atom.type[0];
  table->format[1] = leaf_atom.type[1];
  table->format[2] = leaf_atom.type[2];
  table->format[3] = leaf_atom.type[3];

  quicktime_read_data(file, table->reserved, 6);
  table->data_reference = quicktime_read_int16(file);

  /* Panoramas are neither audio nor video */
  if (quicktime_match_32(leaf_atom.type, "pano"))
    {	    
    minf->is_panorama = 1;
    quicktime_read_pano(file, &table->pano, &leaf_atom);
    }
  else if (quicktime_match_32(leaf_atom.type, "qtvr"))
    {	    
    minf->is_qtvr = 1;
    quicktime_read_qtvr(file, &table->qtvr, &leaf_atom);
    }
  else if (quicktime_match_32(leaf_atom.type, "\0\0\0\0") && file->moov.udta.is_qtvr)
    {	    
    minf->is_object = 1;
    }
  else if (quicktime_match_32(leaf_atom.type, "text"))
    {
    quicktime_stsdtable_read_text(file, table, &leaf_atom);
    minf->is_text = 1;
    }
  else if (quicktime_match_32(leaf_atom.type, "tx3g"))
    {
    quicktime_stsdtable_read_tx3g(file, table, &leaf_atom);
    minf->is_text = 1;
    }
  else if (quicktime_match_32(leaf_atom.type, "tmcd"))
    {	    
    quicktime_stsdtable_read_timecode(file, table, &leaf_atom);
    minf->is_timecode = 1;
    }
  else 
    {
    if(minf->is_audio) quicktime_read_stsd_audio(file, table, &leaf_atom);
    if(minf->is_video) quicktime_read_stsd_video(file, table, &leaf_atom);
    }
  }

void quicktime_stsd_table_init(quicktime_stsd_table_t *table)
  {
  int i;
  table->format[0] = 'y';
  table->format[1] = 'u';
  table->format[2] = 'v';
  table->format[3] = '2';
  for(i = 0; i < 6; i++) table->reserved[i] = 0;
  table->data_reference = 1;

  table->version = 0;
  table->revision = 0;
  table->vendor[0] = 'l';
  table->vendor[1] = 'q';
  table->vendor[2] = 't';
  table->vendor[3] = ' ';

  table->temporal_quality = 100;
  table->spatial_quality = 258;
  table->width = 0;
  table->height = 0;
  table->dpi_horizontal = 72;
  table->dpi_vertical = 72;
  table->data_size = 0;
  table->frames_per_sample = 1;
  for(i = 0; i < 32; i++) table->compressor_name[i] = 0;
  sprintf(table->compressor_name, "%s-%s", PACKAGE, VERSION);
  table->depth = 24;
  table->ctab_id = 65535;
  quicktime_ctab_init(&table->ctab);
  quicktime_pasp_init(&table->pasp);
  quicktime_gama_init(&table->gama);
  quicktime_fiel_init(&table->fiel);
  quicktime_clap_init(&table->clap);
  quicktime_colr_init(&table->colr);
  quicktime_pano_init(&table->pano);
  quicktime_qtvr_init(&table->qtvr);
  quicktime_chan_init(&table->chan);
	
  table->channels = 0;
  table->sample_size = 0;
  table->compression_id = 0;
  table->packet_size = 0;
  table->samplerate = 0.0;
  }

void quicktime_stsd_table_delete(quicktime_stsd_table_t *table)
  {
  /* LQT: Delete table_raw as well */
  if(table->table_raw)
    free(table->table_raw);
  quicktime_ctab_delete(&table->ctab);
  quicktime_wave_delete(&table->wave);
  quicktime_esds_delete(&table->esds);
  quicktime_ftab_delete(&table->tx3g.ftab);
  quicktime_user_atoms_delete(&table->user_atoms);
  
  if(table->tmcd.name)
    free(table->tmcd.name);
  }

void quicktime_stsd_video_dump(quicktime_stsd_table_t *table)
  {
  lqt_dump("       version %d\n", table->version);
  lqt_dump("       revision %d\n", table->revision);
  lqt_dump("       vendor %c%c%c%c\n", table->vendor[0], table->vendor[1], table->vendor[2], table->vendor[3]);
  lqt_dump("       temporal_quality %ld\n", table->temporal_quality);
  lqt_dump("       spatial_quality %ld\n", table->spatial_quality);
  lqt_dump("       width %d\n", table->width);
  lqt_dump("       height %d\n", table->height);
  lqt_dump("       dpi_horizontal %f\n", table->dpi_horizontal);
  lqt_dump("       dpi_vertical %f\n", table->dpi_vertical);
  lqt_dump("       data_size %"PRId64"\n", table->data_size);
  lqt_dump("       frames_per_sample %d\n", table->frames_per_sample);
  lqt_dump("       compressor_name %s\n", table->compressor_name);
  lqt_dump("       depth %d\n", table->depth);
  lqt_dump("       ctab_id %d\n", table->ctab_id);

  if (table->has_pasp)
    quicktime_pasp_dump(&table->pasp);
  if (table->has_clap)
    quicktime_clap_dump(&table->clap);
  if (table->has_colr)
    quicktime_colr_dump(&table->colr);
  if (table->has_fiel)
    quicktime_fiel_dump(&table->fiel);
  if (table->has_gama)
    quicktime_gama_dump(&table->gama);

  if(table->has_ctab) quicktime_ctab_dump(&table->ctab);
  if(table->has_esds) quicktime_esds_dump(&table->esds);
  quicktime_user_atoms_dump(&table->user_atoms);
  }

void quicktime_stsd_audio_dump(quicktime_stsd_table_t *table)
  {
  lqt_dump("       version %d\n", table->version);
  lqt_dump("       revision %d\n", table->revision);
  lqt_dump("       vendor %c%c%c%c\n", table->vendor[0], table->vendor[1], table->vendor[2], table->vendor[3]);
  lqt_dump("       channels %d\n", table->channels);
  lqt_dump("       sample_size %d\n", table->sample_size);

  if(table->version < 2)
    {
    lqt_dump("       compression_id %d\n", table->compression_id);
    lqt_dump("       packet_size %d\n", table->packet_size);
    lqt_dump("       samplerate %f\n",  table->samplerate);
          
    if(table->version == 1)
      {
      lqt_dump("       samples_per_packet: %d\n", table->audio_samples_per_packet);
      lqt_dump("       bytes_per_packet:   %d\n", table->audio_bytes_per_packet);
      lqt_dump("       bytes_per_frame:    %d\n", table->audio_bytes_per_frame);
      lqt_dump("       bytes_per_samples:  %d\n", table->audio_bytes_per_sample);
      }
          
    }
  else if(table->version == 2)
    {
    lqt_dump("       samplerate                     %f\n",  table->samplerate);
    lqt_dump("       formatSpecificFlags:           %08x\n", table->formatSpecificFlags);
    lqt_dump("       constBytesPerAudioPacket:      %d\n", table->constBytesPerAudioPacket);
    lqt_dump("       constLPCMFramesPerAudioPacket: %d\n", table->constLPCMFramesPerAudioPacket);
    }
        
  if(table->has_wave)
    quicktime_wave_dump(&table->wave);
  if(table->has_esds)
    quicktime_esds_dump(&table->esds);
  if(table->has_chan)
    quicktime_chan_dump(&table->chan);
  quicktime_user_atoms_dump(&table->user_atoms);
  }


void quicktime_stsd_table_dump(void *minf_ptr, quicktime_stsd_table_t *table)
  {
  quicktime_minf_t *minf = minf_ptr;
  lqt_dump("       format %c%c%c%c\n",
           table->format[0], table->format[1],
           table->format[2], table->format[3]);

  quicktime_print_chars("       reserved ", table->reserved, 6);
  lqt_dump("       data_reference %d\n", table->data_reference);

  if(minf->is_audio) quicktime_stsd_audio_dump(table);
  else if(minf->is_video) quicktime_stsd_video_dump(table);
        
  else if (quicktime_match_32(table->format, "pano"))
    quicktime_pano_dump(&table->pano);
  else if (quicktime_match_32(table->format, "qtvr"))
    quicktime_qtvr_dump(&table->qtvr);
  else if(quicktime_match_32(table->format, "text"))
    quicktime_stsdtable_dump_text(table);
  else if(quicktime_match_32(table->format, "tx3g"))
    quicktime_stsdtable_dump_tx3g(table);
  else if(quicktime_match_32(table->format, "tx3g"))
    quicktime_stsdtable_dump_tx3g(table);
  else if(quicktime_match_32(table->format, "tmcd"))
    quicktime_stsdtable_dump_timecode(table);
  }

void quicktime_write_stsd_table(quicktime_t *file, quicktime_minf_t *minf, quicktime_stsd_table_t *table)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, table->format);
  quicktime_write_data(file, table->reserved, 6);
  quicktime_write_int16(file, table->data_reference);

  if(minf->is_audio) quicktime_write_stsd_audio(file, table);
  if(minf->is_video) quicktime_write_stsd_video(file, table);
  if(minf->is_qtvr == QTVR_QTVR_PAN) quicktime_write_qtvr(file, &table->qtvr);
  if(minf->is_qtvr == QTVR_QTVR_OBJ) quicktime_write_qtvr(file, &table->qtvr);
  if(minf->is_timecode) quicktime_write_stsd_timecode(file, table);
  
  if(minf->is_text)
    {
    if(quicktime_match_32(table->format, "text"))
      quicktime_write_stsd_text(file, table);
    else if(quicktime_match_32(table->format, "tx3g"))
      quicktime_write_stsd_tx3g(file, table);
    }
        
  quicktime_atom_write_footer(file, &atom);
  }

void quicktime_set_stsd_audio_v1(quicktime_stsd_table_t *table,
                                 uint32_t samples_per_packet,
                                 uint32_t bytes_per_packet,
                                 uint32_t bytes_per_frame,
                                 uint32_t bytes_per_sample)
  {
  table->version = 1;
  table->audio_samples_per_packet = samples_per_packet;
  table->audio_bytes_per_packet = bytes_per_packet;
  table->audio_bytes_per_frame = bytes_per_frame;
  table->audio_bytes_per_sample = bytes_per_sample;
  }

#if 1
void quicktime_set_stsd_audio_v2(quicktime_stsd_table_t *table,
                                 uint32_t formatSpecificFlags,
                                 uint32_t constBytesPerAudioPacket,
                                 uint32_t constLPCMFramesPerAudioPacket)
  {
  table->version = 2;
  table->formatSpecificFlags = formatSpecificFlags;
  table->constBytesPerAudioPacket = constBytesPerAudioPacket;
  table->constLPCMFramesPerAudioPacket = constLPCMFramesPerAudioPacket;
  }
#endif

uint8_t * quicktime_stsd_get_user_atom(quicktime_trak_t * trak, char * name, uint32_t * len)
  {
  quicktime_stsd_table_t *table = &trak->mdia.minf.stbl.stsd.table[0];
  return(quicktime_user_atoms_get_atom(&table->user_atoms, name, len));
  }

void quicktime_stsd_set_user_atom(quicktime_trak_t * trak, char * name,
                                  uint8_t * data, uint32_t len)
  {
  quicktime_stsd_table_t *table = &trak->mdia.minf.stbl.stsd.table[0];
  quicktime_user_atoms_add_atom(&table->user_atoms,
                                name, data, len);
  
  }

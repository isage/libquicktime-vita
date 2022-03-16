/*******************************************************************************
 iods.c

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

static const char * get_video_profile_name(uint8_t video_pl)
  {
  switch (video_pl)
    {
    case 0x00: return "Reserved (0x00) Profile";
    case 0x01: return "Simple Profile @ Level 1";
    case 0x02: return "Simple Profile @ Level 2";
    case 0x03: return "Simple Profile @ Level 3";
    case 0x08: return "Simple Profile @ Level 0";
    case 0x10: return "Simple Scalable Profile @ Level 0";
    case 0x11: return "Simple Scalable Profile @ Level 1";
    case 0x12: return "Simple Scalable Profile @ Level 2";
    case 0x15: return "AVC/H264 Profile";
    case 0x21: return "Core Profile @ Level 1";
    case 0x22: return "Core Profile @ Level 2";
    case 0x32: return "Main Profile @ Level 2";
    case 0x33: return "Main Profile @ Level 3";
    case 0x34: return "Main Profile @ Level 4";
    case 0x42: return "N-bit Profile @ Level 2";
    case 0x51: return "Scalable Texture Profile @ Level 1";
    case 0x61: return "Simple Face Animation Profile @ Level 1";
    case 0x62: return "Simple Face Animation Profile @ Level 2";
    case 0x63: return "Simple FBA Profile @ Level 1";
    case 0x64: return "Simple FBA Profile @ Level 2";
    case 0x71: return "Basic Animated Texture Profile @ Level 1";
    case 0x72: return "Basic Animated Texture Profile @ Level 2";
    case 0x81: return "Hybrid Profile @ Level 1";
    case 0x82: return "Hybrid Profile @ Level 2";
    case 0x91: return "Advanced Real Time Simple Profile @ Level 1";
    case 0x92: return "Advanced Real Time Simple Profile @ Level 2";
    case 0x93: return "Advanced Real Time Simple Profile @ Level 3";
    case 0x94: return "Advanced Real Time Simple Profile @ Level 4";
    case 0xA1: return "Core Scalable Profile @ Level1";
    case 0xA2: return "Core Scalable Profile @ Level2";
    case 0xA3: return "Core Scalable Profile @ Level3";
    case 0xB1: return "Advanced Coding Efficiency Profile @ Level 1";
    case 0xB2: return "Advanced Coding Efficiency Profile @ Level 2";
    case 0xB3: return "Advanced Coding Efficiency Profile @ Level 3";
    case 0xB4: return "Advanced Coding Efficiency Profile @ Level 4";
    case 0xC1: return "Advanced Core Profile @ Level 1";
    case 0xC2: return "Advanced Core Profile @ Level 2";
    case 0xD1: return "Advanced Scalable Texture @ Level1";
    case 0xD2: return "Advanced Scalable Texture @ Level2";
    case 0xE1: return "Simple Studio Profile @ Level 1";
    case 0xE2: return "Simple Studio Profile @ Level 2";
    case 0xE3: return "Simple Studio Profile @ Level 3";
    case 0xE4: return "Simple Studio Profile @ Level 4";
    case 0xE5: return "Core Studio Profile @ Level 1";
    case 0xE6: return "Core Studio Profile @ Level 2";
    case 0xE7: return "Core Studio Profile @ Level 3";
    case 0xE8: return "Core Studio Profile @ Level 4";
    case 0xF0: return "Advanced Simple Profile @ Level 0";
    case 0xF1: return "Advanced Simple Profile @ Level 1";
    case 0xF2: return "Advanced Simple Profile @ Level 2";
    case 0xF3: return "Advanced Simple Profile @ Level 3";
    case 0xF4: return "Advanced Simple Profile @ Level 4";
    case 0xF5: return "Advanced Simple Profile @ Level 5";
    case 0xF7: return "Advanced Simple Profile @ Level 3b";
    case 0xF8: return "Fine Granularity Scalable Profile @ Level 0";
    case 0xF9: return "Fine Granularity Scalable Profile @ Level 1";
    case 0xFA: return "Fine Granularity Scalable Profile @ Level 2";
    case 0xFB: return "Fine Granularity Scalable Profile @ Level 3";
    case 0xFC: return "Fine Granularity Scalable Profile @ Level 4";
    case 0xFD: return "Fine Granularity Scalable Profile @ Level 5";
    case 0xFE: return "Not part of MPEG-4 Visual profiles";
    case 0xFF: return "No visual capability required";
    default: return "ISO Reserved Profile";
    }

  }

static const char * get_audio_profile_name(uint8_t audio_pl)
  {
  switch (audio_pl)
    {
    case 0x00: return "ISO Reserved (0x00)";
    case 0x01: return "Main Audio Profile @ Level 1";
    case 0x02: return "Main Audio Profile @ Level 2";
    case 0x03: return "Main Audio Profile @ Level 3";
    case 0x04: return "Main Audio Profile @ Level 4";
    case 0x05: return "Scalable Audio Profile @ Level 1";
    case 0x06: return "Scalable Audio Profile @ Level 2";
    case 0x07: return "Scalable Audio Profile @ Level 3";
    case 0x08: return "Scalable Audio Profile @ Level 4";
    case 0x09: return "Speech Audio Profile @ Level 1";
    case 0x0A: return "Speech Audio Profile @ Level 2";
    case 0x0B: return "Synthetic Audio Profile @ Level 1";
    case 0x0C: return "Synthetic Audio Profile @ Level 2";
    case 0x0D: return "Synthetic Audio Profile @ Level 3";
    case 0x0E: return "High Quality Audio Profile @ Level 1";
    case 0x0F: return "High Quality Audio Profile @ Level 2";
    case 0x10: return "High Quality Audio Profile @ Level 3";
    case 0x11: return "High Quality Audio Profile @ Level 4";
    case 0x12: return "High Quality Audio Profile @ Level 5";
    case 0x13: return "High Quality Audio Profile @ Level 6";
    case 0x14: return "High Quality Audio Profile @ Level 7";
    case 0x15: return "High Quality Audio Profile @ Level 8";
    case 0x16: return "Low Delay Audio Profile @ Level 1";
    case 0x17: return "Low Delay Audio Profile @ Level 2";
    case 0x18: return "Low Delay Audio Profile @ Level 3";
    case 0x19: return "Low Delay Audio Profile @ Level 4";
    case 0x1A: return "Low Delay Audio Profile @ Level 5";
    case 0x1B: return "Low Delay Audio Profile @ Level 6";
    case 0x1C: return "Low Delay Audio Profile @ Level 7";
    case 0x1D: return "Low Delay Audio Profile @ Level 8";
    case 0x1E: return "Natural Audio Profile @ Level 1";
    case 0x1F: return "Natural Audio Profile @ Level 2";
    case 0x20: return "Natural Audio Profile @ Level 3";
    case 0x21: return "Natural Audio Profile @ Level 4";
    case 0x22: return "Mobile Audio Internetworking Profile @ Level 1";
    case 0x23: return "Mobile Audio Internetworking Profile @ Level 2";
    case 0x24: return "Mobile Audio Internetworking Profile @ Level 3";
    case 0x25: return "Mobile Audio Internetworking Profile @ Level 4";
    case 0x26: return "Mobile Audio Internetworking Profile @ Level 5";
    case 0x27: return "Mobile Audio Internetworking Profile @ Level 6";
    case 0x28: return "AAC Profile @ Level 1";
    case 0x29: return "AAC Profile @ Level 2";
    case 0x2A: return "AAC Profile @ Level 4";
    case 0x2B: return "AAC Profile @ Level 5";
    case 0x2C: return "High Efficiency AAC Profile @ Level 2";
    case 0x2D: return "High Efficiency AAC Profile @ Level 3";
    case 0x2E: return "High Efficiency AAC Profile @ Level 4";
    case 0x2F: return "High Efficiency AAC Profile @ Level 5";
    case 0xFE: return "Not part of MPEG-4 audio profiles";
    case 0xFF: return "No audio capability required";
    default: return "ISO Reserved / User Private";
    }
  }

void quicktime_iods_init(quicktime_iods_t * iods)
  {
  iods->version              = 0;
  iods->flags                = 0;
  iods->ObjectDescriptorID   = 0x004F;
  iods->ODProfileLevel       = 0xff;
  iods->sceneProfileLevel    = 0xff;
  iods->audioProfileId       = 0xff;
  iods->videoProfileId       = 0xff;
  iods->graphicsProfileLevel = 0xff;

  }

void quicktime_iods_delete(quicktime_iods_t * iods)
  {
  if(iods->tracks)
    free(iods->tracks);
  }

void quicktime_iods_dump(quicktime_iods_t * iods)
  {
  int i;
  lqt_dump("Initial object descriptor (iods)\n");
  lqt_dump("  version:              %d\n", iods->version);
  lqt_dump("  flags:                %ld\n", iods->flags);
  lqt_dump("  ObjectDescriptorID:   %04x\n", iods->ObjectDescriptorID);
  lqt_dump("  ODProfileLevel:       %d\n", iods->ODProfileLevel);
  lqt_dump("  sceneProfileLevel:    %d\n", iods->sceneProfileLevel);
  lqt_dump("  audioProfileId:       %d [%s]\n", iods->audioProfileId,
         get_audio_profile_name(iods->audioProfileId));
  lqt_dump("  videoProfileId:       %d [%s]\n", iods->videoProfileId,
         get_video_profile_name(iods->videoProfileId));
  lqt_dump("  graphicsProfileLevel: %d\n", iods->graphicsProfileLevel);
  
  for(i = 0; i < iods->num_tracks; i++)
    {
    lqt_dump("  track %d: ES_ID_IncTag: %d, length: %d, track_id: %d\n",
           i+1, iods->tracks[i].ES_ID_IncTag,
           iods->tracks[i].length, iods->tracks[i].track_id);
    }
  }

void quicktime_iods_add_track(quicktime_iods_t * iods, quicktime_trak_t * trak)
  {
  iods->tracks = realloc(iods->tracks, sizeof(*iods->tracks)*(iods->num_tracks+1));
  iods->tracks[iods->num_tracks].ES_ID_IncTag = 0x0e;
  iods->tracks[iods->num_tracks].length       = 0x04;
  iods->tracks[iods->num_tracks].track_id     = trak->tkhd.track_id;
  iods->num_tracks++;
  }

void quicktime_read_iods(quicktime_t *file, quicktime_iods_t * iods)
  {
  int len, i;
  iods->version = quicktime_read_char(file);
  iods->flags = quicktime_read_int24(file);
  quicktime_read_char(file); /* skip tag */
  len = quicktime_read_mp4_descr_length(file);  /* Length */

  iods->ObjectDescriptorID    = quicktime_read_int16(file);
  iods->ODProfileLevel        = quicktime_read_char(file);
  iods->sceneProfileLevel     = quicktime_read_char(file);
  iods->audioProfileId        = quicktime_read_char(file);
  iods->videoProfileId        = quicktime_read_char(file);
  iods->graphicsProfileLevel  = quicktime_read_char(file);
  
  iods->num_tracks = (len - 7)/6;
  iods->tracks = calloc(iods->num_tracks, sizeof(*(iods->tracks)));

  for(i = 0; i < iods->num_tracks; i++)
    {
    iods->tracks[i].ES_ID_IncTag = quicktime_read_char(file);
    iods->tracks[i].length       = quicktime_read_mp4_descr_length(file);
    iods->tracks[i].track_id     = quicktime_read_int32(file);
    }
  
  /* will skip the remainder of the atom */
  }

void quicktime_write_iods(quicktime_t *file, quicktime_moov_t * moov)
  {
  quicktime_atom_t atom;
  int i;
  quicktime_iods_t * iods = &moov->iods;
  
  quicktime_atom_write_header(file, &atom, "iods");

  quicktime_write_char(file, iods->version);
  quicktime_write_int24(file, iods->flags);

  quicktime_write_char(file, 0x10);       /* MP4_IOD_Tag */
  quicktime_write_mp4_descr_length(file,
                                   7 + (iods->num_tracks * (1+1+4)), 0);    /* length */
  quicktime_write_int16(file, 0x004F); /* ObjectDescriptorID = 1 */
  quicktime_write_char(file, iods->ODProfileLevel);       /* ODProfileLevel */
  quicktime_write_char(file, iods->sceneProfileLevel);       /* sceneProfileLevel */
  quicktime_write_char(file, iods->audioProfileId);       /* audioProfileLevel */
  quicktime_write_char(file, iods->videoProfileId);       /* videoProfileLevel */
  quicktime_write_char(file, iods->graphicsProfileLevel);       /* graphicsProfileLevel */
  
  for (i = 0; i < iods->num_tracks; i++)
    {
    quicktime_write_char(file, iods->tracks[i].ES_ID_IncTag);       /* ES_ID_IncTag */
    quicktime_write_char(file, iods->tracks[i].length);       /* length */
    quicktime_write_int32(file, iods->tracks[i].track_id);
    }
 
  /* no OCI_Descriptors */
  /* no IPMP_DescriptorPointers */
  /* no Extenstion_Descriptors */
  quicktime_atom_write_footer(file, &atom);
  }

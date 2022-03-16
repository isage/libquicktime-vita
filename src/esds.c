/*******************************************************************************
 esds.c

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

int quicktime_read_mp4_descr_length(quicktime_t * file)
  {
  uint8_t b;
  int num_bytes = 0;
  unsigned int length = 0;

  do{
    if(!quicktime_read_data(file, &b, 1))
      return -1;
    num_bytes++;
    length = (length << 7) | (b & 0x7F);
    } while ((b & 0x80) && (num_bytes < 4));

  
  return length;
  }

void quicktime_read_esds(quicktime_t * file, quicktime_esds_t * esds)
  {
  uint8_t tag;
  esds->version = quicktime_read_char(file);
  esds->flags = quicktime_read_int24(file);

  quicktime_read_data(file, &tag, 1);

  if(tag == 0x03)
    {
    if(quicktime_read_mp4_descr_length(file) < 20)
      return;
// elementary stream id
    esds->esid = quicktime_read_int16(file);
// stream priority
    esds->stream_priority = quicktime_read_char(file);
    }
  else
    quicktime_read_int16(file); /* Skip 2 bytes */

  quicktime_read_data(file, &tag, 1);
  
  if(tag != 0x04)
    return;

  if(quicktime_read_mp4_descr_length(file) < 15)
    return;

  quicktime_read_data(file, &esds->objectTypeId, 1);
  quicktime_read_data(file, &esds->streamType, 1);
  esds->bufferSizeDB = quicktime_read_int24(file);
  esds->maxBitrate = quicktime_read_int32(file);
  esds->avgBitrate = quicktime_read_int32(file);
  
  quicktime_read_data(file, &tag, 1);
  
  if(tag != 0x05)
    return;

  esds->decoderConfigLen = quicktime_read_mp4_descr_length(file);

  /* Need some padding for ffmpeg */
  esds->decoderConfig = calloc(esds->decoderConfigLen+16, 1);
  quicktime_read_data(file, esds->decoderConfig, esds->decoderConfigLen);
  
  }

int quicktime_write_mp4_descr_length(quicktime_t *file,
                                     int length, int compact)
  {
  uint8_t b;
  int i;
  int numBytes;
  
  if (compact)
    {
    if (length <= 0x7F)
      {
      numBytes = 1;
      }
    else if(length <= 0x3FFF)
      {
      numBytes = 2;
      }
    else if(length <= 0x1FFFFF)
      {
      numBytes = 3;
      }
    else
      {
      numBytes = 4;
      }
    }
  else
    {
    numBytes = 4;
    }

  for (i = numBytes-1; i >= 0; i--)
    {
    b = (length >> (i * 7)) & 0x7F;
    if (i != 0)
      {
      b |= 0x80;
      }
    quicktime_write_char(file, b);
    }
  
  return numBytes; 
  }
#if 1

void quicktime_write_esds(quicktime_t * file, quicktime_esds_t * esds)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "esds");

  quicktime_write_char(file, 0);  /* Version */
  quicktime_write_int24(file, 0); /* Flags   */

  quicktime_write_char(file, 0x03);	/* ES_DescrTag */

  quicktime_write_mp4_descr_length(file, 
        3 + (5 + (13 + (8 + esds->decoderConfigLen))) + 3, 0);

  quicktime_write_int16(file, esds->esid);
  quicktime_write_char(file, esds->stream_priority);

  /* DecoderConfigDescriptor */
  quicktime_write_char(file, 0x04);	/* DecoderConfigDescrTag */
  
  quicktime_write_mp4_descr_length(file, 
                  10 + (8 + esds->decoderConfigLen), 0);

  quicktime_write_char(file, esds->objectTypeId); /* objectTypeIndication */
  quicktime_write_char(file, esds->streamType);   /* streamType */

  quicktime_write_int24(file, esds->bufferSizeDB); /* buffer size */
  quicktime_write_int32(file, esds->maxBitrate);   /* max bitrate */
  quicktime_write_int32(file, esds->avgBitrate);   /* average bitrate */

  quicktime_write_char(file, 0x05);	/* DecSpecificInfoTag */
  quicktime_write_mp4_descr_length(file, esds->decoderConfigLen, 0);
  quicktime_write_data(file, esds->decoderConfig, esds->decoderConfigLen);

  /* SLConfigDescriptor */
  quicktime_write_char(file, 0x06);	/* SLConfigDescrTag */
  quicktime_write_mp4_descr_length(file, 1, 0);
  quicktime_write_char(file, 0x02);	/* constant in mp4 files */
  
  quicktime_atom_write_footer(file, &atom);
  }

#else

static unsigned int descrLength(unsigned int len)
{
    int i;
    for(i=1; len>>(7*i); i++);
    return len + 1 + i;
}

static void putDescr(quicktime_t * file, int tag, unsigned int size)
  {
  int i= descrLength(size) - size - 2;
  quicktime_write_char(file, tag);
  for(; i>0; i--)
    quicktime_write_char(file, (size>>(7*i)) | 0x80);
  quicktime_write_char(file, size & 0x7F);
  }

void quicktime_write_esds(quicktime_t * file, quicktime_esds_t * esds)
  {
  quicktime_atom_t atom;
  int decoderSpecificInfoLen =
    esds->decoderConfigLen ? descrLength(esds->decoderConfigLen):0;
  quicktime_atom_write_header(file, &atom, "esds");

  quicktime_write_char(file, 0);  /* Version */
  quicktime_write_int24(file, 0); /* Flags   */

  // ES descriptor
  putDescr(file, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) +
           descrLength(1));

  quicktime_write_int16(file, esds->esid);
  quicktime_write_char(file, esds->stream_priority);
  // DecoderConfig descriptor
  putDescr(file, 0x04, 13 + esds->decoderConfigLen);
  // Object type indication
  quicktime_write_char(file, esds->objectTypeId); /* objectTypeIndication */
  quicktime_write_char(file, esds->streamType);   /* streamType */

  quicktime_write_int24(file, esds->bufferSizeDB); /* buffer size */
  quicktime_write_int32(file, esds->maxBitrate);   /* max bitrate */
  quicktime_write_int32(file, esds->avgBitrate);   /* average bitrate */

  // DecoderSpecific info descriptor
  if(decoderSpecificInfoLen)
    {
    putDescr(file, 0x05, esds->decoderConfigLen);
    quicktime_write_data(file, esds->decoderConfig, esds->decoderConfigLen);
    }
  // SL descriptor
  putDescr(file, 0x06, 1);
  quicktime_write_char(file, 0x02);

  quicktime_atom_write_footer(file, &atom);
  
  }

#endif

void quicktime_esds_dump(quicktime_esds_t * esds)
  {
  int i;
  lqt_dump("         esds: \n");
  lqt_dump("           Version:          %d\n", esds->version);
  lqt_dump("           Flags:            0x%06lx\n", esds->flags);
  lqt_dump("           ES ID:            0x%04x\n", esds->esid);
  lqt_dump("           Priority:         0x%02x\n", esds->stream_priority);
  lqt_dump("           objectTypeId:     %d\n", esds->objectTypeId);
  lqt_dump("           streamType:       0x%02x\n", esds->streamType);
  lqt_dump("           bufferSizeDB:     %d\n", esds->bufferSizeDB);

  lqt_dump("           maxBitrate:       %d\n", esds->maxBitrate);
  lqt_dump("           avgBitrate:       %d\n", esds->avgBitrate);
  lqt_dump("           decoderConfigLen: %d\n", esds->decoderConfigLen);
  lqt_dump("           decoderConfig:");

  for(i = 0; i < esds->decoderConfigLen; i++)
    {
    if(!(i % 16))
      lqt_dump("\n           ");
    lqt_dump("%02x ", esds->decoderConfig[i]);
    }
  lqt_dump("\n");
  }

void quicktime_esds_delete(quicktime_esds_t * esds)
  {
  if(esds->decoderConfig) free(esds->decoderConfig);
  }

quicktime_esds_t * quicktime_set_esds(quicktime_trak_t * trak,
                                      const uint8_t * decoderConfig,
                                      int decoderConfigLen)
  {
  quicktime_esds_t * esds;
  trak->mdia.minf.stbl.stsd.table[0].has_esds = 1;
  esds = &trak->mdia.minf.stbl.stsd.table[0].esds;
  esds->decoderConfigLen = decoderConfigLen;
  esds->decoderConfig = malloc(esds->decoderConfigLen);
  memcpy(esds->decoderConfig, decoderConfig, esds->decoderConfigLen);
  return esds;
  }


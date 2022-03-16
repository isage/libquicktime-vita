/*******************************************************************************
 useratoms.c

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

uint8_t * quicktime_user_atoms_get_atom(quicktime_user_atoms_t * u,
                                        char * name, uint32_t * len)
  {
  int i;

  for(i = 0; i < u->num_atoms; i++)
    {
    if((u->atoms[i][4] == name[0]) &&
       (u->atoms[i][5] == name[1]) &&
       (u->atoms[i][6] == name[2]) &&
       (u->atoms[i][7] == name[3]))
      {
      *len =
        ((uint32_t)u->atoms[i][0] << 24) |
        ((uint32_t)u->atoms[i][1] << 16) |
        ((uint32_t)u->atoms[i][2] <<  8) |
        u->atoms[i][3];
      return u->atoms[i];
      }
    }
  return (uint8_t*)0;
  }

void quicktime_user_atoms_add_atom(quicktime_user_atoms_t * u,
                                   char * name, uint8_t * data,
                                   uint32_t len)
  {
  u->atoms =
    realloc(u->atoms, (u->num_atoms+1)*sizeof(*u->atoms));
  
  u->atoms[u->num_atoms] = malloc(len+8);
  
  u->atoms[u->num_atoms][0] = ((len+8) & 0xff000000) >> 24;
  u->atoms[u->num_atoms][1] = ((len+8) & 0x00ff0000) >> 16;
  u->atoms[u->num_atoms][2] = ((len+8) & 0x0000ff00) >>  8;
  u->atoms[u->num_atoms][3] = ((len+8) & 0x000000ff);

  u->atoms[u->num_atoms][4] = name[0];
  u->atoms[u->num_atoms][5] = name[1];
  u->atoms[u->num_atoms][6] = name[2];
  u->atoms[u->num_atoms][7] = name[3];

  memcpy(u->atoms[u->num_atoms]+8, data, len);
  u->num_atoms++;
  }

void quicktime_user_atoms_read_atom(quicktime_t * file,
                                    quicktime_user_atoms_t * u,
                                    quicktime_atom_t * leaf_atom)
  {
  u->atoms =
    realloc(u->atoms, (u->num_atoms+1)*sizeof(*u->atoms));
  u->atoms[u->num_atoms] = malloc(leaf_atom->size);

  u->atoms[u->num_atoms][0] = ((leaf_atom->size) & 0xff000000) >> 24;
  u->atoms[u->num_atoms][1] = ((leaf_atom->size) & 0x00ff0000) >> 16;
  u->atoms[u->num_atoms][2] = ((leaf_atom->size) & 0x0000ff00) >>  8;
  u->atoms[u->num_atoms][3] = ((leaf_atom->size) & 0x000000ff);

  u->atoms[u->num_atoms][4] = leaf_atom->type[0];
  u->atoms[u->num_atoms][5] = leaf_atom->type[1];
  u->atoms[u->num_atoms][6] = leaf_atom->type[2];
  u->atoms[u->num_atoms][7] = leaf_atom->type[3];

  quicktime_read_data(file, u->atoms[u->num_atoms] + 8, leaf_atom->size - 8);

  u->num_atoms++;
  }

void quicktime_user_atoms_delete(quicktime_user_atoms_t * u)
  {
  int i;
  if(u->atoms)
    {
    for(i = 0; i < u->num_atoms; i++)
      {
      free(u->atoms[i]);
      }
    free(u->atoms);
    }
  }

static uint32_t read_be32(uint8_t const* p)
  {
  return ((uint32_t)p[0] << 24) + ((uint32_t)p[1] << 16) + ((uint32_t)p[2] << 8) + (uint32_t)p[3];
  }

static void quicktime_user_atoms_dump_ACLR(uint8_t const* atom, uint32_t len)
  {
  if (len >= 12)
    lqt_dump("           Tag: %.4s\n", atom + 8);
  if (len >= 16)
    lqt_dump("           Version: %.4s\n", atom + 12);
  if (len >= 20)
    {
    uint32_t yuv_range = read_be32(atom + 16);
    if (yuv_range == 1)
      lqt_dump("           YUV range: full -> [0, 255]\n");
    else if (yuv_range == 2)
      lqt_dump("           YUV range: normal -> [16, 235] for Y, [16, 240] for U and V\n");
    else
      lqt_dump("           YUV range: unknown (%u)\n", (unsigned)yuv_range);
    }
  if (len >= 24)
    lqt_dump("           Unknown: %u\n", (unsigned)read_be32(atom + 20));
  if (len > 24)
    lqt_dump("           %u more bytes follow\n", (unsigned)(len - 24));
  }

static void quicktime_user_atoms_dump_APRG(uint8_t const* atom, uint32_t len)
  {
  if (len >= 12)
    lqt_dump("           Tag: %.4s\n", atom + 8);
  if (len >= 16)
    lqt_dump("           Version: %.4s\n", atom + 12);
  if (len >= 20)
    lqt_dump("           Unknown #1: %u\n", (unsigned)read_be32(atom + 16));
  if (len >= 24)
    lqt_dump("           Unknown #2: %u\n", (unsigned)read_be32(atom + 20));
  if (len > 24)
    lqt_dump("           %u more bytes follow\n", (unsigned)(len - 24));
  }

static void quicktime_user_atoms_dump_ARES(uint8_t const* atom, uint32_t len)
  {
  if (len >= 12)
    lqt_dump("           Tag: %.4s\n", atom + 8);
  if (len >= 16)
    lqt_dump("           Version: %.4s\n", atom + 12);
  if (len >= 20)
    lqt_dump("           CID: %u\n", (unsigned)read_be32(atom + 16));
  if (len >= 24)
    lqt_dump("           Frame/field width: %u\n", (unsigned)read_be32(atom + 20));
  if (len >= 28)
    lqt_dump("           Frame/field height: %u\n", (unsigned)read_be32(atom + 24));
  if (len >= 32)
    lqt_dump("           Unknown #1: %u\n", (unsigned)read_be32(atom + 28));
  if (len >= 36)
    lqt_dump("           Unknown #2: %u\n", (unsigned)read_be32(atom + 32));
  if (len >= 40)
    lqt_dump("           Unknown #3: %u\n", (unsigned)read_be32(atom + 36));
  if (len > 40)
    lqt_dump("           %u more bytes follow\n", (unsigned)(len - 40));
  }

void quicktime_user_atoms_dump(quicktime_user_atoms_t * u)
  {
  int i;
  uint32_t len;

  for(i = 0; i < u->num_atoms; i++)
    {
    len =
      ((uint32_t)u->atoms[i][0] << 24) |
      ((uint32_t)u->atoms[i][1] << 16) |
      ((uint32_t)u->atoms[i][2] <<  8) |
    u->atoms[i][3];
    lqt_dump("         User atom %.4s (%d bytes)\n", u->atoms[i] + 4, len);
    if (strncmp((char const*)u->atoms[i] + 4, "ACLR", 4) == 0)
      quicktime_user_atoms_dump_ACLR(u->atoms[i], len);
    else if (strncmp((char const*)u->atoms[i] + 4, "APRG", 4) == 0)
      quicktime_user_atoms_dump_APRG(u->atoms[i], len);
    else if (strncmp((char const*)u->atoms[i] + 4, "ARES", 4) == 0)
      quicktime_user_atoms_dump_ARES(u->atoms[i], len);
    }
  }


void quicktime_write_user_atoms(quicktime_t * file,
                                quicktime_user_atoms_t * u)
  {
  int i;
  uint32_t len;

  for(i = 0; i < u->num_atoms; i++)
    {
    len =
      ((uint32_t)u->atoms[i][0] << 24) |
      ((uint32_t)u->atoms[i][1] << 16) |
      ((uint32_t)u->atoms[i][2] <<  8) |
      u->atoms[i][3];
    
    quicktime_write_data(file, u->atoms[i], len);

    if (strncmp((char const*)u->atoms[i] + 4, "ARES", 4) == 0)
      {
      /* This padding keeps the Windows QuickTime player from crashing.
         FFMpeg inserts this kind of padding as well in mov_write_avid_tag(),
         although they don't specify the reason. */
      static uint8_t const zero_padding[] = { 0, 0, 0, 0 };
      quicktime_write_data(file, zero_padding, 4);
      }
    }
  
  }

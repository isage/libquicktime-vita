/*******************************************************************************
 wave.c

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

uint8_t * quicktime_wave_get_user_atom(quicktime_trak_t * trak, char * name, uint32_t * len)
  {
  quicktime_wave_t * wave = &trak->mdia.minf.stbl.stsd.table[0].wave;
  return(quicktime_user_atoms_get_atom(&wave->user_atoms, name, len));
  }


void quicktime_wave_set_user_atom(quicktime_trak_t * trak, char * name,
                                  uint8_t * data, uint32_t len)
  {
  quicktime_wave_t * wave = &trak->mdia.minf.stbl.stsd.table[0].wave;

  quicktime_user_atoms_add_atom(&wave->user_atoms,
                                name, data, len);
  trak->mdia.minf.stbl.stsd.table[0].has_wave = 1;
  }
  
void quicktime_wave_delete(quicktime_wave_t *wave)
  {
  quicktime_user_atoms_delete(&wave->user_atoms);
  quicktime_esds_delete(&wave->esds);
  }

void quicktime_read_wave(quicktime_t *file, quicktime_wave_t *wave,
                         quicktime_atom_t *wave_atom)
  {
  quicktime_atom_t leaf_atom;
  do{
    quicktime_atom_read_header(file, &leaf_atom);
    
    if(quicktime_atom_is(&leaf_atom, "frma"))
      {
      quicktime_read_frma(file, &wave->frma, &leaf_atom);
      wave->has_frma = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "enda"))
      {
      quicktime_read_enda(file, &wave->enda, &leaf_atom);
      wave->has_enda = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "esds"))
      {
      quicktime_read_esds(file, &wave->esds);
      wave->has_esds = 1;
      quicktime_atom_skip(file, &leaf_atom);
      }
    else if(quicktime_atom_is(&leaf_atom, (char[]){ 0x00, 0x00, 0x00, 0x00 }))
      {
      break;
      }
    else /* Add to user atoms */
      {
      quicktime_user_atoms_read_atom(file,
                                     &wave->user_atoms,
                                     &leaf_atom);
      }
    quicktime_atom_skip(file, &leaf_atom);
  }while(quicktime_position(file) < wave_atom->end);
  
  }

void quicktime_write_wave(quicktime_t *file, quicktime_wave_t *wave)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "wave");

  if(wave->has_frma)
    quicktime_write_frma(file, &wave->frma);
  if(wave->has_esds)
    quicktime_write_esds(file, &wave->esds);
  if(wave->has_enda)
    quicktime_write_enda(file, &wave->enda);

  quicktime_write_user_atoms(file, &wave->user_atoms);
  
  quicktime_atom_write_footer(file, &atom);
  }

void quicktime_wave_dump(quicktime_wave_t *wave)
  {
  lqt_dump("       wave: \n");
  if(wave->has_frma)
    quicktime_frma_dump(&wave->frma);
  if(wave->has_enda)
    quicktime_enda_dump(&wave->enda);
  if(wave->has_esds)
    quicktime_esds_dump(&wave->esds);

  quicktime_user_atoms_dump(&wave->user_atoms);
  
  }

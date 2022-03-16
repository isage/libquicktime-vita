/*******************************************************************************
 tref.c

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


int quicktime_tref_init(quicktime_tref_t *tref)
{
//	tref->trackIndex = 1;
	return 0;
}

static quicktime_track_reference_t * add_reference(quicktime_tref_t *tref)
  {
  tref->num_references++;
  tref->references = realloc(tref->references,
                             tref->num_references * sizeof(*(tref->references)));
  memset(&tref->references[tref->num_references-1],
         0, sizeof(tref->references[tref->num_references-1]));
  return &tref->references[tref->num_references-1];
  }

static void add_track(quicktime_track_reference_t * ref,
                      int track_id)
  {
  ref->num_tracks++;
  ref->tracks = realloc(ref->tracks,
                        ref->num_tracks * sizeof(*ref->tracks));
  ref->tracks[ref->num_tracks-1] = track_id;
  }

int quicktime_tref_init_qtvr(quicktime_tref_t *tref, int track_type)
  {
  quicktime_track_reference_t * ref = add_reference(tref);
  switch(track_type)
	{
	case QTVR_OBJ:
	case QTVR_PAN:
	  ref->type[0] = 'i';
	  ref->type[1] = 'm';
	  ref->type[2] = 'g';
	  ref->type[3] = 't';
	  break;
	case QTVR_QTVR_OBJ:
	  ref->type[0] = 'o';
	  ref->type[1] = 'b';
	  ref->type[2] = 'j';
	  ref->type[3] = 'e';
	  break;
	case QTVR_QTVR_PAN:
	  ref->type[0] = 'p';
	  ref->type[1] = 'a';
	  ref->type[2] = 'n';
	  ref->type[3] = 'o';
	  break;
    default:
//	  lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
//	          "quicktime_tref_init_qtvr invalid track type supplied");
	  return -1;
    }
  
  add_track(ref, 1);
  return 0;
  }

int quicktime_tref_init_chap(quicktime_tref_t * tref, int track_id)
  {
  quicktime_track_reference_t * ref = add_reference(tref);
  ref->type[0] = 'c';
  ref->type[1] = 'h';
  ref->type[2] = 'a';
  ref->type[3] = 'p';
  add_track(ref, track_id);
  return 0;
  }

int quicktime_tref_init_tmcd(quicktime_tref_t * tref, int track_id)
  {
  quicktime_track_reference_t * ref = add_reference(tref);
  ref->type[0] = 't';
  ref->type[1] = 'm';
  ref->type[2] = 'c';
  ref->type[3] = 'd';
  add_track(ref, track_id);
  return 0;
  }



int quicktime_tref_delete(quicktime_tref_t *tref)
  {
  int i;
  for(i = 0; i < tref->num_references; i++)
    {
    if(tref->references[i].tracks)
      free(tref->references[i].tracks);
    }
  if(tref->references) free(tref->references);
  return 0;
  }

void quicktime_tref_dump(quicktime_tref_t *tref)
  {
  int i, j;
  quicktime_track_reference_t * ref;
  
  lqt_dump("  track reference (tref)\n");
  for(i = 0; i < tref->num_references; i++)
    {
    ref = &tref->references[i];
    lqt_dump("     reference type %d type: %c%c%c%c\n",
             i+1,
             ref->type[0], ref->type[1],
             ref->type[2], ref->type[3]);
    lqt_dump("     track indices: %d\n", ref->num_tracks);
    for(j = 0; j < ref->num_tracks; j++)
      {
      lqt_dump("       track_index %d: %d\n", j, ref->tracks[j]);
      }
    
    }
  }

int quicktime_read_tref(quicktime_t *file, quicktime_tref_t *tref,
                        quicktime_atom_t *tref_atom)
  {
  int i;
  quicktime_atom_t atom;
  quicktime_track_reference_t * ref;
  while(quicktime_position(file) < tref_atom->end)
    {
    ref = add_reference(tref);
          
    quicktime_atom_read_header(file, &atom);
    ref->type[0] = atom.type[0];
    ref->type[1] = atom.type[1];
    ref->type[2] = atom.type[2];
    ref->type[3] = atom.type[3];

    ref->num_tracks = (atom.end - quicktime_position(file)) / 4;
    ref->tracks = calloc(ref->num_tracks, sizeof(*ref->tracks));

    for(i = 0; i < ref->num_tracks; i++)
      ref->tracks[i] = quicktime_read_int32(file);
    }
  return 0;
  }

void quicktime_write_tref(quicktime_t *file, quicktime_tref_t *tref )
  {
  int i, j;
  quicktime_atom_t atom, subatom;
  quicktime_atom_write_header(file, &atom, "tref");

  for(i = 0; i < tref->num_references; i++)
    {
    quicktime_atom_write_header(file, &subatom, tref->references[i].type);
    
    for(j = 0; j < tref->references[i].num_tracks; j++)
      quicktime_write_int32(file, tref->references[i].tracks[j]);
    
    quicktime_atom_write_footer(file, &subatom);
    }

  quicktime_atom_write_footer(file, &atom);
  }


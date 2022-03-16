/*******************************************************************************
 multichannel.c

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

/*

 Multichannel support for libquicktime.

*/

#include "lqt_private.h"
#include <stdlib.h>
#include <string.h>

static struct
  {
  lqt_channel_t ch;
  char * name;
  }
channels[] =
  {
    { LQT_CHANNEL_UNKNOWN,            "Unknown"     },
    { LQT_CHANNEL_FRONT_LEFT,         "Front Left"  },
    { LQT_CHANNEL_FRONT_RIGHT,        "Front Right" },
    { LQT_CHANNEL_FRONT_CENTER,       "Front Center" },
    { LQT_CHANNEL_FRONT_CENTER_LEFT,  "Front Center Left" },
    { LQT_CHANNEL_FRONT_CENTER_RIGHT, "Front Center Right" },
    { LQT_CHANNEL_BACK_LEFT,          "Back Left" },
    { LQT_CHANNEL_BACK_RIGHT,         "Back Right" },
    { LQT_CHANNEL_BACK_CENTER,        "Back Center" },
    { LQT_CHANNEL_SIDE_LEFT,          "Side Left" },
    { LQT_CHANNEL_SIDE_RIGHT,         "Side Right" },
#if 0
    { LQT_CHANNEL_TOP_FRONT_LEFT,     "Top Front Left" },
    { LQT_CHANNEL_TOP_FRONT_RIGHT,    "Top Front Right" },
    { LQT_CHANNEL_TOP_FRONT_CENTER,   "Top Front Center" },
    { LQT_CHANNEL_TOP_BACK_LEFT,      "Top Back Left" },
    { LQT_CHANNEL_TOP_BACK_RIGHT,     "Top Back Right" },
    { LQT_CHANNEL_TOP_BACK_CENTER,    "Top Back Center" },
#endif
    { LQT_CHANNEL_LFE,                "LFE" }
  };

const char * lqt_channel_to_string(lqt_channel_t ch)
  {
  int i;
  for(i = 0; i < sizeof(channels) / sizeof(channels[0]); i++)
    {
    if(channels[i].ch == ch)
      return channels[i].name;
    }
  return channels[0].name;
  }


void lqt_set_channel_setup(quicktime_t * file, int track, lqt_channel_t * ch)
  {
  if(!file->atracks[track].channel_setup)
    file->atracks[track].channel_setup = calloc(file->atracks[track].channels,
                                                sizeof(*file->atracks[track].channel_setup));
  
  memcpy(file->atracks[track].channel_setup, ch, sizeof(*ch)*file->atracks[track].channels);
  quicktime_set_chan(&file->atracks[track]);
  }

const lqt_channel_t * lqt_get_channel_setup(quicktime_t * file, int track)
  {
  if((track >= file->total_atracks) || (track < 0))
    return (lqt_channel_t*)0;
  return file->atracks[track].channel_setup;
  }

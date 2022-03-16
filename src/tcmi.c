/*******************************************************************************
 tcmi.c

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

void quicktime_tcmi_init(quicktime_tcmi_t *tcmi)
  {
  tcmi->version = 0;
  tcmi->flags = 1;
  tcmi->font = 0;
  tcmi->face = 0;
  tcmi->size = 12;
  tcmi->fontname[0] = 'S';
  tcmi->fontname[1] = 'y';
  tcmi->fontname[2] = 's';
  tcmi->fontname[3] = 't';
  tcmi->fontname[4] = 'e';
  tcmi->fontname[5] = 'm';
  tcmi->txtcolor[0] = 65535;
  tcmi->txtcolor[1] = 65535;
  tcmi->txtcolor[2] = 65535;
  tcmi->bgcolor[0] = 0;
  tcmi->bgcolor[1] = 0;
  tcmi->bgcolor[2] = 0;
  }

void quicktime_tcmi_delete(quicktime_tcmi_t *tcmi)
  {
  }

void quicktime_tcmi_dump(quicktime_tcmi_t *tcmi)
  {
  lqt_dump("         Timecode media info (tcmi)\n");
  lqt_dump("          version  %d\n", tcmi->version);
  lqt_dump("          flags    %ld\n", tcmi->flags);
  lqt_dump("          font     %d\n", tcmi->font);
  lqt_dump("          face     %d\n", tcmi->face);
  lqt_dump("          size     %d\n", tcmi->size);
  lqt_dump("          txtcolor %d %d %d\n", tcmi->txtcolor[0], tcmi->txtcolor[1], tcmi->txtcolor[2]);
  lqt_dump("          bgcolor  %d %d %d\n", tcmi->bgcolor[0], tcmi->bgcolor[1], tcmi->bgcolor[2]);
  lqt_dump("          fontname %s\n", tcmi->fontname);
  }

void quicktime_read_tcmi(quicktime_t *file, quicktime_tcmi_t *tcmi)
  {
  int i;
  tcmi->version = quicktime_read_char(file);
  tcmi->flags = quicktime_read_int24(file);
  tcmi->font = quicktime_read_int16(file);
  tcmi->face = quicktime_read_int16(file);
  tcmi->size = quicktime_read_int16(file);
  quicktime_read_int16(file);
  for (i = 0; i < 3; ++i)
    tcmi->txtcolor[i] = quicktime_read_int16(file);
  for (i = 0; i < 3; ++i)
    tcmi->bgcolor[i] = quicktime_read_int16(file);
  quicktime_read_pascal(file, tcmi->fontname);

  

  }

void quicktime_write_tcmi(quicktime_t *file, quicktime_tcmi_t *tcmi)
  {
  quicktime_atom_t atom;
  int i;

  quicktime_atom_write_header(file, &atom, "tcmi");
  quicktime_write_char(file, tcmi->version);
  quicktime_write_int24(file, tcmi->flags);
  quicktime_write_int16(file, tcmi->font);
  quicktime_write_int16(file, tcmi->face);
  quicktime_write_int16(file, tcmi->size);
  quicktime_write_int16(file, 0);
  for (i = 0; i < 3; ++i)
    quicktime_write_int16(file, tcmi->txtcolor[i]);
  for (i = 0; i < 3; ++i)
    quicktime_write_int16(file, tcmi->bgcolor[i]);
  quicktime_write_pascal(file, tcmi->fontname);
  quicktime_atom_write_footer(file, &atom);
  }

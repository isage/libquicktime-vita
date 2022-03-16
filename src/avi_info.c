/*******************************************************************************
 avi_info.c

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
#include "charset.h"
#include <stdlib.h>
#include <string.h>

static char * my_strdup(const char * str)
  {
  char * ret = malloc(strlen(str)+1);
  strcpy(ret, str);
  return ret;
  }

/* RS == read_string */

#define RS(tag) \
  if(!strncmp((char*)ptr, #tag, 4))             \
    { \
    info->tag = my_strdup((char*)(ptr + 8));    \
    lqt_charset_convert(cnv, &info->tag, -1, (int*)0); \
    ptr += string_len + 8; \
    if(string_len % 2) \
      ptr++; \
    continue; \
    }

void quicktime_read_riffinfo(quicktime_t *file,
                             quicktime_riffinfo_t *info,
                             quicktime_atom_t *parent_atom)
  {
  uint32_t string_len;
  uint8_t *buf, * ptr, *end_ptr;
  lqt_charset_converter_t * cnv;
  
  int size = parent_atom->end - quicktime_position(file);
  buf = malloc(size);
  ptr = buf;
  
  quicktime_read_data(file, ptr, size);
  
  end_ptr = ptr + size;

  cnv = lqt_charset_converter_create(file, "ISO-8859-1", "UTF-8");
  
  while(ptr < end_ptr)
    {
    string_len = ((uint32_t)ptr[4]) |
      (((uint32_t)ptr[5]) << 8) |
      (((uint32_t)ptr[6]) << 16) |
      (((uint32_t)ptr[7]) << 24);
    
    RS(IARL);
    RS(IART);
    RS(ICMS);
    RS(ICMT);
    RS(ICOP);
    RS(ICRD);
    RS(ICRP);
    RS(IDIM);
    RS(IDPI);
    RS(IENG);
    RS(IGNR);
    RS(IKEY);
    RS(ILGT);
    RS(IMED);
    RS(INAM);
    RS(IPLT);
    RS(IPRD);
    RS(ISBJ);
    RS(ISFT);
    RS(ISHP);
    RS(ISRC);
    RS(ISRF);
    RS(ITCH);

    ptr += string_len + 8;
    if(string_len % 2)
      ptr++;
    }
  free(buf);
  lqt_charset_converter_destroy(cnv);
  }

#undef RS

void quicktime_init_riffinfo(quicktime_riffinfo_t *info)
  {
  info->ISFT = my_strdup(PACKAGE"-"VERSION);
  }

/* WS == write_string */

#define WS(tag) \
  if(info->tag)             \
    { \
    lqt_charset_convert(cnv, &info->tag, -1, (int*)0); \
    quicktime_atom_write_header(file, &child_atom, #tag);\
    /* The trailing '\0' must be written to the file! */\
    quicktime_write_data(file, (uint8_t*)info->tag, strlen(info->tag)+1); \
    quicktime_atom_write_footer(file, &child_atom);\
    }

void quicktime_write_riffinfo(quicktime_t *file,
                              quicktime_riffinfo_t *info)
  {
  quicktime_atom_t atom;
  quicktime_atom_t child_atom;
  lqt_charset_converter_t * cnv;
  
  cnv = lqt_charset_converter_create(file, "UTF-8", "ISO-8859-1");

  quicktime_atom_write_header(file, &atom, "LIST");
  quicktime_write_char32(file, "INFO");
  
  WS(IARL);
  WS(IART);
  WS(ICMS);
  WS(ICMT);
  WS(ICOP);
  WS(ICRD);
  WS(ICRP);
  WS(IDIM);
  WS(IDPI);
  WS(IENG);
  WS(IGNR);
  WS(IKEY);
  WS(ILGT);
  WS(IMED);
  WS(INAM);
  WS(IPLT);
  WS(IPRD);
  WS(ISBJ);
  WS(ISFT);
  WS(ISHP);
  WS(ISRC);
  WS(ISRF);
  WS(ITCH);
  quicktime_atom_write_footer(file, &atom);

  lqt_charset_converter_destroy(cnv);
  }

#define CP_STR_INFO_2_UDTA(src, dst, dst_len) \
  if(info->src) \
    { \
    udta->dst = my_strdup(info->src); \
    udta->dst_len = strlen(info->src); \
    }

void quicktime_riffinfo_2_udta(quicktime_riffinfo_t * info,
                               quicktime_udta_t * udta)
  {
  /* Artist */
  CP_STR_INFO_2_UDTA(IART, artist, artist_len);
  /* Name */
  CP_STR_INFO_2_UDTA(INAM, name, name_len);
  /* Comment */
  CP_STR_INFO_2_UDTA(ICMT, comment, comment_len);
  /* Copyright */
  CP_STR_INFO_2_UDTA(ICOP, copyright, copyright_len);
  /* genre */
  CP_STR_INFO_2_UDTA(IGNR, genre, genre_len);
  }

#undef CP_STR_INFO_2_UDTA

#define CP_STR_UDTA_2_INFO(dst, src) \
  if(udta->src) \
    { \
    info->dst = my_strdup(udta->src); \
    }


void quicktime_udta_2_riffinfo(quicktime_udta_t * udta,
                               quicktime_riffinfo_t * info)
  {
  /* Artist */
  CP_STR_UDTA_2_INFO(IART, artist);
  /* Name */
  CP_STR_UDTA_2_INFO(INAM, name);
  /* Comment */
  CP_STR_UDTA_2_INFO(ICMT, comment);
  /* Copyright */
  CP_STR_UDTA_2_INFO(ICOP, copyright);
  /* genre */
  CP_STR_UDTA_2_INFO(IGNR, genre);
  }

/* FS = free_string */

#define FS(tag) if(info->tag) { free(info->tag); info->tag = (char*)0; }

void quicktime_delete_riffinfo(quicktime_riffinfo_t * info)
  {
  FS(IARL);
  FS(IART);
  FS(ICMS);
  FS(ICMT);
  FS(ICOP);
  FS(ICRD);
  FS(ICRP);
  FS(IDIM);
  FS(IDPI);
  FS(IENG);
  FS(IGNR);
  FS(IKEY);
  FS(ILGT);
  FS(IMED);
  FS(INAM);
  FS(IPLT);
  FS(IPRD);
  FS(ISBJ);
  FS(ISFT);
  FS(ISHP);
  FS(ISRC);
  FS(ISRF);
  FS(ITCH);
  }

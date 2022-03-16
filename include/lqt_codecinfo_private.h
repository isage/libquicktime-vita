/*******************************************************************************
 lqt_codecinfo_private.h

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
 *   Private part of the codec database
 *   It contains interfaces, which are only used
 *   by the codec manager itself and by plugins
 */

/*
 *  Codec database
 */

extern int lqt_num_audio_codecs;
extern int lqt_num_video_codecs;

extern lqt_codec_info_t * lqt_audio_codecs;
extern lqt_codec_info_t * lqt_video_codecs;

/*
 *  Read codec file and return single zero terminated list of all
 *  contained cocecs.
 *  audio_order and video_order retrurn sort strings.
 */

lqt_codec_info_t * lqt_registry_read(char ** audio_order, char ** video_order);

void lqt_register_audio_codec(lqt_codec_info_t * info);
  
void lqt_register_video_codec(lqt_codec_info_t * info);


/* Convert chars to an integer fourcc */

#if 0
#define LQT_CHAR_2_FOURCC( ch0, ch1, ch2, ch3 ) \
  ( (uint32_t)(unsigned char)(ch3) | \
  ( (uint32_t)(unsigned char)(ch2) << 8 ) | \
  ( (uint32_t)(unsigned char)(ch1) << 16 ) | \
  ( (uint32_t)(unsigned char)(ch0) << 24 ) )

/* Convert an integer fourcc to a string */
#endif

#define LQT_FOURCC_2_STRING( str , fourcc ) \
str[0] = (fourcc & 0xFF000000) >> 24; \
str[1] = (fourcc & 0xFF0000) >> 16; \
str[2] = (fourcc & 0xFF00) >> 8; \
str[3] = (fourcc & 0xFF); \
str[4] = fourcc & 0x00;

/* Convert a string to an integer fourcc */

#define LQT_STRING_2_FOURCC( str ) \
  ( ( (uint32_t)(unsigned char)(str[0]) << 24 ) | \
    ( (uint32_t)(unsigned char)(str[1]) << 16 ) | \
    ( (uint32_t)(unsigned char)(str[2]) << 8 ) | \
    ( (uint32_t)(unsigned char)(str[3]) ) )



/*
 *  (un)lock the registry
 */

void lqt_registry_lock();
void lqt_registry_unlock();

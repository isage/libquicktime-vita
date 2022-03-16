/*******************************************************************************
 videocodec.h

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

#ifndef QUICKTIME_VIDEOCODEC_H
#define QUICKTIME_VIDEOCODEC_H

#include <quicktime/quicktime.h>

void lqt_set_fiel_uncompressed(quicktime_t * file, int track);

/*
 * Sets the colr atom of a track to defaults:
 * Color Parameter type: 'nclc'
 * Primaries index : 1
 * Transfer Function index : 1
 * Matrix index: 1
 * According to http://developer.apple.com/mac/library/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html
 * the colr atom is always required when using uncompressed YUV formats.
 *
 * Return type is the same as lqt_set_colr (1 if the call was successful, 0 if there is no such track)
 */
int lqt_set_colr_yuv_uncompressed(quicktime_t *file, int track);

void quicktime_init_codec_raw(quicktime_codec_t * codec_base,
                              quicktime_audio_map_t *atrack,
                              quicktime_video_map_t *vtrack);
void quicktime_init_codec_rawalpha(quicktime_codec_t * codec_base,
                                   quicktime_audio_map_t *atrack,
                                   quicktime_video_map_t *vtrack);

void quicktime_init_codec_v210(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_v308(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_v408(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_v410(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_yuv2(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_2vuy(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_yuvs(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_yuv4(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);
void quicktime_init_codec_yv12(quicktime_codec_t * codec_base,
                               quicktime_audio_map_t *atrack,
                               quicktime_video_map_t *vtrack);

#endif

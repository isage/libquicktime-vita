/*******************************************************************************
 compression.h

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

#ifndef _LQT_COMPRESSION_H_
#define _LQT_COMPRESSION_H_

#pragma GCC visibility push(default)


typedef enum
  {
    LQT_COMPRESSION_NONE = 0,  // Undefined/unsupported

    /* Audio */
    LQT_COMPRESSION_ALAW      = 1,
    LQT_COMPRESSION_ULAW,
    LQT_COMPRESSION_MP2,
    LQT_COMPRESSION_MP3,
    LQT_COMPRESSION_AC3,
    LQT_COMPRESSION_AAC,

    /* Video */
    LQT_COMPRESSION_JPEG = 0x10000, //!< JPEG image
    LQT_COMPRESSION_PNG,            //!< PNG image
    LQT_COMPRESSION_TIFF,           //!< TIFF image
    LQT_COMPRESSION_TGA,            //!< TGA image
    LQT_COMPRESSION_MPEG4_ASP,      //!< MPEG-4 ASP (a.k.a. Divx4)
    LQT_COMPRESSION_H264,           //!< H.264 (Annex B)
    LQT_COMPRESSION_DIRAC,          //!< Complete DIRAC frames
    LQT_COMPRESSION_D10,            //!< D10 according to SMPTE 356M-2001
    LQT_COMPRESSION_DV,             //!< DV Variants

  } lqt_compression_id_t;

#define LQT_COMPRESSION_HAS_P_FRAMES (1<<0) //!< Not all frames are keyframes
#define LQT_COMPRESSION_HAS_B_FRAMES (1<<1) //!< Frames don't appear in presentation order 
#define LQT_COMPRESSION_SBR          (1<<2) //!< Samplerate got doubled by decoder, format and sample counts are for the upsampled rate

typedef struct
  {
  lqt_compression_id_t id;
  int flags;
  int global_header_len;
  uint8_t * global_header;

  int bitrate;
  
  /* Audio format */
  int samplerate;
  int num_channels;

  /* Video format */
  int width;
  int height;
  int pixel_width;
  int pixel_height;
  int colormodel;
  int video_timescale;
  
  } lqt_compression_info_t;


#define LQT_PACKET_KEYFRAME (1<<0) //!< Keyframes

typedef struct
  {
  int flags;

  int data_len;      //!< Length of data
  int data_alloc;    //!< Allocated size for data
  uint8_t * data;    //!< Data
  
  int header_size;   //!< Size of prepended global header (if any)

  int64_t timestamp; //!< Presentation time
  int duration;      //!< Duration of that packet when decompressed
  
  } lqt_packet_t;

/* Housekeeping */

const char * lqt_compression_id_to_string(lqt_compression_id_t id);
lqt_compression_id_t lqt_compression_id_from_string(const char * str);


void lqt_compression_info_free(lqt_compression_info_t * info);

void lqt_compression_info_set_header(lqt_compression_info_t * info,
                                     uint8_t * header,
                                     int header_len);


void lqt_compression_info_copy(lqt_compression_info_t * dst,
                               const lqt_compression_info_t * src);


void lqt_packet_alloc(lqt_packet_t * packet, int bytes);
void lqt_packet_free(lqt_packet_t * packet);

void lqt_compression_info_dump(const lqt_compression_info_t * ci);
void lqt_packet_dump(const lqt_packet_t * p);



/* Reading */

const lqt_compression_info_t * lqt_get_audio_compression_info(quicktime_t * file, int track);
const lqt_compression_info_t * lqt_get_video_compression_info(quicktime_t * file, int track);

int lqt_read_audio_packet(quicktime_t * file, lqt_packet_t * p, int track);
int lqt_read_video_packet(quicktime_t * file, lqt_packet_t * p, int track);

/* Writing */

int lqt_writes_compressed(lqt_file_type_t type,
                          const lqt_compression_info_t * ci,
                          lqt_codec_info_t * codec_info);

int lqt_add_audio_track_compressed(quicktime_t * file,
                                   const lqt_compression_info_t * ci,
                                   lqt_codec_info_t * codec_info);
int lqt_add_video_track_compressed(quicktime_t * file,
                                   const lqt_compression_info_t * ci,
                                   lqt_codec_info_t * codec_info);

int lqt_write_audio_packet(quicktime_t * file, lqt_packet_t * p, int track);
int lqt_write_video_packet(quicktime_t * file, lqt_packet_t * p, int track);

#pragma GCC visibility pop

#endif // _LQT_COMPRESSION_H_

/*******************************************************************************
 qtprivate.h

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

#ifndef PRIVATE_H
#define PRIVATE_H

#include "charset.h"
#include <quicktime/lqt_atoms.h>
#include <inttypes.h>
#include <stdio.h> // For quicktime_s->stream

/* ================================= structures */


/* Version used internally.  You need to query it with the C functions */
/* These must match quicktime4linux !!! */
#define QUICKTIME_MAJOR   2
#define QUICKTIME_MINOR   0
#define QUICKTIME_RELEASE 0

#define HEADER_LENGTH 8
#define MAXTRACKS 1024
#define MAXNODES 1

/* Crazy Mich R. Soft constants */
#define AVI_HASINDEX       0x00000010  // Index at end of file?
#define AVI_MUSTUSEINDEX   0x00000020
#define AVI_ISINTERLEAVED  0x00000100
#define AVI_TRUSTCKTYPE    0x00000800  // Use CKType to find key frames?
#define AVI_WASCAPTUREFILE 0x00010000
#define AVI_COPYRIGHTED    0x00020000
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVI_KEYFRAME       0x10
#define AVI_INDEX_OF_CHUNKS 0x01
#define AVI_INDEX_OF_INDEXES 0x00
                                                                                                                     
#define AVI_FRAME_RATE_BASE 10000
#define MAX_RIFFS  0x100


#define QTVR_GRABBER_SCROLLER_UI 1
#define QTVR_OLD_JOYSTICK_UI 2 
#define QTVR_JOYSTICK_UI 3
#define QTVR_GRABBER_UI 4
#define QTVR_ABSOLUTE_UI 5

/* Forward declarations */

typedef struct quicktime_strl_s quicktime_strl_t;

typedef struct quicktime_codec_s quicktime_codec_t;

typedef struct
  {
  /* for AVI it's the end of the 8 byte header in the file */
  /* for Quicktime it's the start of the 8 byte header in the file */
  int64_t start;
  int64_t end;        /* byte endpoint in file */
  int64_t size;       /* byte size for writing */
  int use_64;         /* Use 64 bit header */
  unsigned char type[4];
  } quicktime_atom_t;


typedef struct
  {
  int64_t start;
  int64_t end;        /* byte endpoint in file */
  int64_t size;       /* byte size for writing */
  unsigned char type[4];
  int child_count;
  int use_64;
  long ID;
  } quicktime_qtatom_t;

typedef struct
  {
  float values[9];
  } quicktime_matrix_t;

typedef struct
  {
  uint32_t major_brand;
  uint32_t minor_version;
  int num_compatible_brands;
  uint32_t * compatible_brands;
  } quicktime_ftyp_t;

typedef struct
  {
  int version;
  long flags;
  uint64_t creation_time;
  uint64_t modification_time;
  int track_id;
  long reserved1;
  uint64_t duration;
  uint8_t reserved2[8];
  int layer;
  int alternate_group;
  float volume;
  long reserved3;
  quicktime_matrix_t matrix;
  float track_width;
  float track_height;
  } quicktime_tkhd_t;


typedef struct
  {
  long seed;
  long flags;
  long size;
  unsigned short int *alpha;
  unsigned short int *red;
  unsigned short int *green;
  unsigned short int *blue;
  } quicktime_ctab_t;



/* ===================== sample table ======================== // */



/* sample description */

typedef struct
  {
  int motion_jpeg_quantization_table;
  } quicktime_mjqt_t;


typedef struct
  {
  int motion_jpeg_huffman_table;
  } quicktime_mjht_t;


typedef struct
  {
  uint16_t version;
  uint16_t revision;
  uint32_t imagingMode;
  uint32_t imagingValidFlags;
  uint32_t correction;
  uint32_t quality;
  uint32_t directdraw;
  uint32_t imagingProperties[6];
  uint32_t reserved1;
  uint32_t reserved2;
  } quicktime_impn_t;

typedef struct
  {
  quicktime_impn_t impn;
  } quicktime_imgp_t;

typedef struct
  {
  long reserved1;
  long reserved2;
  int version;
  int revision;
	
  long STrack; /* Prefix 'S' == Scene */
  long LowResSTrack;
  uint32_t reserved3[6];
  long HSTrack; /* Prefix 'HS' == HotSpot */
  uint32_t reserved4[9];
	
  float HPanStart;
  float HPanEnd;
  float VPanStart;
  float VPanEnd;
  float MinZoom;
  float MaxZoom;
	
  long SHeight;
  long SWidth;
  long NumFrames;
  int reserved5;
  int SNumFramesHeight;
  int SNumFramesWidth;
  int SDepth;
	
  long HSHeight;
  long HSWidth;
  int reserved6;
  int HSNumFramesHeight;
  int HSNumFramesWidth;
  int HSDepth;
  } quicktime_pano_t;

typedef struct
  {
  int version;
  int revision;
  char nodeType[4];
  long locationFlags;
  long locationData;
  long reserved1;
  long reserved2;
  } quicktime_nloc_t;

typedef struct
  {
  int version;
  int revision;
  char nodeType[4];
  long nodeID;
  long nameAtomID;
  long commentAtomID;
  long reserved1;
  long reserved2;
  } quicktime_ndhd_t;

typedef struct
  {
  quicktime_nloc_t nloc;
  int ID;
  } quicktime_vrni_t;

typedef struct
  {
  quicktime_vrni_t vrni[MAXNODES];
  int children;
  } quicktime_vrnp_t;

typedef struct
  {
  int version;
  int revision;
  long NameAtomID;
  long DefaultNodeID;
  long flags;
  long reserved1;
  long reserved2;
	
  } quicktime_vrsc_t;

typedef struct
  {
  quicktime_vrsc_t vrsc;
  quicktime_imgp_t imgp;
  quicktime_vrnp_t vrnp;
  } quicktime_qtvr_t;


/* MPEG-4 esds (elementary stream descriptor) */

typedef struct
  {
  int version;
  long flags;

  uint16_t esid;
  uint8_t stream_priority;
  
  uint8_t  objectTypeId;
  uint8_t  streamType;
  uint32_t bufferSizeDB;
  uint32_t maxBitrate;
  uint32_t avgBitrate;

  int      decoderConfigLen;
  uint8_t* decoderConfig;

  } quicktime_esds_t;

/* MPEG-4 iods */

typedef struct
  {
  int version;
  long flags;

  uint16_t ObjectDescriptorID;
  uint8_t  ODProfileLevel;
  uint8_t  sceneProfileLevel;
  uint8_t  audioProfileId;
  uint8_t  videoProfileId;
  uint8_t  graphicsProfileLevel;

  struct
    {
    uint8_t ES_ID_IncTag;
    uint8_t length;
    uint32_t track_id;
    } * tracks;
  int num_tracks;
  } quicktime_iods_t;

/* User atoms: These can be either inside a wave atom (for audio) or
   in the sample description (for video) */

typedef struct
  {
  int num_atoms;
  uint8_t ** atoms;
  } quicktime_user_atoms_t;

/* wave atom and subatoms */

typedef struct
  {
  char codec[4];
  /* Remainder could be a WAVEFORMATEX structure */
  } quicktime_frma_t;

typedef struct
  {
  int16_t littleEndian;
  } quicktime_enda_t;

typedef struct
  {
  quicktime_frma_t frma;
  int has_frma;
  quicktime_enda_t enda;
  int has_enda;

  quicktime_esds_t esds;
  int has_esds;
  
  quicktime_user_atoms_t user_atoms;
  } quicktime_wave_t;

typedef struct
  {
  int version;
  long flags;
  
  uint32_t mChannelLayoutTag;
  uint32_t mChannelBitmap;
  uint32_t mNumberChannelDescriptions;

  struct
    {
    uint32_t mChannelLabel;
    uint32_t mChannelFlags;
    float    mCoordinates[3];
    } * ChannelDescriptions;
  } quicktime_chan_t;

typedef struct
  {
  int fields;    /* 0, 1, or 2 */
  int dominance;   /* 0 - unknown     1 - top first     2 - bottom first */
  } quicktime_fiel_t;

typedef struct
  {
  float gamma;
  } quicktime_gama_t;

/* Font table for MPEG-4 timed text */

typedef struct
  {
  uint16_t num_fonts;

  struct
    {
    uint16_t font_id;
    char font_name[256];
    } * fonts;
  } quicktime_ftab_t;

/* Sample description for Quicktime text tracks */

typedef struct
  {
  uint32_t displayFlags;
  uint32_t textJustification;
  uint16_t bgColor[3];
  uint16_t defaultTextBox[4];
  uint32_t scrpStartChar;              /*starting character position*/
  uint16_t scrpHeight;
  uint16_t scrpAscent;
  uint16_t scrpFont;
  uint16_t scrpFace;
  uint16_t scrpSize;
  uint16_t scrpColor[3];
  char font_name[256];
  } quicktime_stsd_text_t;

/* Sample description for MPEG-4 text tracks */

typedef struct
  {
  uint32_t display_flags;
  uint8_t horizontal_justification;
  uint8_t vertical_justification;
  uint8_t back_color[4];
  uint16_t defaultTextBox[4];
  uint16_t start_char_offset;
  uint16_t end_char_offset;
  uint16_t font_id;
  uint8_t  style_flags;
  uint8_t  font_size;
  uint8_t  text_color[4];
  int has_ftab;
  quicktime_ftab_t ftab;
  } quicktime_stsd_tx3g_t;

/* Sample description for timecode tracks */

typedef struct
  {
  uint32_t reserved2;
  uint32_t flags;
  uint32_t timescale;
  uint32_t frameduration;
  uint8_t numframes;

  // http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap3/chapter_4_section_4.html#//apple_ref/doc/uid/TP40000939-CH205-BBCGABGG
  //  uint8_t reserved3[3];

  // Real life
  uint8_t reserved3;
  
  char * name;
  } quicktime_stsd_tmcd_t;

typedef struct
  {
  char format[4];
  uint8_t reserved[6];
  int data_reference;

  /* common to audio and video */
  int version;
  int revision;
  char vendor[4];

  /* video description */
  long temporal_quality;
  long spatial_quality;
  int width;
  int height;
  float dpi_horizontal;
  float dpi_vertical;
  int64_t data_size;
  int frames_per_sample;
  char compressor_name[32];
  int depth;
  int ctab_id;
  int has_ctab;
  quicktime_ctab_t ctab;

  quicktime_pasp_t pasp;
  int has_pasp;
  quicktime_colr_t colr;
  int has_colr;
  quicktime_clap_t clap;
  int has_clap;

  quicktime_fiel_t fiel;
  int has_fiel;

  quicktime_gama_t gama;
  int has_gama;

  quicktime_pano_t pano;
  quicktime_qtvr_t qtvr;

  int has_wave;
  quicktime_wave_t wave;
  /* audio description */
  uint32_t channels;
  uint32_t sample_size;
  /* Audio extension for version == 2 */
  uint32_t formatSpecificFlags;
  uint32_t constBytesPerAudioPacket;
  uint32_t constLPCMFramesPerAudioPacket;
  /* LQT: We have int16_t for the compression_id, because otherwise negative
     values don't show up correctly */
  int16_t compression_id;
  int packet_size;
  double samplerate;

  /* Audio extension for version == 1 */

  uint32_t audio_samples_per_packet;
  uint32_t audio_bytes_per_packet;
  uint32_t audio_bytes_per_frame;
  uint32_t audio_bytes_per_sample;


  quicktime_esds_t esds;
  int has_esds;

  quicktime_chan_t chan;
  int has_chan;

  quicktime_user_atoms_t user_atoms;

  /* LQT: We store the complete atom (starting with the fourcc)
     here, because this must be passed to the Sorenson 3 decoder */

  unsigned char * table_raw;
  int table_raw_size;

  /* Quicktime text */
  quicktime_stsd_text_t text;

  /* Quicktime tx3g */
  quicktime_stsd_tx3g_t tx3g;

  quicktime_stsd_tmcd_t tmcd;

  } quicktime_stsd_table_t;


typedef struct
  {
  int version;
  long flags;
  long total_entries;
  quicktime_stsd_table_t *table;
  } quicktime_stsd_t;


/* time to sample */
typedef struct
  {
  uint32_t sample_count;
  int32_t sample_duration;
  } quicktime_stts_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
  long entries_allocated;
  int  default_duration;
  quicktime_stts_table_t *table;
  } quicktime_stts_t;

/* Composition time to sample */

typedef struct
  {
  uint32_t sample_count;
  int32_t sample_duration;
  } quicktime_ctts_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
  long entries_allocated;
  quicktime_ctts_table_t *table;
  } quicktime_ctts_t;

/* sync sample */
typedef struct
  {
  long sample;
  } quicktime_stss_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
  long entries_allocated;
  quicktime_stss_table_t *table;
  } quicktime_stss_t;


/* sample to chunk */
typedef struct
  {
  long chunk;
  long samples;
  long id;
  } quicktime_stsc_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
	
  long entries_allocated;
  quicktime_stsc_table_t *table;
  } quicktime_stsc_t;


/* sample size */
typedef struct
  {
  int64_t size;
  } quicktime_stsz_table_t;

typedef struct
  {
  int version;
  long flags;
  int64_t sample_size;
  long total_entries;

  long entries_allocated;    /* used by the library for allocating a table */
  quicktime_stsz_table_t *table;
  } quicktime_stsz_t;


/* chunk offset */
typedef struct
  {
  int64_t offset;
  } quicktime_stco_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
	
  long entries_allocated;    /* used by the library for allocating a table */
  quicktime_stco_table_t *table;
  int co64;
  } quicktime_stco_t;


/* sample table */
typedef struct
  {
  int version;
  long flags;
  quicktime_stsd_t stsd;
  quicktime_stts_t stts;
  quicktime_stss_t stss;
  quicktime_stsc_t stsc;
  quicktime_stsz_t stsz;
  quicktime_stco_t stco;
  quicktime_ctts_t ctts;
  int has_ctts;
  } quicktime_stbl_t;

typedef struct
  {
  char type[4];
  int num_tracks;
  uint32_t * tracks;
  } quicktime_track_reference_t;

typedef struct
  {
  int num_references;
  quicktime_track_reference_t * references;
  } quicktime_tref_t;

/* data reference */

typedef struct
  {
  int64_t size;
  char type[4];
  int version;
  long flags;
  char *data_reference;
  } quicktime_dref_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;
  quicktime_dref_table_t *table;
  } quicktime_dref_t;

/* data information */

typedef struct
  {
  quicktime_dref_t dref;
  } quicktime_dinf_t;

/* video media header */

typedef struct
  {
  int version;
  long flags;
  int graphics_mode;
  int opcolor[3];
  } quicktime_vmhd_t;


/* sound media header */

typedef struct
  {
  int version;
  long flags;
  int balance;
  int reserved;
  } quicktime_smhd_t;


/* Base media info */

typedef struct
  {
  int version;
  long flags;
  int graphics_mode;
  int opcolor[3];
  int balance;
  int reserved;
  } quicktime_gmin_t;

/* Obscure text atom found inside the gmhd atom
 * of text tracks
 * TODO: Reverse engineer this
 */

typedef struct
  {
  uint32_t unk[9];
  } quicktime_gmhd_text_t;

typedef struct
  {
  int version;
  long flags;
  int font;
  int face;
  int size;
  int txtcolor[3];
  int bgcolor[3];
  char fontname[256];
  } quicktime_tcmi_t;

typedef struct
  {
  quicktime_tcmi_t tcmi;
  } quicktime_tmcd_t;

/* Base (generic) media header */

typedef struct
  {
  quicktime_gmin_t gmin;

  int has_gmhd_text;
  quicktime_gmhd_text_t gmhd_text;
  
  int has_tmcd;
  quicktime_tmcd_t tmcd;
  
  } quicktime_gmhd_t;

typedef struct
  {
  int version;
  long flags;
  } quicktime_nmhd_t;

/* handler reference */

typedef struct
  {
  int version;
  long flags;
  char component_type[4];
  char component_subtype[4];
  char component_manufacturer[4];
  long component_flags;
  long component_flag_mask;
  char component_name[256];
  } quicktime_hdlr_t;

/* media information */

typedef struct
  {
  int is_video;
  int is_audio;
  int is_audio_vbr;   /* Special flag indicating VBR audio */
  int is_panorama;
  int is_qtvr;
  int is_object;
  int is_text;
  int is_timecode;
  quicktime_vmhd_t vmhd;
  quicktime_smhd_t smhd;
  quicktime_gmhd_t gmhd;
  int has_gmhd;

  quicktime_nmhd_t nmhd;
  int has_nmhd;

  quicktime_stbl_t stbl;
  quicktime_hdlr_t hdlr;
  int has_hdlr;

  quicktime_dinf_t dinf;
  } quicktime_minf_t;


/* media header */

typedef struct
  {
  int version;
  long flags;
  unsigned long creation_time;
  unsigned long modification_time;
  long time_scale;
  long duration;
  int language;
  int quality;
  } quicktime_mdhd_t;


/* media */

typedef struct
  {
  quicktime_mdhd_t mdhd;
  quicktime_minf_t minf;
  quicktime_hdlr_t hdlr;
  } quicktime_mdia_t;

/* edit list */
typedef struct
  {
  uint32_t duration;
  int32_t time;
  float rate;
  } quicktime_elst_table_t;

typedef struct
  {
  int version;
  long flags;
  long total_entries;

  quicktime_elst_table_t *table;
  } quicktime_elst_t;

typedef struct
  {
  quicktime_elst_t elst;
  } quicktime_edts_t;

/* qtvr navg (v1.0) */
typedef struct {
int    version;        // Always 1
int    columns;    // Number of columns in movie
int    rows;        // Number rows in movie
int    reserved;        // Zero
int    loop_frames;        // Number of frames shot at each position
int    loop_dur;        // The duration of each frame
int    movietype;        // kStandardObject, kObjectInScene, or
// kOldNavigableMovieScene
int    loop_timescale;        // Number of ticks before next frame of
// loop is displayed
float    fieldofview;        // 180.0 for kStandardObject or
// kObjectInScene, actual  degrees for
// kOldNavigableMovieScene.
float    startHPan;        // Start horizontal pan angle in
//  degrees
float    endHPan;        // End horizontal pan angle in  degrees
float    endVPan;        // End vertical pan angle in  degrees
float    startVPan;        // Start vertical pan angle in  degrees
float    initialHPan;        // Initial horizontal pan angle in
//  degrees (poster view)
float    initialVPan;        // Initial vertical pan angle in  degrees
// (poster view)
long    reserved2;        // Zero
  } quicktime_navg_t;


typedef struct
  {
  quicktime_tkhd_t tkhd;
  quicktime_mdia_t mdia;
  quicktime_edts_t edts;
  int has_edts;
  
  quicktime_tref_t tref;
  quicktime_strl_t * strl; // != NULL for AVI files during reading and writing
  int chunk_sizes_alloc;
  int64_t * chunk_sizes; /* This contains the chunk sizes for audio
                            tracks. They can not so easily be obtained
                            during decoding */
  int has_tref;

  /* Stuff for writing chunks is stored here */
  quicktime_atom_t chunk_atom;
  int chunk_num;
  int chunk_samples;
  
  int64_t pts_offset;
  
  } quicktime_trak_t;


typedef struct
  {
  int version;
  long flags;
  uint64_t creation_time;
  uint64_t modification_time;
  long time_scale;
  uint64_t duration;
  float preferred_rate;
  float preferred_volume;
  uint8_t reserved[10];
  quicktime_matrix_t matrix;
  long preview_time;
  long preview_duration;
  long poster_time;
  long selection_time;
  long selection_duration;
  long current_time;
  long next_track_id;
  } quicktime_mvhd_t;


typedef struct
  {
  char *copyright;
  int copyright_len;
  char *name;
  int name_len;
  char *info;
  int info_len;
  /* Additional Metadata for libquicktime */
  char *album;
  int album_len;
  char *author;
  int author_len;
  char *artist;
  int artist_len;
  char *genre;
  int genre_len;
  char *track;
  int track_len;
  char *comment;
  int comment_len;
  int is_qtvr;
  /* player controls */
  char ctyp[4];
  quicktime_navg_t navg;
  quicktime_hdlr_t hdlr;
  int has_hdlr;
  } quicktime_udta_t;


typedef struct
  {
  int total_tracks;

  quicktime_mvhd_t mvhd;
  quicktime_trak_t *trak[MAXTRACKS];
  quicktime_udta_t udta;
  int has_ctab;
  quicktime_ctab_t ctab;
  int has_iods;
  quicktime_iods_t iods;

  } quicktime_moov_t;

typedef struct
  {
  quicktime_atom_t atom;
  } quicktime_mdat_t;

typedef struct
  {
  /* Offset of end of 8 byte chunk header relative to ix->base_offset */
  uint32_t relative_offset;
  /* size of data without 8 byte header */
  uint32_t size;
  } quicktime_ixtable_t;
                                                                                                                     
typedef struct
  {
  quicktime_atom_t atom;
  quicktime_ixtable_t *table;
  int table_size;
  int table_allocation;
  int longs_per_entry;
  int index_type;
  /* ixtable relative_offset is relative to this */
  int64_t base_offset;
  /* ix atom title */
  char tag[5];
  /* corresponding chunk id */
  char chunk_id[5];
  } quicktime_ix_t;
typedef struct
  {
  quicktime_atom_t atom;
                                                                                                                     
  } quicktime_movi_t;
                                                                                                                     
typedef struct
  {
  /* Start of start of corresponding ix## header */
  int64_t index_offset;
  /* Size not including 8 byte header */
  int index_size;
  /* duration in "ticks" */
  int duration;
                                                                                                                     
  /* Partial index for reading only. */
  quicktime_ix_t *ix;
  } quicktime_indxtable_t;

typedef struct
  {
  quicktime_atom_t atom;
  int longs_per_entry;
  int index_subtype;
  int index_type;
  /* corresponding chunk id: 00wb, 00dc */
  char chunk_id[5];
                                                                                                                     
  /* Number of partial indexes here */
  int table_size;
  int table_allocation;
  quicktime_indxtable_t *table;

  /* The following are used internally only. They are
     set by quicktime_finalize_strl and used by quicktime_finalize_indx */
  uint32_t offset;
  uint32_t size;
  } quicktime_indx_t;

/* AVI structures */

/* strh */

typedef struct
  {
  char fccType[4];
  char fccHandler[4];
  uint32_t dwFlags;
  uint32_t dwReserved1;
  uint32_t dwInitialFrames;
  uint32_t dwScale;
  uint32_t dwRate;
  uint32_t dwStart;
  uint32_t dwLength;
  uint32_t dwSuggestedBufferSize;
  uint32_t dwQuality;
  uint32_t dwSampleSize;

  struct {
  uint16_t left;
  uint16_t top;
  uint16_t right;
  uint16_t bottom;
    }  rcFrame;
  
  } quicktime_strh_t;

typedef struct
  {
  uint32_t dwMicroSecPerFrame;
  uint32_t dwMaxBytesPerSec;
  uint32_t dwReserved1;
  uint32_t dwFlags;
  uint32_t dwTotalFrames;
  uint32_t dwInitialFrames;
  uint32_t dwStreams;
  uint32_t dwSuggestedBufferSize;
  uint32_t dwWidth;
  uint32_t dwHeight;
  uint32_t dwScale;
  uint32_t dwRate;
  uint32_t dwStart;
  uint32_t dwLength;
  } quicktime_avih_t;

typedef struct
  {
  uint32_t  biSize;  /* sizeof(BITMAPINFOHEADER) */
  uint32_t  biWidth;
  uint32_t  biHeight;
  uint16_t  biPlanes; /* unused */
  uint16_t  biBitCount;
  char biCompression[5]; /* fourcc of image */
  uint32_t  biSizeImage;   /* size of image. For uncompressed images */
                           /* ( biCompression 0 or 3 ) can be zero.  */

  uint32_t  biXPelsPerMeter; /* unused */
  uint32_t  biYPelsPerMeter; /* unused */
  uint32_t  biClrUsed;       /* valid only for palettized images. */
  /* Number of colors in palette. */
  uint32_t  biClrImportant;

  /* Additional stuff */
  int ext_size;
  uint8_t * ext_data;
  } quicktime_BITMAPINFOHEADER_t;

typedef struct
  {
  uint32_t v1;
  uint16_t v2;
  uint16_t v3;
  uint8_t  v4[8];
  } quicktime_GUID_t;

typedef enum
  {
    LQT_WAVEFORMAT_WAVEFORMAT,
    LQT_WAVEFORMAT_PCMWAVEFORMAT,
    LQT_WAVEFORMAT_WAVEFORMATEX,
    LQT_WAVEFORMAT_WAVEFORMATEXTENSIBLE,
  } quicktime_WAVEFORMAT_type_t;

typedef struct
  {
  quicktime_WAVEFORMAT_type_t type;

  struct
    {
    struct
      {
      uint16_t  wFormatTag;     /* value that identifies compression format */
      uint16_t  nChannels;
      uint32_t  nSamplesPerSec;
      uint32_t  nAvgBytesPerSec;
      uint16_t  nBlockAlign;    /* size of a data sample */
      } WAVEFORMAT;
    struct
      {
      uint16_t  wBitsPerSample;
      } PCMWAVEFORMAT;
    struct
      {
      uint16_t  cbSize;         /* size of format-specific data */
      uint8_t * ext_data;
      int       ext_size; /* Can be different from cbSize for WAVEFORMATEXTENSIBLE */
      } WAVEFORMATEX;
    struct
      {
      union
        {
        uint16_t wValidBitsPerSample;
        uint16_t wSamplesPerBlock;
        uint16_t wReserved;
        } Samples;
      uint32_t    dwChannelMask;
      quicktime_GUID_t SubFormat;
      } WAVEFORMATEXTENSIBLE;
    } f;
  } quicktime_WAVEFORMAT_t;

typedef union
  {
  quicktime_BITMAPINFOHEADER_t bh;
  quicktime_WAVEFORMAT_t       wf;
  } quicktime_strf_t;

struct quicktime_strl_s
  {
  quicktime_atom_t atom;
  /* Super index for reading */
  quicktime_indx_t indx;

  /* strh stuff */
        
  int64_t  strh_offset;
  int64_t  end_pos;
  quicktime_strh_t strh;
  /* Format */
  quicktime_strf_t strf;

  /* The next ones will be used for generating index tables
     with the lowest possible error */

  int64_t total_bytes;
  int64_t total_samples;
  int64_t total_blocks;

  uint32_t size; /* The size after the strl was first written */

  /* Tag for writer with NULL termination: 00wb, 00dc   Not available in reader.*/
  char tag[5];
  /* Flags for reader.  Not available in writer. */
  int is_audio;
  int is_video;
  /* Notify reader the super indexes are valid */
  int have_indx;
  };

typedef struct
  {
  int64_t avih_offset;
  quicktime_avih_t avih;
  quicktime_atom_t atom;
  int64_t total_frames_offset;
                                                                                                                     
  /* AVI equivalent for each trak.  Use file->moov.total_tracks */
  /* Need it for super indexes during reading. */
  quicktime_strl_t *strl[MAXTRACKS];
  } quicktime_hdrl_t;

typedef struct
  {
  char tag[5];
  uint32_t flags;
  /* Start of 8 byte chunk header relative to start of the 'movi' string */
  int32_t offset;
  /* Size of chunk less the 8 byte header */
  int32_t size;
  } quicktime_idx1table_t;

typedef struct
  {
  char * IARL; // Archival Location. Indicates where the subject of the file is archived.
  char * IART; // Artist. Lists the artist of the original subject of the file. For example, "Michaelangelo."
  char * ICMS; // Commissioned. Lists the name of the person or organization that commissioned the subject of
               // the file. For example, "Pope Julian II."
  char * ICMT; // Comments. Provides general comments about the file or the subject of the file. If the
               // comment is several sentences long, end each sentence with a period. Do not include newline
               // characters.
  char * ICOP; // Copyright. Records the copyright information for the file. For example,
               // "Copyright Encyclopedia International 1991." If there are multiple copyrights, separate them
               // by a semicolon followed by a space.
  char * ICRD; // Creation date. Specifies the date the subject of the file was created. List dates in
               // year-month-day format, padding one-digit months and days with a zero on the left. For example,
               // "1553-05-03" for May 3, 1553.
  char * ICRP; // Cropped. Describes whether an image has been cropped and, if so, how it was cropped. For example,
               // "lower right corner."
  char * IDIM; // Dimensions. Specifies the size of the original subject of the file. For example,
               // "8.5 in h, 11 in w."
  char * IDPI; // Dots Per Inch. Stores dots per inch setting of the digitizer used to produce the file, such as
               // "300."
  char * IENG; // Engineer. Stores the name of the engineer who worked on the file. If there are multiple engineers,
               // separate the names by a semicolon and a blank. For example, "Smith, John; Adams, Joe."
  char * IGNR; // Genre. Describes the original work, such as, "landscape," "portrait," "still life," etc.
  char * IKEY; // Keywords. Provides a list of keywords that refer to the file or subject of the file. Separate
               // multiple keywords with a semicolon and a blank. For example, "Seattle; aerial view; scenery."
  char * ILGT; // Lightness. Describes the changes in lightness settings on the digitizer required to produce the
               // file. Note that the format of this information depends on hardware used.
  char * IMED; // Medium. Describes the original subject of the file, such as, "computer image," "drawing,"
               // "lithograph," and so forth.
  char * INAM; // Name. Stores the title of the subject of the file, such as, "Seattle From Above."
  char * IPLT; // Palette Setting. Specifies the number of colors requested when digitizing an image, such as "256."
  char * IPRD; // Product. Specifies the name of the title the file was originally intended for, such as
               // "Encyclopedia of Pacific Northwest Geography."
  char * ISBJ; // Subject. Describes the conbittents of the file, such as "Aerial view of Seattle."
  char * ISFT; // Software. Identifies the name of the software package used to create the file, such as
               // "Microsoft WaveEdit."
  char * ISHP; // Sharpness. Identifies the changes in sharpness for the digitizer required to produce the file
               // (the format depends on the hardware used).
  char * ISRC; // Source. Identifies the name of the person or organization who supplied the original subject of the
               // file. For example, "Trey Research."
  char * ISRF; // Source Form. Identifies the original form of the material that was digitized, such as "slide,"
               // "paper," "map," and so forth. This is not necessarily the same as IMED.
  char * ITCH; // Technician. Identifies the technician who digitized the subject file. For example, "Smith, John."
  } quicktime_riffinfo_t;

typedef struct
  {
  quicktime_atom_t atom;
  quicktime_idx1table_t *table;
  int table_size;
  int table_allocation;
  } quicktime_idx1_t;
                                                                                                                     
typedef struct
  {
  quicktime_atom_t atom;
  quicktime_movi_t movi;
  quicktime_hdrl_t hdrl;
  quicktime_riffinfo_t info;
  /* Full index */
  quicktime_idx1_t idx1;
  /* Notify reader the idx1 table is valid */
  int have_idx1;
  int have_hdrl;
  int have_info;
  } quicktime_riff_t;


/* table of pointers to every track */
typedef struct
  {
  quicktime_trak_t *track; /* real quicktime track corresponding to this table */
  int channels;

  int samplerate;

  /* number of audio channels in the track */
  lqt_channel_t * channel_setup;
  int64_t current_position;   /* current sample in output file */
  int64_t cur_chunk;      /* current chunk in output file */
  
  int cur_vbr_packet;     // Current packet within this chunk
  int total_vbr_packets;  // Total number of VBR packets in this chunk
  
  /* Last position if set by the codec. If last position and current position
     differ, the codec knows, that a seek happened since the last decode call */
  int64_t last_position;

  quicktime_codec_t * codec;

  int eof; /* This is set to 1 by the core if one tries to read beyond EOF */

  /* Another API enhancement: Codecs only deliver samples in the format specified by
     sample_format. The usual decode() functions will convert them to int16_t or float */
   
  lqt_sample_format_t sample_format; /* Set by the codec */

  uint8_t * sample_buffer;
  int sample_buffer_alloc;  /* Allocated size in SAMPLES of the sample buffer */

  /* VBR stuff */
  int64_t vbr_frame_start;
  
  /* The total samples are calculated while initializing, but they MIGHT
     be changed by the faad decoder, in the case resampling happens. */
  int64_t total_samples;

  /* WAV ID: Taken from the codec_info and saved here for writing the AVI header */
  int wav_id;

  /* PCM codecs need this */
  int block_align;

  lqt_compression_info_t ci;
  
  } quicktime_audio_map_t;

typedef struct
  {
  quicktime_trak_t *track;
  quicktime_trak_t *timecode_track;
  long current_position;   /* current frame in output file */
  long cur_chunk;      /* current chunk in output file */

  quicktime_codec_t * codec;
  /* Timestamp of the NEXT frame decoded. Unit is the timescale of the track */
  /* For Encoding: Timestamp of the LAST encoded frame */
  int64_t timestamp;
  
  int64_t stts_index;
  int64_t stts_count;

  int64_t ctts_index;
  int64_t ctts_count;

  int stream_cmodel; // Colormodel, which is read/written natively by the codec
  int io_cmodel;  // Colormodel, which is used by the encode/decode functions
  int stream_row_span, stream_row_span_uv;

  int io_row_span, io_row_span_uv;

  /* This is used by the core to do implicit colorspace conversion and scaling
     (NOT recommended!!) */
  uint8_t ** temp_frame;

  /* In some cases (IMX + VBI) the frame we are working with has greater
   * height than the track height from the tkhd atom.
   * This variable holds their difference. */
  int height_extension;

  lqt_chroma_placement_t chroma_placement;
  lqt_interlace_mode_t interlace_mode;
  
  // Timecode stuff
  uint32_t encode_timecode;
  int has_encode_timecode;
  long cur_timecode_chunk;      /* current chunk in output file */

  /* Timestamp of the last encoded timecode */
  int64_t timecode_timestamp;

  /* For decoding, *all* timecodes are read here.
   * For encoding, this contains a small buffer such that we don't
   start a new chunk for each timecode. */
  int num_timecodes;
  uint32_t * timecodes;
  int timecodes_written;

  /* Reordering table: We build this table during encoding and
     generate the stts and ctts atoms at the end */

  int * picture_numbers;
  int picture_numbers_alloc;

  int64_t * timestamps;
  int timestamps_alloc;

  int64_t duration;

  /* For encoding */
  quicktime_atom_t chunk_atom;
  int keyframe;

  lqt_compression_info_t ci;
  
  } quicktime_video_map_t;

/* Text track */

typedef struct
  {
  quicktime_trak_t *track;
  int is_chapter_track; /* For encoding only */
  int current_position;
  
  lqt_charset_converter_t * cnv;

  char * text_buffer;
  int text_buffer_alloc;

  int initialized; /* For encoding only */
  int64_t cur_chunk;
  
  } quicktime_text_map_t;

/* obji */

typedef struct
  {
  int version;
  int revision;
  int movieType;
  int viewStateCount;
  int defaultViewState;
  int mouseDownViewState;
  long viewDuration;
  long columns;
  long rows;
  float mouseMotionScale;
  float minPan;
  float maxPan;
  float defaultPan;
  float minTilt;
  float maxTilt;
  float defaultTilt;
  float minFOV;
  float FOV;
  float defaultFOV;
  float defaultViewCenterH;
  float defaultViewCenterV;
  float viewRate;
  float frameRate;
  long animSettings;
  long controlSettings;
  } quicktime_obji_t;

/* pdat */

typedef struct
  {
  int version;
  int revision;
  long imageRefTrackIndex;
  long hotSpotRefTrackIndex;
  float minPan;
  float maxPan;
  float defaultPan;
  float minTilt;
  float maxTilt;
  float defaultTilt;
  float minFOV;
  float maxFOV;
  float defaultFOV;
  long imageSizeX;
  long imageSizeY;
  int imageNumFramesX;
  int imageNumFramesY;
  long hotSpotSizeX;
  long hotSpotSizeY;
  int hotSpotNumFramesX;
  int hotSpotNumFramesY;
  long flags;
  char panoType[4];
  long reserved;
  } quicktime_pdat_t;

/* pHdr */

typedef struct
  {
  unsigned long    nodeID;
  float        defHPan;
  float        defVPan;
  float        defZoom;

  // constraints for this node; use zero for default
  float        minHPan;
  float        minVPan;
  float        minZoom;
  float        maxHPan;
  float        maxVPan;
  float        maxZoom;

  long        reserved1;        // must be zero
  long        reserved2;        // must be zero
  long        nameStrOffset;        // offset into string table atom
  long        commentStrOffset;    // offset into string table atom
  } quicktime_pHdr_t;

/* qtvr node */

typedef struct
  {
  int node_type;
  int64_t node_start; /* start frame */ 
  int64_t node_size; /* size of node in frames */
  quicktime_ndhd_t ndhd;
  quicktime_obji_t obji;
  quicktime_pdat_t pdat;
  } quicktime_qtvr_node_t;

/* file descriptor passed to all routines */

struct quicktime_s
  {
  FILE *stream;
  int64_t total_length;
  int encoding_started;
  quicktime_mdat_t mdat;
  quicktime_moov_t moov;
  quicktime_ftyp_t ftyp;
  int has_ftyp;

  lqt_file_type_t file_type;

  int rd;
  int wr;

  /* If the moov atom is compressed */
  int compressed_moov;
  unsigned char *moov_data;
  /*
   * Temporary storage of compressed sizes.  If the file length is shorter than the
   * uncompressed sizes, it won't work.
   */
  int64_t moov_end;
  int64_t moov_size;

  /* AVI tree */
  quicktime_riff_t *riff[MAX_RIFFS];
  int total_riffs;
  uint32_t max_riff_size;

  /* for begining and ending frame writes where the user wants to write the  */
  /* file descriptor directly */
  int64_t offset;
  /* I/O */
  /* Current position of virtual file descriptor */
  int64_t file_position;      
  // Work around a bug in glibc where ftello returns only 32 bits by maintaining
  // our own position
  int64_t ftell_position;

  /* Read ahead buffer */
  int64_t preload_size;      /* Enables preload when nonzero. */
  uint8_t *preload_buffer;
  int64_t preload_start;     /* Start of preload_buffer in file */
  int64_t preload_end;       /* End of preload buffer in file */
  int64_t preload_ptr;       /* Offset of preload_start in preload_buffer */

  /* Write ahead buffer */
  /* Amount of data in presave buffer */
  int64_t presave_size;
  /* Next presave byte's position in file */
  int64_t presave_position;
  uint8_t *presave_buffer;
  /* Presave doesn't matter a whole lot, so its size is fixed */
#define QUICKTIME_PRESAVE 0x100000

  /* mapping of audio channels to movie tracks */
  /* one audio map entry exists for each channel */
  int total_atracks;
  quicktime_audio_map_t *atracks;

  /* mapping of video tracks to movie tracks */
  int total_vtracks;
  quicktime_video_map_t *vtracks;

  /* Mapping of text tracks to movie tracks */
  int total_ttracks;
  quicktime_text_map_t *ttracks;

  /* Parameters for frame currently being decoded */
  int in_x, in_y, in_w, in_h, out_w, out_h;
  /*      Libquicktime change: color_model and row_span are now saved per track */
  /*	int color_model, row_span; */

  quicktime_qtvr_node_t qtvr_node[MAXNODES];

  /* Logging support */
  lqt_log_callback_t log_callback;
  void * log_data;

  /* I/O Error is saved here: It has the advantage, that
     codecs don't have to check the return values of quicktime_[read|write]_data
     all the time. */
  int io_error;
  int io_eof;
  
  quicktime_trak_t * write_trak;
  };

struct quicktime_codec_s
  {
  int (*delete_codec)(quicktime_codec_t *codec);
  int (*decode_video)(quicktime_t *file, 
                      unsigned char **row_pointers, 
                      int track);
  int (*encode_video)(quicktime_t *file, 
                      unsigned char **row_pointers, 
                      int track);
  /* API Change: Return value is the number of samples */
  int (*decode_audio)(quicktime_t *file, 
                      void * output,
                      long samples, 
                      int track);
  int (*encode_audio)(quicktime_t *file, 
                      void * input,
                      long samples,
                      int track);
  int (*set_parameter)(quicktime_t *file, 
                       int track, 
                       const char *key, 
                       const void *value);
  /* Set the encoding pass */
  int (*set_pass)(quicktime_t *file, 
                  int track, int pass, int total_passes,
                  const char * stats_file);
  /* Encode and write remaining stuff before closing */
  int (*flush)(quicktime_t *file, 
               int track);
  /* Resynchronize the codec after seeking */
  void (*resync)(quicktime_t *file, int track);

  int (*writes_compressed)(lqt_file_type_t type, const lqt_compression_info_t * ci);
  int (*init_compressed)(quicktime_t * file, int track);

  int (*write_packet)(quicktime_t * file, lqt_packet_t * p, int track);
  int (*read_packet)(quicktime_t * file, lqt_packet_t * p, int track);

  
  void *priv;

  /* The followings are for libquicktime only */
  void *module;     /* Needed by libquicktime for dynamic loading */

  lqt_codec_info_t * info;

  };

#endif

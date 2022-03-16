/*******************************************************************************
 language.c

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
#include <string.h>

#define LOG_DOMAIN "language"

/* Definitions for charsets. These are mappings from Macintosh charset
   symbols to iconv charsets. The ones, which are set to (char*)0
   are not available in iconv (replacements??) */

#define smRoman            { "MACINTOSH",    "ISO-8859-1"    },
#define smHebrew           { (char*)0,       "ISO-8859-8"    },
#define smJapanese         { (char*)0,       "SHIFT-JIS"     },
#define smArabic           { (char*)0,       "ISO-8859-6"    },
#define smExtArabic        { (char*)0,       "ISO-8859-6"    },
#define smGreek            { (char*)0,       (char*)0        },
#define smCentralEuroRoman { (char*)0,       "ISO-8859-2"    },
#define smIcelandic        { "MAC-IS",       (char*)0        },
#define smTradChinese      { (char*)0,       (char*)0        },
#define smDevanagari       { (char*)0,       (char*)0        },
#define smThai             { (char*)0,       (char*)0        },
#define smKorean           { (char*)0,       (char*)0        },
#define smSami             { "MAC-SAMI",     (char*)0        },
#define smCyrillic         { "MAC-CYRILLIC", (char*)0        },
#define smSimpChinese      { (char*)0,       (char*)0        },
#define smCeltic           { (char*)0,       (char*)0        },
#define smRomanian         { (char*)0,       (char*)0        },
#define smUkrainian        { "MAC-UK",       (char*)0        },
#define smArmenian         { (char*)0,       (char*)0        },
#define smGeorgian         { (char*)0,       (char*)0        },
#define smMongolian        { (char*)0,       (char*)0        },
#define smTibetan          { (char*)0,       (char*)0        },
#define smBengali          { (char*)0,       (char*)0        },
#define smGuriati          { (char*)0,       (char*)0        },
#define smGurmukhi         { (char*)0,       (char*)0        },
#define smOriya            { (char*)0,       (char*)0        },
#define smMalayalam        { (char*)0,       (char*)0        },
#define smKannada          { (char*)0,       (char*)0        },
#define smTamil            { (char*)0,       (char*)0        },
#define smTelugu           { (char*)0,       (char*)0        },
#define smSinhalese        { (char*)0,       (char*)0        },
#define smBurmese          { (char*)0,       (char*)0        },
#define smKhmer            { (char*)0,       (char*)0        },
#define smLaotian          { (char*)0,       (char*)0        },
#define smVietnamese       { (char*)0,       (char*)0        },
#define smEthiopic         { (char*)0,       (char*)0        },

/* Language / character set codecs */

static struct
  {
  int  mac_code;  // Integer mac code
  char language[4];   // 3 character language code

  struct
    {
    char * charset; // Character set (understood by iconv_open)
    char * charset_fallback; // Character set (understood by iconv_open)
    } cs;
  }
mac_languages[] =
  {
    {   0, "eng", smRoman }, // English
    {   1, "fra", smRoman }, // French
    {   2, "ger", smRoman }, // German
    {   3, "ita", smRoman }, // Italian
    {   4, "dut", smRoman }, // Dutch
    {   5, "swe", smRoman }, // Swedish
    {   6, "spa", smRoman }, // Spanish
    {   7, "dan", smRoman }, // Danish
    {   8, "por", smRoman }, // Portuguese
    {   9, "nor", smRoman }, // Norwegian
    {  10, "heb", smHebrew }, // Hebrew
    {  11, "jpn", smJapanese }, // Japanese
    {  12, "ara", smArabic }, // Arabic
    {  13, "fin", smRoman }, // Finnish
    {  14, "gre", smGreek }, // Greek
    {  15, "ice", smIcelandic }, // Icelandic
    {  16, "mlt", smRoman }, // Maltese
    {  17, "tur", smRoman }, // Turkish
    {  18, "scr", smRoman }, // Croatian
    {  19, "chi", smTradChinese }, // Traditional Chinese
    {  20, "urd", smArabic }, // Urdu
    {  21, "hin", smDevanagari }, // Hindi
    {  22, "tha", smThai }, // Thai
    {  23, "kor", smKorean}, // Korean
    {  24, "lit", smCentralEuroRoman }, // Lithuanian
    {  25, "pol", smCentralEuroRoman }, // Polish
    {  26, "hun", smCentralEuroRoman }, // Hungarian
    {  27, "est", smCentralEuroRoman }, // Estonian
    {  28, "lav", smCentralEuroRoman }, // Latvian
    {  29, "smi", smSami }, // Saamisk
    {  30, "fao", smIcelandic}, // Faeroese
    {  31, "far", smArabic }, // Farsi
    {  32, "rus", smCyrillic }, // Russian
    {  33, "chi", smSimpChinese }, // Simplified Chinese
    {  34, "dut", smRoman }, // Flemish
    {  35, "gle", smCeltic }, // Irish
    {  36, "alb", smRoman }, // Albanian
    {  37, "rum", smRomanian }, // Romanian
    {  38, "cze", smCentralEuroRoman }, // Czech
    {  39, "slo", smCentralEuroRoman }, // Slovak
    {  40, "slv", smCentralEuroRoman }, // Slovenian
    {  41, "yid", smHebrew }, // Yiddish
    {  42, "scc", smCyrillic }, // Serbian
    {  43, "mac", smCyrillic }, // Macedonian
    {  44, "bul", smCyrillic }, // Bulgarian
    {  45, "ukr", smUkrainian }, // Ukrainian
    {  46, "bel", smCyrillic }, // Byelorussian
    {  47, "uzb", smCyrillic }, // Uzbek
    {  48, "kaz", smCyrillic }, // Kazakh
    {  49, "aze", smCyrillic }, // Azerbaijani (cyrillic)
    {  50, "aze", smArabic }, // Azerbaijani (arabic)
    {  51, "arm", smArmenian }, // Armenian
    {  52, "geo", smGeorgian }, // Georgian
    {  53, "mol", smCyrillic }, // Moldavian
    {  54, "kir", smCyrillic }, // Kirghiz
    {  55, "tgk", smCyrillic }, // Tajiki
    {  56, "tuk", smCyrillic }, // Turkmen
    {  57, "mon", smMongolian }, // Mongolian
    {  58, "mon", smCyrillic }, // Mongolian (cyrillic)
    {  59, "pus", smArabic }, // Pashto
    {  60, "kur", smArabic }, // Kurdish
    {  61, "kas", smArabic }, // Kashmiri
    {  62, "snd", smExtArabic }, // Sindhi
    {  63, "tib", smTibetan }, // Tibetan
    {  64, "nep", smDevanagari }, // Nepali
    {  65, "san", smDevanagari }, // Sanskrit
    //    {  66, "", smDevanagari }, // Marathi ??
    {  67, "ben", smBengali }, // Bengali
    {  68, "asm", smBengali }, // Assamese
    {  69, "guj", smGuriati }, // Gujarati
    {  70, "pan", smGurmukhi }, // Punjabi
    {  71, "ori", smOriya }, // Oriya
    {  72, "mal", smMalayalam }, // Malayalam
    {  73, "kan", smKannada }, // Kannada
    {  74, "tam", smTamil }, // Tamil
    {  75, "tel", smTelugu }, // Telugu
    {  76, "sin", smSinhalese }, // Sinhalese
    {  77, "bur", smBurmese }, // Burmese
    {  78, "khm", smKhmer }, // Khmer
    {  79, "lao", smLaotian }, // Lao
    {  80, "vie", smVietnamese }, // Vietnamese
    {  81, "ind", smRoman }, // Indonesian
    {  82, "tgl", smRoman }, // Tagalog
    {  83, "may", smRoman }, // Malay (roman)
    {  84, "may", smArabic }, // Malay (arabic)
    {  85, "amh", smEthiopic }, // Amharic
    {  86, "tir", smEthiopic }, // Tigrinya
    {  87, "orm", smEthiopic }, // Oromo
    {  88, "som", smRoman }, // Somali
    {  89, "swa", smRoman }, // Swahili
    {  90, "kin", smRoman }, // Kinyarwanda
    {  91, "run", smRoman }, // Rundi
    {  92, "nya", smRoman }, // Chewa
    {  93, "mlg", smRoman }, // Malagasy
    {  94, "epo", smRoman }, // Esperanto
    { 128, "wel", smRoman }, // Welsh
    { 129, "baq", smRoman }, // Basque
    { 130, "cat", smRoman }, // Catalan
    { 131, "lat", smRoman }, // Latin
    { 132, "que", smRoman }, // Quechua
    { 133, "grn", smRoman }, // Guarani
    { 134, "aym", smRoman }, // Aymara
    { 135, "tat", smCyrillic }, // Tatar
    { 136, "uig", smArabic }, // Uighur
    { 137, "dzo", smTibetan }, // Dzongkha
    { 138, "jav", smRoman }, // Javanese (roman)
  };

#define NUM_CODES (sizeof(mac_languages)/sizeof(mac_languages[0]))

static int get_language(quicktime_trak_t * trak, char * ret,
                        lqt_file_type_t file_type)
  {
  int i;
  if(IS_MP4(file_type))
    {
    ret[0] = ((trak->mdia.mdhd.language >> 10) & 0x1f) + 0x60;
    ret[1] = ((trak->mdia.mdhd.language >> 5) & 0x1f)  + 0x60;
    ret[2] = (trak->mdia.mdhd.language & 0x1f)         + 0x60;
    ret[3] = '\0';
    return 1;
    }
  for(i = 0; i < NUM_CODES; i++)
    {
    if(trak->mdia.mdhd.language == mac_languages[i].mac_code)
      {
      strcpy(ret, mac_languages[i].language);
      return 1;
      }
    }
  return 0;
  }

static char * unicode_string = LQT_UTF_8_16;

const char * lqt_get_charset(int mac_code, lqt_file_type_t file_type)
  {
  int i;

  if(IS_MP4(file_type))
    return unicode_string;
  
  for(i = 0; i < NUM_CODES; i++)
    {
    if(mac_code == mac_languages[i].mac_code)
      return mac_languages[i].cs.charset;
    }
  return (char*)0;
  }

const char * lqt_get_charset_fallback(int mac_code, lqt_file_type_t file_type)
  {
  int i;

  if(IS_MP4(file_type))
    return unicode_string;
  
  for(i = 0; i < NUM_CODES; i++)
    {
    if(mac_code == mac_languages[i].mac_code)
      return mac_languages[i].cs.charset_fallback;
    }
  return (char*)0;
  }


static int set_language_code(quicktime_trak_t * trak,
                             const char * language, lqt_file_type_t file_type)
  {
  int i;

  if(IS_MP4(file_type))
    {
    trak->mdia.mdhd.language =
      ((int)(language[0]-0x60) << 10) |
      ((int)(language[1]-0x60) << 5) |
      ((int)(language[2]-0x60));
    return 0;
    }
  
  for(i = 0; i < NUM_CODES; i++)
    {
    if(!strcmp(language, mac_languages[i].language))
      {
      trak->mdia.mdhd.language = mac_languages[i].mac_code;
      return 1;
      }
    }
  return 0;
  }

void lqt_set_audio_language(quicktime_t * file, int track, const char * language)
  {
  if((track < 0) || (track >= file->total_atracks))
    return;
  set_language_code((file->atracks[track].track), language, file->file_type);
  }

int  lqt_get_audio_language(quicktime_t * file, int track, char * language)
  {
  if((track < 0) || (track >= file->total_atracks))
    return 0;
  return get_language(file->atracks[track].track, language, file->file_type);
  }


void lqt_set_text_language(quicktime_t * file, int track, const char * language)
  {
  if((track < 0) || (track >= file->total_ttracks))
    return;
  
  set_language_code((file->ttracks[track].track), language, file->file_type);
  }

int  lqt_get_text_language(quicktime_t * file, int track, char * language)
  {
  if((track < 0) || (track >= file->total_ttracks))
    return 0;
  return get_language(file->ttracks[track].track, language, file->file_type);
  }

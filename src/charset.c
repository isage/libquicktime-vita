/*******************************************************************************
 charset.c

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
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LOG_DOMAIN "charset"

struct lqt_charset_converter_s
  {
  iconv_t cd;
  quicktime_t * file; /* For logging */
  int utf_8_16;
  char * out_charset;

  char * in_buffer;
  int in_buffer_alloc;
  };

lqt_charset_converter_t *
lqt_charset_converter_create(quicktime_t * file, const char * src_charset, const char * dst_charset)
  {
  lqt_charset_converter_t * ret = calloc(1, sizeof(*ret));

  if(!strcmp(src_charset, LQT_UTF_8_16))
    {
    ret->out_charset = malloc(strlen(dst_charset)+1);
    strcpy(ret->out_charset, dst_charset);
    ret->utf_8_16 = 1;
    ret->cd = (iconv_t)-1;
    }
  else
    {
    ret->cd = iconv_open(dst_charset, src_charset);
    if(ret->cd == (iconv_t)-1)
      {
      free(ret);
      return (lqt_charset_converter_t*)0;
      }
    }
  ret->file = file;
  return ret;
  
  }

#define BYTES_INCREMENT 10

static int do_convert(lqt_charset_converter_t * cnv, char * in_string, int len, int * out_len,
               char ** ret, int * ret_alloc)
  {

  char *inbuf;
  char *outbuf;
  int output_pos;

  size_t inbytesleft;
  size_t outbytesleft;

  /* Check for MP4 Unicode */
  if(cnv->utf_8_16 && (cnv->cd == (iconv_t)-1))
    {
    /* Byte order Little Endian */
    if((len > 1) &&
       ((uint8_t)in_string[0] == 0xff) &&
       ((uint8_t)in_string[1] == 0xfe))
      {
      cnv->cd = iconv_open(cnv->out_charset, "UTF-16LE");
      if(cnv->cd == (iconv_t)-1)
        {
        lqt_log(cnv->file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot open iconv for conversion to %s from %s", 
                cnv->out_charset, "UTF-16LE");
        return 0;
        }
      }
    /* Byte order Big Endian */
    else if((len > 1) &&
            ((uint8_t)in_string[0] == 0xfe) &&
            ((uint8_t)in_string[1] == 0xff))
      {
      cnv->cd = iconv_open(cnv->out_charset, "UTF-16BE");
      if(cnv->cd == (iconv_t)-1)
        {
        lqt_log(cnv->file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot open iconv for conversion to %s from %s",
                cnv->out_charset, "UTF-16BE");
        return 0;
        }
      }
    /* UTF-8 */
    else if(!strcmp(cnv->out_charset, "UTF-8"))
      {
      if(*ret_alloc < len+1)
        {
        *ret_alloc = len + BYTES_INCREMENT;
        *ret       = realloc(*ret, *ret_alloc);
        }
      strncpy(*ret, in_string, len);
      (*ret)[len] = '\0';
      if(out_len)
        *out_len = len;
      return 1;
      }
    else
      {
      cnv->cd = iconv_open(cnv->out_charset, "UTF-8");
      if(cnv->cd == (iconv_t)-1)
        {
        lqt_log(cnv->file, LQT_LOG_ERROR, LOG_DOMAIN, "Cannot open iconv for conversion to %s from %s",
                cnv->out_charset, "UTF-8");
        return 0;
        }
      }
    }
    
  
  if((*ret_alloc) < len + BYTES_INCREMENT)
    {
    *ret_alloc = len + BYTES_INCREMENT;
    *ret       = realloc(*ret, *ret_alloc);
    }
  inbytesleft  = len;
  outbytesleft = *ret_alloc;

  inbuf  = in_string;
  outbuf = *ret;
  while(1)
    {
    if(iconv(cnv->cd, &inbuf, &inbytesleft,
             &outbuf, &outbytesleft) == (size_t)-1)
      {
      switch(errno)
        {
        case E2BIG:
          output_pos = (int)(outbuf - *ret);

          *ret_alloc   += BYTES_INCREMENT;
          outbytesleft += BYTES_INCREMENT;

          *ret = realloc(*ret, *ret_alloc);
          outbuf = &((*ret)[output_pos]);
          break;
        case EILSEQ:
          lqt_log(cnv->file, LQT_LOG_ERROR, LOG_DOMAIN, "Invalid Multibyte sequence");
          return 0;
          break;
        case EINVAL:
          lqt_log(cnv->file, LQT_LOG_ERROR, LOG_DOMAIN, "Incomplete Multibyte sequence");
          return 0;
          break;
        }
      }
    if(!inbytesleft)
      break;
    }
  /* Zero terminate */

  output_pos = (int)(outbuf - *ret);

  if(outbytesleft < 2)
    {
    *ret_alloc+=2;
    *ret = realloc(*ret, *ret_alloc);
    outbuf = &((*ret)[output_pos]);
    }
  outbuf[0] = '\0';
  outbuf[1] = '\0';
  if(out_len)
    *out_len = outbuf - *ret;
  return 1;
  }

/* We convert all strings "in place" */

void lqt_charset_convert(lqt_charset_converter_t * cnv,
                         char ** str, int in_len, int * out_len)
  {
  char * new = (char*)0;
  int new_alloc = 0;
  if(!(*str))
    return;
  if(in_len < 0) in_len = strlen(*str);
  
  if(!do_convert(cnv, *str, in_len, out_len,
                 &new, &new_alloc))
    {
    if(new) free(new);
    return;
    }
  free(*str);
  *str = new;
  }

void lqt_charset_convert_realloc(lqt_charset_converter_t * cnv,
                                 const char * in_str, int in_len,
                                 char ** out_str, int * out_alloc, int * out_len)
  {
  if(in_len < 0) in_len = strlen(in_str);

  if(cnv->in_buffer_alloc < in_len + 2)
    {
    cnv->in_buffer_alloc = in_len + 128;
    cnv->in_buffer = realloc(cnv->in_buffer, cnv->in_buffer_alloc);
    }

  memcpy(cnv->in_buffer, in_str, in_len);
  cnv->in_buffer[in_len] = '\0';
  cnv->in_buffer[in_len+1] = '\0';
  
  do_convert(cnv, cnv->in_buffer, in_len, out_len,
             out_str, out_alloc);
  }

void lqt_charset_converter_destroy(lqt_charset_converter_t * cnv)
  {
  if(cnv->cd != (iconv_t)-1)
    iconv_close(cnv->cd);
  if(cnv->out_charset)
    free(cnv->out_charset);
  free(cnv);
  }

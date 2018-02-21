/* mz_strm_posix.h -- Stream for filesystem access for posix/linux
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 2009-2010 Mathias Svensson
     Modifications for Zip64 support
     http://result42.com
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_STREAM_POSIX_H
#define _MZ_STREAM_POSIX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

int32_t mz_stream_posix_open(void *stream, const char *path, int32_t mode);
int32_t mz_stream_posix_is_open(void *stream);
int32_t mz_stream_posix_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_posix_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_posix_tell(void *stream);
int32_t mz_stream_posix_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_posix_close(void *stream);
int32_t mz_stream_posix_error(void *stream);

void*   mz_stream_posix_create(void **stream);
void    mz_stream_posix_delete(void **stream);

void*   mz_stream_posix_get_interface(void);

/***************************************************************************/

#if !defined(_WIN32) && !defined(MZ_USE_WIN32_API)
#define mz_stream_os_open    mz_stream_posix_open
#define mz_stream_os_is_open mz_stream_posix_is_open
#define mz_stream_os_read    mz_stream_posix_read
#define mz_stream_os_write   mz_stream_posix_write
#define mz_stream_os_tell    mz_stream_posix_tell
#define mz_stream_os_seek    mz_stream_posix_seek
#define mz_stream_os_close   mz_stream_posix_close
#define mz_stream_os_error   mz_stream_posix_error

#define mz_stream_os_create  mz_stream_posix_create
#define mz_stream_os_delete  mz_stream_posix_delete

#define mz_stream_os_get_interface \
                             mz_stream_posix_get_interface
#endif

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

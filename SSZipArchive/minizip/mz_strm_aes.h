/* mz_strm_aes.h -- Stream for WinZIP AES encryption
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
      https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_STREAM_AES_H
#define _MZ_STREAM_AES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

int32_t mz_stream_aes_open(void *stream, const char *filename, int32_t mode);
int32_t mz_stream_aes_is_open(void *stream);
int32_t mz_stream_aes_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_aes_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_aes_tell(void *stream);
int32_t mz_stream_aes_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_aes_close(void *stream);
int32_t mz_stream_aes_error(void *stream);

void    mz_stream_aes_set_password(void *stream, const char *password);
void    mz_stream_aes_set_encryption_mode(void *stream, int16_t encryption_mode);

int32_t mz_stream_aes_get_prop_int64(void *stream, int32_t prop, int64_t *value);

void*   mz_stream_aes_create(void **stream);
void    mz_stream_aes_delete(void **stream);

void*   mz_stream_aes_get_interface(void);

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

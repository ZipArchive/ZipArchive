/* mz_strm.h -- Stream interface
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_STREAM_H
#define _MZ_STREAM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

#define MZ_STREAM_PROP_TOTAL_IN             (1)
#define MZ_STREAM_PROP_TOTAL_IN_MAX         (2)
#define MZ_STREAM_PROP_TOTAL_OUT            (3)
#define MZ_STREAM_PROP_TOTAL_OUT_MAX        (4)
#define MZ_STREAM_PROP_HEADER_SIZE          (5)
#define MZ_STREAM_PROP_FOOTER_SIZE          (6)
#define MZ_STREAM_PROP_DISK_SIZE            (7)
#define MZ_STREAM_PROP_DISK_NUMBER          (8)
#define MZ_STREAM_PROP_COMPRESS_LEVEL       (9)

/***************************************************************************/

typedef int32_t (*mz_stream_open_cb)           (void *stream, const char *path, int32_t mode);
typedef int32_t (*mz_stream_is_open_cb)        (void *stream);
typedef int32_t (*mz_stream_read_cb)           (void *stream, void *buf, int32_t size);
typedef int32_t (*mz_stream_write_cb)          (void *stream, const void *buf, int32_t size);
typedef int64_t (*mz_stream_tell_cb)           (void *stream);
typedef int32_t (*mz_stream_seek_cb)           (void *stream, int64_t offset, int32_t origin);
typedef int32_t (*mz_stream_close_cb)          (void *stream);
typedef int32_t (*mz_stream_error_cb)          (void *stream);
typedef void*   (*mz_stream_create_cb)         (void **stream);
typedef void    (*mz_stream_destroy_cb)        (void **stream);

typedef int32_t (*mz_stream_get_prop_int64_cb) (void *stream, int32_t prop, int64_t *value);
typedef int32_t (*mz_stream_set_prop_int64_cb) (void *stream, int32_t prop, int64_t value);

/***************************************************************************/

typedef struct mz_stream_vtbl_s
{
    mz_stream_open_cb           open;
    mz_stream_is_open_cb        is_open;
    mz_stream_read_cb           read;
    mz_stream_write_cb          write;
    mz_stream_tell_cb           tell;
    mz_stream_seek_cb           seek;
    mz_stream_close_cb          close;
    mz_stream_error_cb          error;
    mz_stream_create_cb         create;
    mz_stream_destroy_cb        destroy;

    mz_stream_get_prop_int64_cb get_prop_int64;
    mz_stream_set_prop_int64_cb set_prop_int64;
} mz_stream_vtbl;

typedef struct mz_stream_s {
    mz_stream_vtbl              *vtbl;
    struct mz_stream_s          *base;
} mz_stream;

/***************************************************************************/

int32_t mz_stream_open(void *stream, const char *path, int32_t mode);
int32_t mz_stream_is_open(void *stream);
int32_t mz_stream_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_read_uint8(void *stream, uint8_t *value);
int32_t mz_stream_read_uint16(void *stream, uint16_t *value);
int32_t mz_stream_read_uint32(void *stream, uint32_t *value);
int32_t mz_stream_read_uint64(void *stream, uint64_t *value);
int32_t mz_stream_write(void *stream, const void *buf, int32_t size);
int32_t mz_stream_write_uint8(void *stream, uint8_t value);
int32_t mz_stream_write_uint16(void *stream, uint16_t value);
int32_t mz_stream_write_uint32(void *stream, uint32_t value);
int32_t mz_stream_write_uint64(void *stream, uint64_t value);
int32_t mz_stream_copy(void *target, void *source, int32_t len);
int64_t mz_stream_tell(void *stream);
int32_t mz_stream_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_close(void *stream);
int32_t mz_stream_error(void *stream);

int32_t mz_stream_set_base(void *stream, void *base);
int32_t mz_stream_get_prop_int64(void *stream, int32_t prop, int64_t *value);
int32_t mz_stream_set_prop_int64(void *stream, int32_t prop, int64_t value);

void*   mz_stream_create(void **stream, mz_stream_vtbl *vtbl);
void    mz_stream_delete(void **stream);

/***************************************************************************/

typedef int32_t (*mz_stream_crc32_update)(int32_t value, const void *buf, int32_t size);

int32_t mz_stream_crc32_open(void *stream, const char *filename, int32_t mode);
int32_t mz_stream_crc32_is_open(void *stream);
int32_t mz_stream_crc32_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_crc32_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_crc32_tell(void *stream);
int32_t mz_stream_crc32_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_crc32_close(void *stream);
int32_t mz_stream_crc32_error(void *stream);

int32_t mz_stream_crc32_get_value(void *stream);

int32_t mz_stream_crc32_get_prop_int64(void *stream, int32_t prop, int64_t *value);
int32_t mz_stream_crc32_set_update_func(void *stream, mz_stream_crc32_update update);

void*   mz_stream_crc32_create(void **stream);
void    mz_stream_crc32_delete(void **stream);

void*   mz_stream_crc32_get_interface(void);

/***************************************************************************/

int32_t mz_stream_raw_open(void *stream, const char *filename, int32_t mode);
int32_t mz_stream_raw_is_open(void *stream);
int32_t mz_stream_raw_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_raw_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_raw_tell(void *stream);
int32_t mz_stream_raw_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_raw_close(void *stream);
int32_t mz_stream_raw_error(void *stream);

int32_t mz_stream_raw_get_prop_int64(void *stream, int32_t prop, int64_t *value);
int32_t mz_stream_raw_set_prop_int64(void *stream, int32_t prop, int64_t value);

void*   mz_stream_raw_create(void **stream);
void    mz_stream_raw_delete(void **stream);

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

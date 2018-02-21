/* mz_strm_posix.c -- Stream for filesystem access for posix/linux
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Modifications for Zip64 support
     Copyright (C) 2009-2010 Mathias Svensson
     http://result42.com
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mz.h"
#include "mz_strm.h"
#include "mz_strm_posix.h"

/***************************************************************************/

#if defined(MZ_USE_FILE32API)
#  define fopen64 fopen
#  define ftello64 ftell
#  define fseeko64 fseek
#else
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || \
      defined(__OpenBSD__) || defined(__APPLE__) || defined(__ANDROID__)
#    define fopen64 fopen
#    define ftello64 ftello
#    define fseeko64 fseeko
#  endif
#  ifdef _MSC_VER
#    define fopen64 fopen
#    if (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
#      define ftello64 _ftelli64
#      define fseeko64 _fseeki64
#    else /* old MSC */
#      define ftello64 ftell
#      define fseeko64 fseek
#    endif
#  endif
#endif

/***************************************************************************/

mz_stream_vtbl mz_stream_posix_vtbl = {
    mz_stream_posix_open,
    mz_stream_posix_is_open,
    mz_stream_posix_read,
    mz_stream_posix_write,
    mz_stream_posix_tell,
    mz_stream_posix_seek,
    mz_stream_posix_close,
    mz_stream_posix_error,
    mz_stream_posix_create,
    mz_stream_posix_delete
};

/***************************************************************************/

typedef struct mz_stream_posix_s
{
    mz_stream   stream;
    int32_t     error;
    FILE        *handle;
} mz_stream_posix;

/***************************************************************************/

int32_t mz_stream_posix_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_posix *posix = (mz_stream_posix *)stream;
    const char *mode_fopen = NULL;

    if (path == NULL)
        return MZ_STREAM_ERROR;

    if ((mode & MZ_OPEN_MODE_READWRITE) == MZ_OPEN_MODE_READ)
        mode_fopen = "rb";
    else if (mode & MZ_OPEN_MODE_APPEND)
        mode_fopen = "ab";
    else if (mode & MZ_OPEN_MODE_CREATE)
        mode_fopen = "wb";
    else
        return MZ_STREAM_ERROR;

    posix->handle = fopen64(path, mode_fopen);
    if (posix->handle == NULL)
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }

    return MZ_OK;
}

int32_t mz_stream_posix_is_open(void *stream)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    if (posix->handle == NULL)
        return MZ_STREAM_ERROR;
    return MZ_OK;
}

int32_t mz_stream_posix_read(void *stream, void *buf, int32_t size)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    int32_t read = (int32_t)fread(buf, 1, (size_t)size, posix->handle);
    if (read < size && ferror(posix->handle))
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }
    return read;
}

int32_t mz_stream_posix_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    int32_t written = (int32_t)fwrite(buf, 1, (size_t)size, posix->handle);
    if (written < size && ferror(posix->handle))
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }
    return written;
}

int64_t mz_stream_posix_tell(void *stream)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    int64_t position = ftello64(posix->handle);
    if (position == -1)
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }
    return position;
}

int32_t mz_stream_posix_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    int32_t fseek_origin = 0;

    switch (origin)
    {
        case MZ_SEEK_CUR:
            fseek_origin = SEEK_CUR;
            break;
        case MZ_SEEK_END:
            fseek_origin = SEEK_END;
            break;
        case MZ_SEEK_SET:
            fseek_origin = SEEK_SET;
            break;
        default:
            return MZ_STREAM_ERROR;
    }

    if (fseeko64(posix->handle, offset, fseek_origin) != 0)
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }

    return MZ_OK;
}

int32_t mz_stream_posix_close(void *stream)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    int32_t closed = 0;
    if (posix->handle != NULL)
    {
        closed = fclose(posix->handle);
        posix->handle = NULL;
    }
    if (closed != 0)
    {
        posix->error = errno;
        return MZ_STREAM_ERROR;
    }
    return MZ_OK;
}

int32_t mz_stream_posix_error(void *stream)
{
    mz_stream_posix *posix = (mz_stream_posix*)stream;
    return posix->error;
}

void *mz_stream_posix_create(void **stream)
{
    mz_stream_posix *posix = NULL;

    posix = (mz_stream_posix *)malloc(sizeof(mz_stream_posix));
    if (posix != NULL)
    {
        memset(posix, 0, sizeof(mz_stream_posix));
        posix->stream.vtbl = &mz_stream_posix_vtbl;
    }
    if (stream != NULL)
        *stream = posix;
 
    return posix;
}

void mz_stream_posix_delete(void **stream)
{
    mz_stream_posix *posix = NULL;
    if (stream == NULL)
        return;
    posix = (mz_stream_posix *)*stream;
    if (posix != NULL)
        free(posix);
    *stream = NULL;
}

void *mz_stream_posix_get_interface(void)
{
    return (void *)&mz_stream_posix_vtbl;
}

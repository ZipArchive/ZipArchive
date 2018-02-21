/* mz_strm_buf.c -- Stream for buffering reads/writes
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   This version of ioapi is designed to buffer IO.

   Copyright (C) 2010-2018 Nathan Moinvaziri
      https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mz.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"

/***************************************************************************/

#if 0
#  define mz_stream_buffered_print(s,f,...) printf(f,__VA_ARGS__);
#else
#  define mz_stream_buffered_print(s,f,...)
#endif

/***************************************************************************/

mz_stream_vtbl mz_stream_buffered_vtbl = {
    mz_stream_buffered_open,
    mz_stream_buffered_is_open,
    mz_stream_buffered_read,
    mz_stream_buffered_write,
    mz_stream_buffered_tell,
    mz_stream_buffered_seek,
    mz_stream_buffered_close,
    mz_stream_buffered_error,
    mz_stream_buffered_create,
    mz_stream_buffered_delete
};

/***************************************************************************/

typedef struct mz_stream_buffered_s {
    mz_stream stream;
    int32_t   error;
    char      readbuf[INT16_MAX];
    int32_t   readbuf_len;
    int32_t   readbuf_pos;
    int32_t   readbuf_hits;
    int32_t   readbuf_misses;
    char      writebuf[INT16_MAX];
    int32_t   writebuf_len;
    int32_t   writebuf_pos;
    int32_t   writebuf_hits;
    int32_t   writebuf_misses;
    int64_t   position;
} mz_stream_buffered;

/***************************************************************************/

int32_t mz_stream_buffered_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    mz_stream_buffered_print(buffered, "open [mode %d]\n", mode);
    return mz_stream_open(buffered->stream.base, path, mode);
}

int32_t mz_stream_buffered_is_open(void *stream)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    return mz_stream_is_open(buffered->stream.base);
}

static int32_t mz_stream_buffered_flush(void *stream, int32_t *written)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int32_t total_bytes_written = 0;
    int32_t bytes_to_write = buffered->writebuf_len;
    int32_t bytes_left_to_write = buffered->writebuf_len;
    int32_t bytes_written = 0;

    *written = 0;

    while (bytes_left_to_write > 0)
    {
        bytes_written = mz_stream_write(buffered->stream.base, 
            buffered->writebuf + (bytes_to_write - bytes_left_to_write), bytes_left_to_write);

        if (bytes_written != bytes_left_to_write)
            return MZ_STREAM_ERROR;

        buffered->writebuf_misses += 1;

        mz_stream_buffered_print(stream, "write flush [%d:%d len %d]\n", 
            bytes_to_write, bytes_left_to_write, buffered->writebuf_len);

        total_bytes_written += bytes_written;
        bytes_left_to_write -= bytes_written;
        buffered->position += bytes_written;
    }

    buffered->writebuf_len = 0;
    buffered->writebuf_pos = 0;

    *written = total_bytes_written;
    return MZ_OK;
}

int32_t mz_stream_buffered_read(void *stream, void *buf, int32_t size)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int32_t buf_len = 0;
    int32_t bytes_to_read = 0;
    int32_t bytes_to_copy = 0;
    int32_t bytes_left_to_read = size;
    int32_t bytes_read = 0;

    mz_stream_buffered_print(stream, "read [size %ld pos %lld]\n", size, buffered->position);

    if (buffered->writebuf_len > 0)
        mz_stream_buffered_print(stream, "switch from write to read, not yet supported [%lld]\n", 
            buffered->position);

    while (bytes_left_to_read > 0)
    {
        if ((buffered->readbuf_len == 0) || (buffered->readbuf_pos == buffered->readbuf_len))
        {
            if (buffered->readbuf_len == sizeof(buffered->readbuf))
            {
                buffered->readbuf_pos = 0;
                buffered->readbuf_len = 0;
            }

            bytes_to_read = sizeof(buffered->readbuf) - (buffered->readbuf_len - buffered->readbuf_pos);
            bytes_read = mz_stream_read(buffered->stream.base, buffered->readbuf + buffered->readbuf_pos, bytes_to_read);
            if (bytes_read < 0)
                return bytes_read;

            buffered->readbuf_misses += 1;
            buffered->readbuf_len += bytes_read;
            buffered->position += bytes_read;

            mz_stream_buffered_print(stream, "filled [read %d/%d buf %d:%d pos %lld]\n", 
                bytes_read, bytes_to_read, buffered->readbuf_pos, buffered->readbuf_len, buffered->position);

            if (bytes_read == 0)
                break;
        }

        if ((buffered->readbuf_len - buffered->readbuf_pos) > 0)
        {
            bytes_to_copy = (uint32_t)(buffered->readbuf_len - buffered->readbuf_pos);
            if (bytes_to_copy > bytes_left_to_read)
                bytes_to_copy = bytes_left_to_read;

            memcpy((char *)buf + buf_len, buffered->readbuf + buffered->readbuf_pos, bytes_to_copy);

            buf_len += bytes_to_copy;
            bytes_left_to_read -= bytes_to_copy;

            buffered->readbuf_hits += 1;
            buffered->readbuf_pos += bytes_to_copy;

            mz_stream_buffered_print(stream, "emptied [copied %d remaining %d buf %d:%d pos %lld]\n", 
                bytes_to_copy, bytes_left_to_read, buffered->readbuf_pos, buffered->readbuf_len, buffered->position);
        }
    }

    return size - bytes_left_to_read;
}

int32_t mz_stream_buffered_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int32_t bytes_to_write = size;
    int32_t bytes_left_to_write = size;
    int32_t bytes_to_copy = 0;
    int32_t bytes_used = 0;
    int32_t bytes_flushed = 0;


    mz_stream_buffered_print(stream, "write [size %ld len %d pos %lld]\n", 
        size, buffered->writebuf_len, buffered->position);

    if (buffered->readbuf_len > 0)
    {
        buffered->position -= buffered->readbuf_len;
        buffered->position += buffered->readbuf_pos;

        buffered->readbuf_len = 0;
        buffered->readbuf_pos = 0;

        mz_stream_buffered_print(stream, "switch from read to write [%lld]\n", buffered->position);

        if (mz_stream_seek(buffered->stream.base, buffered->position, MZ_SEEK_SET) != MZ_OK)
            return MZ_STREAM_ERROR;
    }

    while (bytes_left_to_write > 0)
    {
        bytes_used = buffered->writebuf_len;
        if (bytes_used > buffered->writebuf_pos)
            bytes_used = buffered->writebuf_pos;
        bytes_to_copy = (uint32_t)(sizeof(buffered->writebuf) - bytes_used);
        if (bytes_to_copy > bytes_left_to_write)
            bytes_to_copy = bytes_left_to_write;

        if (bytes_to_copy == 0)
        {
            if (mz_stream_buffered_flush(stream, &bytes_flushed) != MZ_OK)
                return MZ_STREAM_ERROR;
            if (bytes_flushed == 0)
                return 0;

            continue;
        }

        memcpy(buffered->writebuf + buffered->writebuf_pos, (char *)buf + (bytes_to_write - bytes_left_to_write), bytes_to_copy);

        mz_stream_buffered_print(stream, "write copy [remaining %d write %d:%d len %d]\n", 
            bytes_to_copy, bytes_to_write, bytes_left_to_write, buffered->writebuf_len);

        bytes_left_to_write -= bytes_to_copy;

        buffered->writebuf_pos += bytes_to_copy;
        buffered->writebuf_hits += 1;
        if (buffered->writebuf_pos > buffered->writebuf_len)
            buffered->writebuf_len += buffered->writebuf_pos - buffered->writebuf_len;
    }

    return size - bytes_left_to_write;
}

int64_t mz_stream_buffered_tell(void *stream)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int64_t position = mz_stream_tell(buffered->stream.base);

    buffered->position = position;

    mz_stream_buffered_print(stream, "tell [pos %llu readpos %d writepos %d err %d]\n",
        buffered->position, buffered->readbuf_pos, buffered->writebuf_pos, errno);

    if (buffered->readbuf_len > 0)
        position -= (buffered->readbuf_len - buffered->readbuf_pos);
    if (buffered->writebuf_len > 0)
        position += buffered->writebuf_pos;
    return position;
}

int32_t mz_stream_buffered_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int32_t bytes_flushed = 0;

    mz_stream_buffered_print(stream, "seek [origin %d offset %llu pos %lld]\n", origin, offset, buffered->position);

    switch (origin)
    {
        case MZ_SEEK_SET:

            if (buffered->writebuf_len > 0)
            {
                if ((offset >= buffered->position) && (offset <= buffered->position + buffered->writebuf_len))
                {
                    buffered->writebuf_pos = (uint32_t)(offset - buffered->position);
                    return MZ_OK;
                }
            }

            if ((buffered->readbuf_len > 0) && (offset < buffered->position) && 
                (offset >= buffered->position - buffered->readbuf_len))
            {
                buffered->readbuf_pos = (uint32_t)(offset - (buffered->position - buffered->readbuf_len));
                return MZ_OK;
            }

            if (mz_stream_buffered_flush(stream, &bytes_flushed) != MZ_OK)
                return MZ_STREAM_ERROR;

            buffered->position = offset;
            break;

        case MZ_SEEK_CUR:

            if (buffered->readbuf_len > 0)
            {
                if (offset <= (buffered->readbuf_len - buffered->readbuf_pos))
                {
                    buffered->readbuf_pos += (uint32_t)offset;
                    return MZ_OK;
                }
                offset -= (buffered->readbuf_len - buffered->readbuf_pos);
                buffered->position += offset;
            }
            if (buffered->writebuf_len > 0)
            {
                if (offset <= (buffered->writebuf_len - buffered->writebuf_pos))
                {
                    buffered->writebuf_pos += (uint32_t)offset;
                    return MZ_OK;
                }
                //offset -= (buffered->writebuf_len - buffered->writebuf_pos);
            }

            if (mz_stream_buffered_flush(stream, &bytes_flushed) != MZ_OK)
                return MZ_STREAM_ERROR;

            break;

        case MZ_SEEK_END:

            if (buffered->writebuf_len > 0)
            {
                buffered->writebuf_pos = buffered->writebuf_len;
                return MZ_OK;
            }
            break;
    }

    buffered->readbuf_len = 0;
    buffered->readbuf_pos = 0;
    buffered->writebuf_len = 0;
    buffered->writebuf_pos = 0;

    return mz_stream_seek(buffered->stream.base, offset, origin);
}

int32_t mz_stream_buffered_close(void *stream)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    int32_t bytes_flushed = 0;

    mz_stream_buffered_flush(stream, &bytes_flushed);
    mz_stream_buffered_print(stream, "close\n");

    if (buffered->readbuf_hits + buffered->readbuf_misses > 0)
        mz_stream_buffered_print(stream, "read efficency %.02f%%\n", 
            (buffered->readbuf_hits / ((float)buffered->readbuf_hits + buffered->readbuf_misses)) * 100);

    if (buffered->writebuf_hits + buffered->writebuf_misses > 0)
        mz_stream_buffered_print(stream, "write efficency %.02f%%\n", 
            (buffered->writebuf_hits / ((float)buffered->writebuf_hits + buffered->writebuf_misses)) * 100);

    return mz_stream_close(buffered->stream.base);
}

int32_t mz_stream_buffered_error(void *stream)
{
    mz_stream_buffered *buffered = (mz_stream_buffered *)stream;
    return mz_stream_error(buffered->stream.base);
}

void *mz_stream_buffered_create(void **stream)
{
    mz_stream_buffered *buffered = NULL;

    buffered = (mz_stream_buffered *)malloc(sizeof(mz_stream_buffered));
    if (buffered != NULL)
    {
        memset(buffered, 0, sizeof(mz_stream_buffered));
        buffered->stream.vtbl = &mz_stream_buffered_vtbl;
    }
    if (stream != NULL)
        *stream = buffered;

    return buffered;
}

void mz_stream_buffered_delete(void **stream)
{
    mz_stream_buffered *buffered = NULL;
    if (stream == NULL)
        return;
    buffered = (mz_stream_buffered *)*stream;
    if (buffered != NULL)
        free(buffered);
    *stream = NULL;
}

void *mz_stream_buffered_get_interface(void)
{
    return (void *)&mz_stream_buffered_vtbl;
}

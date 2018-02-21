/* mz_strm_split.c -- Stream for split files
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_split.h"

/***************************************************************************/

#define MZ_ZIP_MAGIC_DISKHEADER         (0x08074b50)

#if defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#endif

/***************************************************************************/

mz_stream_vtbl mz_stream_split_vtbl = {
    mz_stream_split_open,
    mz_stream_split_is_open,
    mz_stream_split_read,
    mz_stream_split_write,
    mz_stream_split_tell,
    mz_stream_split_seek,
    mz_stream_split_close,
    mz_stream_split_error,
    mz_stream_split_create,
    mz_stream_split_delete,
    mz_stream_split_get_prop_int64,
    mz_stream_split_set_prop_int64
};

/***************************************************************************/

typedef struct mz_stream_split_s {
    mz_stream   stream;
    int32_t     is_open;
    int64_t     disk_size;
    int64_t     total_in;
    int64_t     total_in_disk;
    int64_t     total_out;
    int64_t     total_out_disk;
    int32_t     mode;
    char        *path_cd;
    int32_t     path_cd_size;
    char        *path_disk;
    int32_t     path_disk_size;
    int32_t     number_disk;
    int32_t     current_disk;
    int32_t     reached_end;
} mz_stream_split;

/***************************************************************************/

static int32_t mz_stream_split_open_disk(void *stream, int32_t number_disk)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    uint32_t magic = 0;
    int32_t i = 0;
    int32_t err = MZ_OK;
    int16_t disk_part = 0;


    // Check if we are reading or writing a disk part or the cd disk
    if (number_disk >= 0)
    {
        if ((split->mode & MZ_OPEN_MODE_WRITE) == 0)
            disk_part = MZ_OPEN_MODE_READ;
        else if (split->disk_size > 0)
            disk_part = MZ_OPEN_MODE_WRITE;
    }

    // Construct disk path
    if (disk_part > 0)
    {
        for (i = strlen(split->path_disk) - 1; i >= 0; i -= 1)
        {
            if (split->path_disk[i] != '.')
                continue;
            snprintf(&split->path_disk[i], split->path_disk_size - i, ".z%02d", number_disk + 1);
            break;
        }
    }
    else
    {
        strncpy(split->path_disk, split->path_cd, split->path_disk_size);
    }

    // If disk part doesn't exist during reading then return MZ_EXIST_ERROR
    if (disk_part == MZ_OPEN_MODE_READ)
        err = mz_os_file_exists(split->path_disk);

    if (err == MZ_OK)
        err = mz_stream_open(split->stream.base, split->path_disk, split->mode);

    if (err == MZ_OK)
    {
        split->total_in_disk = 0;
        split->total_out_disk = 0;
        split->current_disk = number_disk;

        if (split->mode & MZ_OPEN_MODE_WRITE)
        {
            if ((split->current_disk == 0) && (split->disk_size > 0))
            {
                err = mz_stream_write_uint32(split->stream.base, MZ_ZIP_MAGIC_DISKHEADER);
                split->total_out_disk += 4;
                split->total_out += split->total_out_disk;
            }
        }
        else if (split->mode & MZ_OPEN_MODE_READ)
        {
            if (split->current_disk == 0)
            {
                err = mz_stream_read_uint32(split->stream.base, &magic);
                if (magic != MZ_ZIP_MAGIC_DISKHEADER)
                    err = MZ_FORMAT_ERROR;
            }
        }
    }

    if (err == MZ_OK)
        split->is_open = 1;

    return err;
}

static int32_t mz_stream_split_close_disk(void *stream)
{
    mz_stream_split *split = (mz_stream_split *)stream;

    if (mz_stream_is_open(split->stream.base) != MZ_OK)
        return MZ_OK;

    return mz_stream_close(split->stream.base);
}

static int32_t mz_stream_split_goto_disk(void *stream, int32_t number_disk)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t err = MZ_OK;

    if ((split->disk_size == 0) && (split->mode & MZ_OPEN_MODE_WRITE))
    {
        if (mz_stream_is_open(split->stream.base) != MZ_OK)
            err = mz_stream_split_open_disk(stream, number_disk);
    }
    else if (number_disk != split->current_disk)
    {
        err = mz_stream_split_close_disk(stream);
        if (err == MZ_OK)
        {
            err = mz_stream_split_open_disk(stream, number_disk);
            if (err == MZ_OK)
                split->number_disk = number_disk;
        }
    }

    return err;
}

int32_t mz_stream_split_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t number_disk = 0;

    split->mode = mode;

    split->path_cd_size = strlen(path) + 1;
    split->path_cd = (char *)malloc(split->path_cd_size);
    strncpy(split->path_cd, path, split->path_cd_size);

    split->path_disk_size = strlen(path) + 10;
    split->path_disk = (char *)malloc(split->path_disk_size);
    strncpy(split->path_disk, path, split->path_disk_size);

    if (mode & MZ_OPEN_MODE_WRITE)
    {
        number_disk = 0;
        split->current_disk = -1;
    }
    else if (mode & MZ_OPEN_MODE_READ)
    {
        number_disk = -1;
        split->current_disk = 0;
    }

    return mz_stream_split_goto_disk(stream, number_disk);
}

int32_t mz_stream_split_is_open(void *stream)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    if (split->is_open == 1)
        return MZ_OK;
    return MZ_STREAM_ERROR;
}

int32_t mz_stream_split_read(void *stream, void *buf, int32_t size)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t bytes_left = size;
    int32_t read = 0;
    int32_t err = MZ_OK;
    uint8_t *buf_ptr = (uint8_t *)buf;

    err = mz_stream_split_goto_disk(stream, split->number_disk);
    if (err != MZ_OK)
        return err;

    while (bytes_left > 0)
    {
        read = mz_stream_read(split->stream.base, buf_ptr, bytes_left);
        if (read < 0)
            return read;
        if (read == 0)
        {
            if (split->current_disk < 0) // No more disks to goto
                break;
            err = mz_stream_split_goto_disk(stream, split->current_disk + 1);
            if (err == MZ_EXIST_ERROR)
                break;
            if (err != MZ_OK)
                return err;
        }

        bytes_left -= read;
        buf_ptr += read;
        split->total_in += read;
        split->total_in_disk += read;
    }
    return size - bytes_left;
}

int32_t mz_stream_split_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t written = 0;
    int32_t bytes_left = size;
    int32_t bytes_to_write = 0;
    int32_t bytes_avail = 0;
    int32_t number_disk = -1;
    int32_t err = MZ_OK;
    uint8_t *buf_ptr = (uint8_t *)buf;

    while (bytes_left > 0)
    {
        bytes_to_write = bytes_left;

        if (split->disk_size > 0)
        {
            if ((split->total_out_disk == split->disk_size && split->total_out > 0) ||
                (split->number_disk == -1 && split->number_disk != split->current_disk))
            {
                if (split->number_disk != -1)
                    number_disk = split->current_disk + 1;

                err = mz_stream_split_goto_disk(stream, number_disk);
                if (err != MZ_OK)
                    return err;
            }

            if (split->number_disk != -1)
            {
                bytes_avail = (int32_t)(split->disk_size - split->total_out_disk);
                if (bytes_to_write > bytes_avail)
                    bytes_to_write = bytes_avail;
            }
        }
        
        written = mz_stream_write(split->stream.base, buf_ptr, bytes_to_write);
        if (written != bytes_to_write)
            return MZ_STREAM_ERROR;
        
        bytes_left -= written;
        buf_ptr += written;
        split->total_out += written;
        split->total_out_disk += written;
    }

    return size - bytes_left;
}

int64_t mz_stream_split_tell(void *stream)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t err = MZ_OK;
    err = mz_stream_split_goto_disk(stream, split->number_disk);
    if (err != MZ_OK)
        return err;
    return mz_stream_tell(split->stream.base);
}

int32_t mz_stream_split_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t err = MZ_OK;
    err = mz_stream_split_goto_disk(stream, split->number_disk);
    if (err != MZ_OK)
        return err;
    return mz_stream_seek(split->stream.base, offset, origin);
}

int32_t mz_stream_split_close(void *stream)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    int32_t err = MZ_OK;

    err = mz_stream_split_close_disk(stream);
    split->is_open = 0;
    return err;
}

int32_t mz_stream_split_error(void *stream)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    return mz_stream_error(split->stream.base);
}

int32_t mz_stream_split_get_prop_int64(void *stream, int32_t prop, int64_t *value)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    switch (prop)
    {
    case MZ_STREAM_PROP_TOTAL_OUT:
        *value = split->total_out;
        return MZ_OK;
    case MZ_STREAM_PROP_DISK_NUMBER:
        *value = split->number_disk;
        return MZ_OK;
    case MZ_STREAM_PROP_DISK_SIZE:
        *value = split->disk_size;
        return MZ_OK;
    }
    return MZ_EXIST_ERROR;
}

int32_t mz_stream_split_set_prop_int64(void *stream, int32_t prop, int64_t value)
{
    mz_stream_split *split = (mz_stream_split *)stream;
    switch (prop)
    {
    case MZ_STREAM_PROP_DISK_NUMBER:
        split->number_disk = (int32_t)value;
        return MZ_OK;
    case MZ_STREAM_PROP_DISK_SIZE:
        split->disk_size = value;
        return MZ_OK;
    }
    return MZ_EXIST_ERROR;
}

void *mz_stream_split_create(void **stream)
{
    mz_stream_split *split = NULL;

    split = (mz_stream_split *)malloc(sizeof(mz_stream_split));
    if (split != NULL)
    {
        memset(split, 0, sizeof(mz_stream_split));
        split->stream.vtbl = &mz_stream_split_vtbl;
    }
    if (stream != NULL)
        *stream = split;

    return split;
}

void mz_stream_split_delete(void **stream)
{
    mz_stream_split *split = NULL;
    if (stream == NULL)
        return;
    split = (mz_stream_split *)*stream;
    if (split != NULL)
    {
        if (split->path_cd)
            free(split->path_cd);
        if (split->path_disk)
            free(split->path_disk);

        free(split);
    }
    *stream = NULL;
}

void *mz_stream_split_get_interface(void)
{
    return (void *)&mz_stream_split_vtbl;
}

/* mz_strm_crypt.c -- Code for traditional PKWARE encryption
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 1998-2005 Gilles Vollant
     Modifications for Info-ZIP crypting
     http://www.winimage.com/zLibDll/minizip.html
   Copyright (C) 2003 Terry Thorsen

   This code is a modified version of crypting code in Info-ZIP distribution

   Copyright (C) 1990-2000 Info-ZIP.  All rights reserved.

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.

   This encryption code is a direct transcription of the algorithm from
   Roger Schlafly, described by Phil Katz in the file appnote.txt. This
   file (appnote.txt) is distributed with the PKZIP program (even in the
   version without encryption capabilities).
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_crypt.h"

/***************************************************************************/

#define RAND_HEAD_LEN  12

/***************************************************************************/

mz_stream_vtbl mz_stream_crypt_vtbl = {
    mz_stream_crypt_open,
    mz_stream_crypt_is_open,
    mz_stream_crypt_read,
    mz_stream_crypt_write,
    mz_stream_crypt_tell,
    mz_stream_crypt_seek,
    mz_stream_crypt_close,
    mz_stream_crypt_error,
    mz_stream_crypt_create,
    mz_stream_crypt_delete,
    mz_stream_crypt_get_prop_int64
};

/***************************************************************************/

typedef struct mz_stream_crypt_s {
    mz_stream       stream;
    int32_t         error;
    int16_t         initialized;
    uint8_t         buffer[INT16_MAX];
    int64_t         total_in;
    int64_t         total_out;
    uint32_t        keys[3];          // keys defining the pseudo-random sequence
    const z_crc_t   *crc_32_tab;
    uint8_t         verify1;
    uint8_t         verify2;
    const char      *password;
} mz_stream_crypt;

/***************************************************************************/

#define zdecode(keys,crc_32_tab,c) \
    (mz_stream_crypt_update_keys(keys,crc_32_tab, c ^= mz_stream_crypt_decrypt_byte(keys)))

#define zencode(keys,crc_32_tab,c,t) \
    (t = mz_stream_crypt_decrypt_byte(keys), mz_stream_crypt_update_keys(keys,crc_32_tab,c), t^(c))

/***************************************************************************/

static uint8_t mz_stream_crypt_decrypt_byte(uint32_t *keys)
{
    unsigned temp;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    temp = ((uint32_t)(*(keys+2)) & 0xffff) | 2;
    return (uint8_t)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

static uint8_t mz_stream_crypt_update_keys(uint32_t *keys, const z_crc_t *crc_32_tab, int32_t c)
{
    #define CRC32(c, b) ((*(crc_32_tab+(((uint32_t)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

    (*(keys+0)) = (uint32_t)CRC32((*(keys+0)), c);
    (*(keys+1)) += (*(keys+0)) & 0xff;
    (*(keys+1)) = (*(keys+1)) * 134775813L + 1;
    {
        register int32_t keyshift = (int32_t)((*(keys + 1)) >> 24);
        (*(keys+2)) = (uint32_t)CRC32((*(keys+2)), keyshift);
    }
    return (uint8_t)c;
}

static void mz_stream_crypt_init_keys(const char *password, uint32_t *keys, const z_crc_t *crc_32_tab)
{
    *(keys+0) = 305419896L;
    *(keys+1) = 591751049L;
    *(keys+2) = 878082192L;

    while (*password != 0)
    {
        mz_stream_crypt_update_keys(keys, crc_32_tab, *password);
        password += 1;
    }
}

/***************************************************************************/

int32_t mz_stream_crypt_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    uint16_t t = 0;
    int16_t i = 0;
    uint8_t verify1 = 0;
    uint8_t verify2 = 0;
    uint8_t header[RAND_HEAD_LEN];
    const char *password = path;

    crypt->total_in = 0;
    crypt->total_out = 0;
    crypt->initialized = 0;

    if (mz_stream_is_open(crypt->stream.base) != MZ_OK)
        return MZ_STREAM_ERROR;

    if (password == NULL)
        password = crypt->password;
    if (password == NULL)
        return MZ_STREAM_ERROR;

    crypt->crc_32_tab = get_crc_table();
    if (crypt->crc_32_tab == NULL)
        return MZ_STREAM_ERROR;

    mz_stream_crypt_init_keys(password, crypt->keys, crypt->crc_32_tab);

    if (mode & MZ_OPEN_MODE_WRITE)
    {
        // First generate RAND_HEAD_LEN - 2 random bytes.
        mz_os_rand(header, RAND_HEAD_LEN - 2);
        
        // Encrypt random header (last two bytes is high word of crc)
        for (i = 0; i < RAND_HEAD_LEN - 2; i++)
            header[i] = (uint8_t)zencode(crypt->keys, crypt->crc_32_tab, header[i], t);
        
        header[i++] = (uint8_t)zencode(crypt->keys, crypt->crc_32_tab, crypt->verify1, t);
        header[i++] = (uint8_t)zencode(crypt->keys, crypt->crc_32_tab, crypt->verify2, t);

        if (mz_stream_write(crypt->stream.base, header, RAND_HEAD_LEN) != RAND_HEAD_LEN)
            return MZ_STREAM_ERROR;

        crypt->total_out += RAND_HEAD_LEN;
    }
    else if (mode & MZ_OPEN_MODE_READ)
    {
        if (mz_stream_read(crypt->stream.base, header, RAND_HEAD_LEN) != RAND_HEAD_LEN)
            return MZ_STREAM_ERROR;

        for (i = 0; i < RAND_HEAD_LEN - 2; i++)
            header[i] = (uint8_t)zdecode(crypt->keys, crypt->crc_32_tab, header[i]);

        verify1 = (uint8_t)zdecode(crypt->keys, crypt->crc_32_tab, header[i++]);
        verify2 = (uint8_t)zdecode(crypt->keys, crypt->crc_32_tab, header[i++]);

        // Older versions used 2 byte check, newer versions use 1 byte check.
        if ((verify2 != 0) && (verify2 != crypt->verify2))
            return MZ_PASSWORD_ERROR;

        crypt->total_in += RAND_HEAD_LEN;
    }

    crypt->initialized = 1;
    return MZ_OK;
}

int32_t mz_stream_crypt_is_open(void *stream)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    if (crypt->initialized == 0)
        return MZ_STREAM_ERROR;
    return MZ_OK;
}

int32_t mz_stream_crypt_read(void *stream, void *buf, int32_t size)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    uint8_t *buf_ptr = (uint8_t *)buf;
    uint32_t read = 0;
    uint32_t i = 0;

    read = mz_stream_read(crypt->stream.base, buf, size);

    for (i = 0; i < read; i++)
        buf_ptr[i] = (uint8_t)zdecode(crypt->keys, crypt->crc_32_tab, buf_ptr[i]);

    crypt->total_in += read;
    return read;
}

int32_t mz_stream_crypt_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    uint8_t *buf_ptr = (uint8_t *)buf;
    uint32_t written = 0;
    uint16_t t = 0;
    int32_t i = 0;

    if (size > sizeof(crypt->buffer))
        return MZ_STREAM_ERROR;

    for (i = 0; i < size; i++)
        crypt->buffer[i] = (uint8_t)zencode(crypt->keys, crypt->crc_32_tab, buf_ptr[i], t);

    written = mz_stream_write(crypt->stream.base, crypt->buffer, size);

    if (written > 0)
        crypt->total_out += written;

    return written;
}

int64_t mz_stream_crypt_tell(void *stream)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    return mz_stream_tell(crypt->stream.base);
}

int32_t mz_stream_crypt_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    return mz_stream_seek(crypt->stream.base, offset, origin);
}

int32_t mz_stream_crypt_close(void *stream)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    crypt->initialized = 0;
    return MZ_OK;
}

int32_t mz_stream_crypt_error(void *stream)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    return crypt->error;
}

void mz_stream_crypt_set_password(void *stream, const char *password)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    crypt->password = password;
}

void mz_stream_crypt_set_verify(void *stream, uint8_t verify1, uint8_t verify2)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    crypt->verify1 = verify1;
    crypt->verify2 = verify2;
}

void mz_stream_crypt_get_verify(void *stream, uint8_t *verify1, uint8_t *verify2)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    *verify1 = crypt->verify1;
    *verify2 = crypt->verify2;
}

int32_t mz_stream_crypt_get_prop_int64(void *stream, int32_t prop, int64_t *value)
{
    mz_stream_crypt *crypt = (mz_stream_crypt *)stream;
    switch (prop)
    {
    case MZ_STREAM_PROP_TOTAL_IN:
        *value = crypt->total_in;
        return MZ_OK;
    case MZ_STREAM_PROP_TOTAL_OUT:
        *value = crypt->total_out;
        return MZ_OK;
    }
    return MZ_EXIST_ERROR;
}

void *mz_stream_crypt_create(void **stream)
{
    mz_stream_crypt *crypt = NULL;

    crypt = (mz_stream_crypt *)malloc(sizeof(mz_stream_crypt));
    if (crypt != NULL)
    {
        memset(crypt, 0, sizeof(mz_stream_crypt));
        crypt->stream.vtbl = &mz_stream_crypt_vtbl;
    }

    if (stream != NULL)
        *stream = crypt;
    return crypt;
}

void mz_stream_crypt_delete(void **stream)
{
    mz_stream_crypt *crypt = NULL;
    if (stream == NULL)
        return;
    crypt = (mz_stream_crypt *)*stream;
    if (crypt != NULL)
        free(crypt);
    *stream = NULL;
}

void *mz_stream_crypt_get_interface(void)
{
    return (void *)&mz_stream_crypt_vtbl;
}
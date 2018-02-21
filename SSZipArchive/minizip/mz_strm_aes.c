/* mz_strm_aes.c -- Stream for WinZip AES encryption
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
      https://github.com/nmoinvaz/minizip
   Copyright (C) 1998-2010 Brian Gladman, Worcester, UK

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"
#include "hmac.h"
#include "pwd2key.h"

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_aes.h"

/***************************************************************************/

#define MZ_AES_KEY_LENGTH(mode)     (8 * (mode & 3) + 8)
#define MZ_AES_KEY_LENGTH_MAX       (32)
#define MZ_AES_KEYING_ITERATIONS    (1000)
#define MZ_AES_SALT_LENGTH(mode)    (4 * (mode & 3) + 4)
#define MZ_AES_SALT_LENGTH_MAX      (16)
#define MZ_AES_MAC_LENGTH(mode)     (10)
#define MZ_AES_PW_LENGTH_MAX        (128)
#define MZ_AES_PW_VERIFY_SIZE       (2)
#define MZ_AES_AUTHCODE_SIZE        (10)

/***************************************************************************/

mz_stream_vtbl mz_stream_aes_vtbl = {
    mz_stream_aes_open,
    mz_stream_aes_is_open,
    mz_stream_aes_read,
    mz_stream_aes_write,
    mz_stream_aes_tell,
    mz_stream_aes_seek,
    mz_stream_aes_close,
    mz_stream_aes_error,
    mz_stream_aes_create,
    mz_stream_aes_delete,
    mz_stream_aes_get_prop_int64
};

/***************************************************************************/

typedef struct mz_stream_aes_s {
    mz_stream       stream;
    int32_t         mode;
    int32_t         error;
    int16_t         initialized;
    uint8_t         buffer[INT16_MAX];
    int64_t         total_in;
    int64_t         total_out;
    int16_t         encryption_mode;
    const char      *password;
    aes_encrypt_ctx encr_ctx[1];
    hmac_ctx        auth_ctx[1];
    uint8_t         nonce[AES_BLOCK_SIZE]; 
    uint8_t         encr_bfr[AES_BLOCK_SIZE];
    uint32_t        encr_pos;
} mz_stream_aes;

/***************************************************************************/

int32_t mz_stream_aes_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    uint8_t kbuf[2 * MZ_AES_KEY_LENGTH_MAX + MZ_AES_PW_VERIFY_SIZE];
    uint8_t verify[MZ_AES_PW_VERIFY_SIZE];
    uint8_t verify_expected[MZ_AES_PW_VERIFY_SIZE];
    uint8_t salt_value[MZ_AES_SALT_LENGTH_MAX];
    int32_t salt_length = 0;
    const char *password = path;
    int32_t password_length = 0;
    int32_t key_length = 0;

    aes->total_in = 0;
    aes->total_out = 0;
    aes->initialized = 0;

    if (mz_stream_is_open(aes->stream.base) != MZ_OK)
        return MZ_STREAM_ERROR;

    if (password == NULL)
        password = aes->password;
    if (password == NULL)
        return MZ_PARAM_ERROR;
    password_length = (int32_t)strlen(password);
    if (password_length > MZ_AES_PW_LENGTH_MAX)
        return MZ_PARAM_ERROR;

    if (aes->encryption_mode < 1 || aes->encryption_mode > 3)
        return MZ_PARAM_ERROR;
    salt_length = MZ_AES_SALT_LENGTH(aes->encryption_mode);

    if (mode & MZ_OPEN_MODE_WRITE)
    {
        mz_os_rand(salt_value, salt_length);
    }
    else if (mode & MZ_OPEN_MODE_READ)
    {
        if (mz_stream_read(aes->stream.base, salt_value, salt_length) != salt_length)
            return MZ_STREAM_ERROR;
    }

    key_length = MZ_AES_KEY_LENGTH(aes->encryption_mode);
    // Derive the encryption and authentication keys and the password verifier
    derive_key((const uint8_t *)password, password_length, salt_value, salt_length,
        MZ_AES_KEYING_ITERATIONS, kbuf, 2 * key_length + MZ_AES_PW_VERIFY_SIZE);

    // Initialize the encryption nonce and buffer pos
    aes->encr_pos = AES_BLOCK_SIZE;
    memset(aes->nonce, 0, AES_BLOCK_SIZE * sizeof(uint8_t));

    // Initialize for encryption using key 1
    aes_encrypt_key(kbuf, key_length, aes->encr_ctx);

    // Initialize for authentication using key 2
    hmac_sha_begin(HMAC_SHA1, aes->auth_ctx);
    hmac_sha_key(kbuf + key_length, key_length, aes->auth_ctx);

    memcpy(verify, kbuf + 2 * key_length, MZ_AES_PW_VERIFY_SIZE);

    if (mode & MZ_OPEN_MODE_WRITE)
    {
        if (mz_stream_write(aes->stream.base, salt_value, salt_length) != salt_length)
            return MZ_STREAM_ERROR;

        aes->total_out += salt_length;

        if (mz_stream_write(aes->stream.base, verify, MZ_AES_PW_VERIFY_SIZE) != MZ_AES_PW_VERIFY_SIZE)
            return MZ_STREAM_ERROR;

        aes->total_out += MZ_AES_PW_VERIFY_SIZE;
    }
    else if (mode & MZ_OPEN_MODE_READ)
    {
        aes->total_in += salt_length;

        if (mz_stream_read(aes->stream.base, verify_expected, MZ_AES_PW_VERIFY_SIZE) != MZ_AES_PW_VERIFY_SIZE)
            return MZ_STREAM_ERROR;

        aes->total_in += MZ_AES_PW_VERIFY_SIZE;

        if (memcmp(verify_expected, verify, MZ_AES_PW_VERIFY_SIZE) != 0)
            return MZ_PASSWORD_ERROR;
    }

    aes->mode = mode;
    aes->initialized = 1;

    return MZ_OK;
}

int32_t mz_stream_aes_is_open(void *stream)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    if (aes->initialized == 0)
        return MZ_STREAM_ERROR;
    return MZ_OK;
}

static int32_t mz_stream_aes_encrypt_data(void *stream, uint8_t *buf, int32_t size)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    uint32_t pos = aes->encr_pos;
    uint32_t i = 0;

    while (i < (uint32_t)size)
    {
        if (pos == AES_BLOCK_SIZE)
        {
            uint32_t j = 0;

            // Increment encryption nonce
            while (j < 8 && !++aes->nonce[j])
                j += 1;

            // Encrypt the nonce to form next xor buffer
            aes_encrypt(aes->nonce, aes->encr_bfr, aes->encr_ctx);
            pos = 0;
        }

        buf[i++] ^= aes->encr_bfr[pos++];
    }

    aes->encr_pos = pos;
    return MZ_OK;
}

int32_t mz_stream_aes_read(void *stream, void *buf, int32_t size)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    int32_t read = 0;
    read = mz_stream_read(aes->stream.base, buf, size);
    if (read > 0)
    {
        hmac_sha_data((uint8_t *)buf, read, aes->auth_ctx);
        mz_stream_aes_encrypt_data(stream, (uint8_t *)buf, read);
    }

    aes->total_in += read;
    return read;
}

int32_t mz_stream_aes_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    int32_t written = 0;

    if (size > sizeof(aes->buffer))
        return MZ_STREAM_ERROR;

    memcpy(aes->buffer, buf, size);
    mz_stream_aes_encrypt_data(stream, (uint8_t *)aes->buffer, size);
    hmac_sha_data((uint8_t *)aes->buffer, size, aes->auth_ctx);

    written = mz_stream_write(aes->stream.base, aes->buffer, size);
    if (written > 0)
        aes->total_out += written;
    return written;
}

int64_t mz_stream_aes_tell(void *stream)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    return mz_stream_tell(aes->stream.base);
}

int32_t mz_stream_aes_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    return mz_stream_seek(aes->stream.base, offset, origin);
}

int32_t mz_stream_aes_close(void *stream)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    uint8_t authcode[MZ_AES_AUTHCODE_SIZE];
    uint8_t verify_authcode[MZ_AES_AUTHCODE_SIZE];

    if (MZ_AES_MAC_LENGTH(aes->encryption_mode) != MZ_AES_AUTHCODE_SIZE)
        return MZ_STREAM_ERROR;
    hmac_sha_end(authcode, MZ_AES_MAC_LENGTH(aes->encryption_mode), aes->auth_ctx);

    if (aes->mode & MZ_OPEN_MODE_WRITE)
    {
        if (mz_stream_write(aes->stream.base, authcode, MZ_AES_AUTHCODE_SIZE) != MZ_AES_AUTHCODE_SIZE)
            return MZ_STREAM_ERROR;

        aes->total_out += MZ_AES_AUTHCODE_SIZE;
    }
    else if (aes->mode & MZ_OPEN_MODE_READ)
    {
        if (mz_stream_read(aes->stream.base, verify_authcode, MZ_AES_AUTHCODE_SIZE) != MZ_AES_AUTHCODE_SIZE)
            return MZ_STREAM_ERROR;

        aes->total_in += MZ_AES_AUTHCODE_SIZE;

        if (memcmp(authcode, verify_authcode, MZ_AES_AUTHCODE_SIZE) != 0)
            return MZ_CRC_ERROR;
    }

    aes->initialized = 0;
    return MZ_OK;
}

int32_t mz_stream_aes_error(void *stream)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    return aes->error;
}

void mz_stream_aes_set_password(void *stream, const char *password)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    aes->password = password;
}

void mz_stream_aes_set_encryption_mode(void *stream, int16_t encryption_mode)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    aes->encryption_mode = encryption_mode;
}

int32_t mz_stream_aes_get_prop_int64(void *stream, int32_t prop, int64_t *value)
{
    mz_stream_aes *aes = (mz_stream_aes *)stream;
    switch (prop)
    {
    case MZ_STREAM_PROP_TOTAL_IN: 
        *value = aes->total_in;
        return MZ_OK;
    case MZ_STREAM_PROP_TOTAL_OUT: 
        *value = aes->total_out;
        return MZ_OK;
    case MZ_STREAM_PROP_HEADER_SIZE:
        *value = MZ_AES_SALT_LENGTH(aes->encryption_mode) + MZ_AES_PW_VERIFY_SIZE;
        return MZ_OK;
    case MZ_STREAM_PROP_FOOTER_SIZE:
        *value = MZ_AES_AUTHCODE_SIZE;
        return MZ_OK;
    }
    return MZ_EXIST_ERROR;
}

void *mz_stream_aes_create(void **stream)
{
    mz_stream_aes *aes = NULL;

    aes = (mz_stream_aes *)malloc(sizeof(mz_stream_aes));
    if (aes != NULL)
    {
        memset(aes, 0, sizeof(mz_stream_aes));
        aes->stream.vtbl = &mz_stream_aes_vtbl;
        aes->encryption_mode = MZ_AES_ENCRYPTION_MODE_256;
    }
    if (stream != NULL)
        *stream = aes;

    return aes;
}

void mz_stream_aes_delete(void **stream)
{
    mz_stream_aes *aes = NULL;
    if (stream == NULL)
        return;
    aes = (mz_stream_aes *)*stream;
    if (aes != NULL)
        free(aes);
    *stream = NULL;
}

void *mz_stream_aes_get_interface(void)
{
    return (void *)&mz_stream_aes_vtbl;
}

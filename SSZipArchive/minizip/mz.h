/* mz.h -- Errors codes, zip flags and magic
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_H
#define _MZ_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

// MZ_VERSION
#define MZ_VERSION                      ("2.2.7")

// MZ_ERROR
#define MZ_OK                           (0)
#define MZ_STREAM_ERROR                 (-1)
#define MZ_DATA_ERROR                   (-3)
#define MZ_MEM_ERROR                    (-4)
#define MZ_END_OF_LIST                  (-100)
#define MZ_END_OF_STREAM                (-101)
#define MZ_PARAM_ERROR                  (-102)
#define MZ_FORMAT_ERROR                 (-103)
#define MZ_INTERNAL_ERROR               (-104)
#define MZ_CRC_ERROR                    (-105)
#define MZ_CRYPT_ERROR                  (-106)
#define MZ_EXIST_ERROR                  (-107)
#define MZ_PASSWORD_ERROR               (-108)

// MZ_OPEN
#define MZ_OPEN_MODE_READ               (0x01)
#define MZ_OPEN_MODE_WRITE              (0x02)
#define MZ_OPEN_MODE_READWRITE          (MZ_OPEN_MODE_READ | MZ_OPEN_MODE_WRITE)
#define MZ_OPEN_MODE_APPEND             (0x04)
#define MZ_OPEN_MODE_CREATE             (0x08)
#define MZ_OPEN_MODE_EXISTING           (0x10)

// MZ_SEEK
#define MZ_SEEK_SET                     (0)
#define MZ_SEEK_CUR                     (1)
#define MZ_SEEK_END                     (2)

// MZ_COMPRESS
#define MZ_COMPRESS_METHOD_RAW          (0)
#define MZ_COMPRESS_METHOD_DEFLATE      (8)
#define MZ_COMPRESS_METHOD_BZIP2        (12)
#define MZ_COMPRESS_METHOD_LZMA         (14)
#define MZ_COMPRESS_METHOD_AES          (99)

#define MZ_COMPRESS_LEVEL_DEFAULT       (-1)
#define MZ_COMPRESS_LEVEL_FAST          (2)
#define MZ_COMPRESS_LEVEL_NORMAL        (6)
#define MZ_COMPRESS_LEVEL_BEST          (9)

// MZ_ZIP
#define MZ_ZIP_FLAG_ENCRYPTED           (1 << 0)
#define MZ_ZIP_FLAG_LZMA_EOS_MARKER     (1 << 1)
#define MZ_ZIP_FLAG_DEFLATE_MAX         (1 << 1)
#define MZ_ZIP_FLAG_DEFLATE_NORMAL      (0)
#define MZ_ZIP_FLAG_DEFLATE_FAST        (1 << 2)
#define MZ_ZIP_FLAG_DEFLATE_SUPER_FAST  (MZ_ZIP_FLAG_DEFLATE_FAST | \
                                         MZ_ZIP_FLAG_DEFLATE_MAX)
#define MZ_ZIP_FLAG_DATA_DESCRIPTOR     (1 << 3)

// MZ_AES
#define MZ_AES_VERSION                  (1)
#define MZ_AES_ENCRYPTION_MODE_128      (0x01)
#define MZ_AES_ENCRYPTION_MODE_192      (0x02)
#define MZ_AES_ENCRYPTION_MODE_256      (0x03)

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

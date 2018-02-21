/* mz_os.h -- System functions
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_OS_H
#define _MZ_OS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

#if !defined(_WIN32) && !defined(MZ_USE_WIN32_API)
#include "mz_os_posix.h"
#include "mz_strm_posix.h"
#else
#include "mz_os_win32.h"
#include "mz_strm_win32.h"
#endif

/***************************************************************************/

#ifdef HAVE_LZMA
#define MZ_VERSION_MADEBY_ZIP_VERSION (63)
#elif HAVE_AES
#define MZ_VERSION_MADEBY_ZIP_VERSION (51)
#elif HAVE_BZIP2
#define MZ_VERSION_MADEBY_ZIP_VERSION (46)
#else
#define MZ_VERSION_MADEBY_ZIP_VERSION (45)
#endif

#define MZ_VERSION_MADEBY             ((MZ_VERSION_MADEBY_HOST_SYSTEM << 8) | \
                                       (MZ_VERSION_MADEBY_ZIP_VERSION))

/***************************************************************************/

int32_t mz_make_dir(const char *path);
// Creates a directory recursively

int32_t mz_path_combine(char *path, const char *join, int32_t max_path);
// Combines two paths

int32_t mz_get_file_crc(const char *path, uint32_t *result_crc);
// Gets the crc32 hash of a file

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

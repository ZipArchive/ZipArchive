/* zip.h -- Backwards compatible zip interface
   part of the minizip-ng project

   Copyright (C) Nathan Moinvaziri
     https://github.com/zlib-ng/minizip-ng
   Copyright (C) 1998-2010 Gilles Vollant
     https://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.

 WARNING: Be very careful updating/overwriting this file.
 It has specific changes for ZipArchive support, notably MZ_COMPAT_VERSION.
*/

#ifndef _zip64_H
#define _zip64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if !defined(_ZLIB_H) && !defined(ZLIB_H) && !defined(ZLIB_H_)
#  if __has_include(<zlib-ng.h>)
#    include <zlib-ng.h>
#  elif __has_include(<zlib.h>)
#    include <zlib.h>
#  endif
#endif

#ifndef _ZLIBIOAPI_H
#  include "ioapi.h"
#endif

/***************************************************************************/

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagzipFile__ {
    int unused;
} zipFile__;
typedef zipFile__ *zipFile;
#else
typedef void *zipFile;
#endif

/***************************************************************************/

#define ZIP_OK            (0)
#define ZIP_EOF           (0)
#define ZIP_ERRNO         (-1) /* Z_ERRNO */
#define ZIP_PARAMERROR    (-102)
#define ZIP_BADZIPFILE    (-103)
#define ZIP_INTERNALERROR (-104)

/***************************************************************************/
/* default memLevel */
#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL MAX_MEM_LEVEL
#  endif
#endif

/***************************************************************************/
/* tm_zip contain date/time info */
typedef struct tm_zip_s {
    int tm_sec;  /* seconds after the minute - [0,59] */
    int tm_min;  /* minutes after the hour - [0,59] */
    int tm_hour; /* hours since midnight - [0,23] */
    int tm_mday; /* day of the month - [1,31] */
    int tm_mon;  /* months since January - [0,11] */
    int tm_year; /* years - [1980..2044] */
} tm_zip;

/***************************************************************************/

// ZipArchive 2.x+ uses dos_date
#define MZ_COMPAT_VERSION 120

#if !defined(MZ_COMPAT_VERSION) || MZ_COMPAT_VERSION <= 110
#  define mz_dos_date dosDate
#else
#  define mz_dos_date dos_date
#endif

typedef struct {
    tm_zip tmz_date;           /* date in understandable format           */
    unsigned long mz_dos_date; /* if dos_date == 0, tmz_date is used      */
    unsigned long internal_fa; /* internal file attributes        2 bytes */
    unsigned long external_fa; /* external file attributes        4 bytes */
} zip_fileinfo;

typedef const char *zipcharpc;

#define APPEND_STATUS_CREATE      (0)
#define APPEND_STATUS_CREATEAFTER (1)
#define APPEND_STATUS_ADDINZIP    (2)

/***************************************************************************/
/* Writing a zip file  */

/* Compatibility layer with the original minizip library (zip.h). */
ZEXPORT zipFile zipOpen(const char *path, int append);
ZEXPORT zipFile zipOpen64(const void *path, int append);

ZEXPORT zipFile zipOpen2(const char *path, int append, zipcharpc *globalcomment, zlib_filefunc_def *pzlib_filefunc_def);
ZEXPORT zipFile zipOpen2_64(const void *path, int append, zipcharpc *globalcomment,
                            zlib_filefunc64_def *pzlib_filefunc_def);
/* zipOpen3 is not supported */

ZEXPORT int zipOpenNewFileInZip(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                const void *extrafield_local, uint16_t size_extrafield_local,
                                const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                int compression_method, int level);
ZEXPORT int zipOpenNewFileInZip64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                  const void *extrafield_local, uint16_t size_extrafield_local,
                                  const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                  int compression_method, int level, int zip64);
ZEXPORT int zipOpenNewFileInZip2(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                 const void *extrafield_local, uint16_t size_extrafield_local,
                                 const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                 int compression_method, int level, int raw);
ZEXPORT int zipOpenNewFileInZip2_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                    const void *extrafield_local, uint16_t size_extrafield_local,
                                    const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                    int compression_method, int level, int raw, int zip64);
ZEXPORT int zipOpenNewFileInZip3(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                 const void *extrafield_local, uint16_t size_extrafield_local,
                                 const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                 int compression_method, int level, int raw, int windowBits, int memLevel, int strategy,
                                 const char *password, unsigned long crc_for_crypting);
ZEXPORT int zipOpenNewFileInZip3_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                    const void *extrafield_local, uint16_t size_extrafield_local,
                                    const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                    int compression_method, int level, int raw, int windowBits, int memLevel,
                                    int strategy, const char *password, unsigned long crc_for_crypting, int zip64);
ZEXPORT int zipOpenNewFileInZip4(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                 const void *extrafield_local, uint16_t size_extrafield_local,
                                 const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                 int compression_method, int level, int raw, int windowBits, int memLevel, int strategy,
                                 const char *password, unsigned long crc_for_crypting, unsigned long version_madeby,
                                 unsigned long flag_base);
ZEXPORT int zipOpenNewFileInZip4_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                    const void *extrafield_local, uint16_t size_extrafield_local,
                                    const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                    int compression_method, int level, int raw, int windowBits, int memLevel,
                                    int strategy, const char *password, unsigned long crc_for_crypting,
                                    unsigned long version_madeby, unsigned long flag_base, int zip64);
ZEXPORT int zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len);
ZEXPORT int zipCloseFileInZipRaw(zipFile file, unsigned long uncompressed_size, unsigned long crc32);
ZEXPORT int zipCloseFileInZipRaw64(zipFile file, uint64_t uncompressed_size, unsigned long crc32);
ZEXPORT int zipCloseFileInZip(zipFile file);
/* zipAlreadyThere is too new */
ZEXPORT int zipClose(zipFile file, const char *global_comment);
/* zipRemoveExtraInfoBlock is not supported */

/* Compatibility layer with older minizip-ng (mz_zip.h). */
ZEXPORT zipFile zipOpen_MZ(void *stream, int append, zipcharpc *globalcomment);
ZEXPORT void *zipGetHandle_MZ(zipFile);
ZEXPORT void *zipGetStream_MZ(zipFile file);
ZEXPORT int zipOpenNewFileInZip_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                   const void *extrafield_local, uint16_t size_extrafield_local,
                                   const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                   int compression_method, int level, int zip64);
ZEXPORT int zipOpenNewFileInZip5(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                 const void *extrafield_local, uint16_t size_extrafield_local,
                                 const void *extrafield_global, uint16_t size_extrafield_global, const char *comment,
                                 int compression_method, int level, int raw, int windowBits, int memLevel, int strategy,
                                 const char *password, unsigned long crc_for_crypting, unsigned long version_madeby,
                                 unsigned long flag_base, int zip64);
ZEXPORT int zipCloseFileInZip64(zipFile file);
ZEXPORT int zipClose_64(zipFile file, const char *global_comment);
ZEXPORT int zipClose2_64(zipFile file, const char *global_comment, uint16_t version_madeby);
int zipClose_MZ(zipFile file, const char *global_comment);
int zipClose2_MZ(zipFile file, const char *global_comment, uint16_t version_madeby);

#ifdef __cplusplus
}
#endif

#endif /* _zip64_H */

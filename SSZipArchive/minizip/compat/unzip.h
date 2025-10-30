/* unzip.h -- Backwards compatible unzip interface
   part of the minizip-ng project

   Copyright (C) Nathan Moinvaziri
     https://github.com/zlib-ng/minizip-ng
   Copyright (C) 1998-2010 Gilles Vollant
     https://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.

 WARNING: Be very careful updating/overwriting this file.
 It has specific changes for ZipArchive support with some structs moved to SSZipCommon for public access.
*/

#ifndef _unz64_H
#define _unz64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SSZipCommon.h"

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

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagunzFile__ {
    int unused;
} unzFile__;
typedef unzFile__ *unzFile;
#else
typedef void *unzFile;
#endif

/***************************************************************************/

#define UNZ_OK                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (-1) /* Z_ERRNO */
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR          (-102)
#define UNZ_BADZIPFILE          (-103)
#define UNZ_INTERNALERROR       (-104)
#define UNZ_CRCERROR            (-105)
#define UNZ_BADPASSWORD         (-106) /* minizip-ng */

/***************************************************************************/
/* tm_unz contain date/time info */
typedef struct tm_unz_s {
    int tm_sec;  /* seconds after the minute - [0,59] */
    int tm_min;  /* minutes after the hour - [0,59] */
    int tm_hour; /* hours since midnight - [0,23] */
    int tm_mday; /* day of the month - [1,31] */
    int tm_mon;  /* months since January - [0,11] */
    int tm_year; /* years - [1980..2044] */
} tm_unz;

/***************************************************************************/

// ZipArchive 2.x+ uses dos_date
#define MZ_COMPAT_VERSION 120

#if !defined(MZ_COMPAT_VERSION) || MZ_COMPAT_VERSION <= 110
#  define mz_dos_date dosDate
#else
#  define mz_dos_date dos_date
#endif

/* Global data about the zip from end of central dir */
typedef struct unz_global_info64_s {
    uint64_t number_entry;      /* total number of entries in the central dir on this disk */
    unsigned long size_comment; /* size of the global comment of the zipfile */
    /* minizip-ng fields */
    uint32_t number_disk_with_CD; /* number the the disk with central dir, used for spanning ZIP */
} unz_global_info64;

/* Information about a file in the zip */
typedef struct unz_file_info64_s {
    unsigned long version;            /* version made by                 2 bytes */
    unsigned long version_needed;     /* version needed to extract       2 bytes */
    unsigned long flag;               /* general purpose bit flag        2 bytes */
    unsigned long compression_method; /* compression method              2 bytes */
    unsigned long mz_dos_date;        /* last mod file date in Dos fmt   4 bytes */
    unsigned long crc;                /* crc-32                          4 bytes */
    ZPOS64_T compressed_size;         /* compressed size                 8 bytes */
    ZPOS64_T uncompressed_size;       /* uncompressed size               8 bytes */
    unsigned long size_filename;      /* filename length                 2 bytes */
    unsigned long size_file_extra;    /* extra field length              2 bytes */
    unsigned long size_file_comment;  /* file comment length             2 bytes */

    unsigned long disk_num_start; /* disk number start               4 bytes */
    unsigned long internal_fa;    /* internal file attributes        2 bytes */
    unsigned long external_fa;    /* external file attributes        4 bytes */

    tm_unz tmu_date;

    /* minizip-ng fields */
    ZPOS64_T disk_offset;
    uint16_t size_file_extra_internal;
} unz_file_info64;

/***************************************************************************/

#if !defined(MZ_COMPAT_VERSION) || MZ_COMPAT_VERSION < 110
/* Possible values:
   0 - Uses OS default, e.g. Windows ignores case.
   1 - Is case sensitive.
   >= 2 - Ignore case.
*/
typedef int unzFileNameCase;
#else
typedef int (*unzFileNameComparer)(unzFile file, const char *filename1, const char *filename2);
#endif
typedef int (*unzIteratorFunction)(unzFile file);
typedef int (*unzIteratorFunction2)(unzFile file, unz_file_info64 *pfile_info, char *filename, uint16_t filename_size,
                                    void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size);

/***************************************************************************/
/* Reading a zip file */

/* Compatibility layer with the original minizip library (unzip.h). */
ZEXPORT unzFile unzOpen(const char *path);
ZEXPORT unzFile unzOpen64(const void *path);
ZEXPORT unzFile unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def);
ZEXPORT unzFile unzOpen2_64(const void *path, zlib_filefunc64_def *pzlib_filefunc_def);
ZEXPORT int unzClose(unzFile file);

ZEXPORT int unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info32);
ZEXPORT int unzGetGlobalInfo64(unzFile file, unz_global_info64 *pglobal_info);
ZEXPORT int unzGetGlobalComment(unzFile file, char *comment, unsigned long comment_size);
ZEXPORT int unzOpenCurrentFile(unzFile file);
ZEXPORT int unzOpenCurrentFilePassword(unzFile file, const char *password);
ZEXPORT int unzOpenCurrentFile2(unzFile file, int *method, int *level, int raw);
ZEXPORT int unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password);
ZEXPORT int unzReadCurrentFile(unzFile file, void *buf, uint32_t len);
ZEXPORT int unzCloseCurrentFile(unzFile file);
ZEXPORT int unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, char *filename, unsigned long filename_size,
                                  void *extrafield, unsigned long extrafield_size, char *comment,
                                  unsigned long comment_size);
ZEXPORT int unzGetCurrentFileInfo64(unzFile file, unz_file_info64 *pfile_info, char *filename,
                                    unsigned long filename_size, void *extrafield, unsigned long extrafield_size,
                                    char *comment, unsigned long comment_size);
ZEXPORT int unzGoToFirstFile(unzFile file);
ZEXPORT int unzGoToNextFile(unzFile file);
#if !defined(MZ_COMPAT_VERSION) || MZ_COMPAT_VERSION < 110
ZEXPORT int unzLocateFile(unzFile file, const char *filename, unzFileNameCase filename_case);
#else
ZEXPORT int unzLocateFile(unzFile file, const char *filename, unzFileNameComparer filename_compare_func);
#endif
/* unzStringFileNameCompare is too new */
ZEXPORT int unzGetLocalExtrafield(unzFile file, void *buf, unsigned int len);

/* Compatibility layer with older minizip-ng (mz_unzip.h). */
unzFile unzOpen_MZ(void *stream);
ZEXPORT int unzClose_MZ(unzFile file);
ZEXPORT void *unzGetHandle_MZ(unzFile file);
ZEXPORT void *unzGetStream_MZ(unzFile file);

/***************************************************************************/
/* Raw access to zip file */

typedef struct unz_file_pos_s {
    uint32_t pos_in_zip_directory; /* offset in zip file directory */
    uint32_t num_of_file;          /* # of file */
} unz_file_pos;

typedef struct unz64_file_pos_s {
    int64_t pos_in_zip_directory; /* offset in zip file directory  */
    uint64_t num_of_file;         /* # of file */
} unz64_file_pos;

/* Compatibility layer with the original minizip library (unzip.h). */
ZEXPORT int unzGetFilePos(unzFile file, unz_file_pos *file_pos);
ZEXPORT int unzGoToFilePos(unzFile file, unz_file_pos *file_pos);
ZEXPORT int unzGetFilePos64(unzFile file, unz64_file_pos *file_pos);
ZEXPORT int unzGoToFilePos64(unzFile file, const unz64_file_pos *file_pos);
ZEXPORT int64_t unzGetOffset64(unzFile file);
ZEXPORT unsigned long unzGetOffset(unzFile file);
ZEXPORT int unzSetOffset64(unzFile file, int64_t pos);
ZEXPORT int unzSetOffset(unzFile file, unsigned long pos);
ZEXPORT int32_t unztell(unzFile file);
ZEXPORT uint64_t unztell64(unzFile file);
ZEXPORT int unzeof(unzFile file);

/* Compatibility layer with older minizip-ng (mz_unzip.h). */
ZEXPORT int32_t unzTell(unzFile file);
ZEXPORT uint64_t unzTell64(unzFile file);
ZEXPORT int unzSeek(unzFile file, int32_t offset, int origin);
ZEXPORT int unzSeek64(unzFile file, int64_t offset, int origin);
ZEXPORT int unzEndOfFile(unzFile file);
ZEXPORT void *unzGetStream(unzFile file);

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _unz64_H */
